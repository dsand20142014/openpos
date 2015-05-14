d nodemgr used to handle all this itself,
 * but now we are much simpler because of the LDM.
 */

struct host_info {
	struct hpsb_host *host;
	struct list_head list;
	struct task_struct *thread;
};

static int nodemgr_bus_match(struct device * dev, struct device_driver * drv);
static int nodemgr_uevent(struct device *dev, struct kobj_uevent_env *env);

struct bus_type ieee1394_bus_type = {
	.name		= "ieee1394",
	.match		= nodemgr_bus_match,
};

static void host_cls_release(struct device *dev)
{
	put_device(&container_of((dev), struct hpsb_host, host_dev)->device);
}

struct class hpsb_host_class = {
	.name		= "ieee1394_host",
	.dev_release	= host_cls_release,
};

static void ne_cls_release(struct device *dev)
{
	put_device(&container_of((dev), struct node_entry, node_dev)->device);
}

static struct class nodemgr_ne_class = {
	.name		= "ieee1394_node",
	.dev_release	= ne_cls_release,
};

static void ud_cls_release(struct device *dev)
{
	put_device(&container_of((dev), struct unit_directory, unit_dev)->device);
}

/* The name here is only so that unit directory hotplug works with old
 * style hotplug, which only ever did unit directories anyway.
 */
static struct class nodemgr_ud_class = {
	.name		= "ieee1394",
	.dev_release	= ud_cls_release,
	.dev_uevent	= nodemgr_uevent,
};

static struct hpsb_highlevel nodemgr_highlevel;


static void nodemgr_release_ud(struct device *dev)
{
	struct unit_directory *ud = container_of(dev, struct unit_directory, device);

	if (ud->vendor_name_kv)
		csr1212_release_keyval(ud->vendor_name_kv);
	if (ud->model_name_kv)
		csr1212_release_keyval(ud->model_name_kv);

	kfree(ud);
}

static void nodemgr_release_ne(struct device *dev)
{
	struct node_entry *ne = container_of(dev, struct node_entry, device);

	if (ne->vendor_name_kv)
		csr1212_release_keyval(ne->vendor_name_kv);

	kfree(ne);
}


static void nodemgr_release_host(struct device *dev)
{
	struct hpsb_host *host = container_of(dev, struct hpsb_host, device);

	csr1212_destroy_csr(host->csr.rom);

	kfree(host);
}

static int nodemgr_ud_platform_data;

static struct device nodemgr_dev_template_ud = {
	.bus		= &ieee1394_bus_type,
	.release	= nodemgr_release_ud,
	.platform_data	= &nodemgr_ud_platform_data,
};

static struct device nodemgr_dev_template_ne = {
	.bus		= &ieee1394_bus_type,
	.release	= nodemgr_release_ne,
};

/* This dummy driver prevents the host devices from being scanned. We have no
 * useful drivers for them yet, and there would be a deadlock possible if the
 * driver core scans the host device while the host's low-level driver (i.e.
 * the host's parent device) is being removed. */
static struct device_driver nodemgr_mid_layer_driver = {
	.bus		= &ieee1394_bus_type,
	.name		= "nodemgr",
	.owner		= THIS_MODULE,
};

struct device nodemgr_dev_template_host = {
	.bus		= &ieee1394_bus_type,
	.release	= nodemgr_release_host,
};


#define fw_attr(class, class_type, field, type, format_string)		\
static ssize_t fw_show_##class##_##field (struct device *dev, struct device_attribute *attr, char *buf)\
{									\
	class_type *class;						\
	class = container_of(dev, class_type, device);			\
	return sprintf(buf, format_string, (type)class->field);		\
}									\
static struct device_attribute dev_attr_##class##_##field = {		\
	.attr = {.name = __stringify(field), .mode = S_IRUGO },		\
	.show   = fw_show_##class##_##field,				\
};

#define fw_attr_td(class, class_type, td_kv)				\
static ssize_t fw_show_##class##_##td_kv (struct device *dev, struct device_attribute *attr, char *buf)\
{									\
	int len;							\
	class_type *class = container_of(dev, class_type, device);	\
	len = (class->td_kv->value.leaf.len - 2) * sizeof(quadlet_t);	\
	memcpy(buf,							\
	       CSR1212_TEXTUAL_DESCRIPTOR_LEAF_DATA(class->td_kv),	\
	       len);							\
	while (buf[len - 1] == '\0')					\
		len--;							\
	buf[len++] = '\n';						\
	buf[len] = '\0';						\
	return len;							\
}									\
static struct device_attribute dev_attr_##class##_##td_kv = {		\
	.attr = {.name = __stringify(td_kv), .mode = S_IRUGO },		\
	.show   = fw_show_##class##_##td_kv,				\
};


#define fw_drv_attr(field, type, format_string)			\
static ssize_t fw_drv_show_##field (struct device_driver *drv, char *buf) \
{								\
	struct hpsb_protocol_driver *driver;			\
	driver = container_of(drv, struct hpsb_protocol_driver, driver); \
	return sprintf(buf, format_string, (type)driver->field);\
}								\
static struct driver_attribute driver_attr_drv_##field = {	\
	.attr = {.name = __stringify(field), .mode = S_IRUGO },	\
	.show   = fw_drv_show_##field,				\
};


static ssize_t fw_show_ne_bus_options(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct node_entry *ne = container_of(dev, struct node_entry, device);

	return sprintf(buf, "IRMC(%d) CMC(%d) ISC(%d) BMC(%d) PMC(%d) GEN(%d) "
		       "LSPD(%d) MAX_REC(%d) MAX_ROM(%d) CYC_CLK_ACC(%d)\n",
		       ne->busopt.irmc,
		       ne->busopt.cmc, ne->busopt.isc, ne->busopt.bmc,
		       ne->busopt.pmc, ne->busopt.generation, ne->busopt.lnkspd,
		       ne->busopt.max_rec,
		       ne->busopt.max_rom,
		       ne->busopt.cyc_clk_acc);
}
static DEVICE_ATTR(bus_options,S_IRUGO,fw_show_ne_bus_options,NULL);


#ifdef HPSB_DEBUG_TLABELS
static ssize_t fw_show_ne_tlabels_free(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct node_entry *ne = container_of(dev, struct node_entry, device);
	unsigned long flags;
	unsigned long *tp = ne->host->tl_pool[NODEID_TO_NODE(ne->nodeid)].map;
	int tf;

	spin_lock_irqsave(&hpsb_tlabel_lock, flags);
	tf = 64 - bitmap_weight(tp, 64);
	spin_unlock_irqrestore(&hpsb_tlabel_lock, flags);

	return sprintf(buf, "%d\n", tf);
}
static DEVICE_ATTR(tlabels_free,S_IRUGO,fw_show_ne_tlabels_free,NULL);


static ssize_t fw_show_ne_tlabels_mask(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct node_entry *ne = container_of(dev, struct node_entry, device);
	unsigned long flags;
	unsigned long *tp = ne->host->tl_pool[NODEID_TO_NODE(ne->nodeid)].map;
	u64 tm;

	spin_lock_irqsave(&hpsb_tlabel_lock, flags);
#if (BITS_PER_LONG <= 32)
	tm = ((u64)tp[0] << 32) + tp[1];
#else
	tm = tp[0];
#endif
	spin_unlock_irqrestore(&hpsb_tlabel_lock, flags);

	return sprintf(buf, "0x%016llx\n", (unsigned long long)tm);
}
static DEVICE_ATTR(tlabels_mask, S_IRUGO, fw_show_ne_tlabels_mask, NULL);
#endif /* HPSB_DEBUG_TLABELS */


static ssize_t fw_set_ignore_driver(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct unit_directory *ud = container_of(dev, struct unit_directory, device);
	int state = simple_strtoul(buf, NULL, 10);

	if (state == 1) {
		ud->ignore_driver = 1;
		device_release_driver(dev);
	} else if (state == 0)
		ud->ignore_driver = 0;

	return count;
}
static ssize_t fw_get_ignore_driver(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct unit_directory *ud = container_of(dev, struct unit_directory, device);

	return sprintf(buf, "%d\n", ud->ignore_driver);
}
static DEVICE_ATTR(ignore_driver, S_IWUSR | S_IRUGO, fw_get_ignore_driver, fw_set_ignore_driver);


static ssize_t fw_set_rescan(struct bus_type *bus, const char *buf,
			     size_t count)
{
	int error = 0;

	if (simple_strtoul(buf, NULL, 10) == 1)
		error = bus_rescan_devices(&ieee1394_bus_type);
	return error ? error : count;
}
static ssize_t fw_get_rescan(struct bus_type *bus, char *buf)
{
	return sprintf(buf, "You can force a rescan of the bus for "
			"drivers by writing a 1 to this file\n");
}
static BUS_ATTR(rescan, S_IWUSR | S_IRUGO, fw_get_rescan, fw_set_rescan);


static ssize_t fw_set_ignore_drivers(struct bus_type *bus, const char *buf, size_t count)
{
	int state = simple_strtoul(buf, NULL, 10);

	if (state == 1)
		ignore_drivers = 1;
	else if (state == 0)
		ignore_drivers = 0;

	return count;
}
static ssize_t fw_get_ignore_drivers(struct bus_type *bus, char *buf)
{
	return sprintf(buf, "%d\n", ignore_drivers);
}
static BUS_ATTR(ignore_drivers, S_IWUSR | S_IRUGO, fw_get_ignore_drivers, fw_set_ignore_drivers);


struct bus_attribute *const fw_bus_attrs[] = {
	&bus_attr_rescan,
	&bus_attr_ignore_drivers,
	NULL
};


fw_attr(ne, struct node_entry, capabilities, unsigned int, "0x%06x\n")
fw_attr(ne, struct node_entry, nodeid, unsigned int, "0x%04x\n")

fw_attr(ne, struct node_entry, vendor_id, unsigned int, "0x%06x\n")
fw_attr_td(ne, struct node_entry, vendor_name_kv)

fw_attr(ne, struct node_entry, guid, unsigned long long, "0x%016Lx\n")
fw_attr(ne, struct node_entry, guid_vendor_id, unsigned int, "0x%06x\n")
fw_attr(ne, struct node_entry, in_limbo, int, "%d\n");

static struct device_attribute *const fw_ne_attrs[] = {
	&dev_attr_ne_guid,
	&dev_attr_ne_guid_vendor_id,
	&dev_attr_ne_capabilities,
	&dev_attr_ne_vendor_id,
	&dev_attr_ne_nodeid,
	&dev_attr_bus_options,
#ifdef HPSB_DEBUG_TLABELS
	&dev_attr_tlabels_free,
	&dev_attr_tlabels_mask,
#endif
};



fw_attr(ud, struct unit_directory, address, unsigned long long, "0x%016Lx\n")
fw_attr(ud, struct unit_directory, length, int, "%d\n")
/* These are all dependent on the value being provided */
fw_attr(ud, struct unit_directory, vendor_id, unsigned int, "0x%06x\n")
fw_attr(ud, struct unit_directory, model_id, unsigned int, "0x%06x\n")
fw_attr(ud, struct unit_directory, specifier_id, unsigned int, "0x%06x\n")
fw_attr(ud, struct unit_directory, version, unsigned int, "0x%06x\n")
fw_attr_td(ud, struct unit_directory, vendor_name_kv)
fw_attr_td(ud, struct unit_directory, model_name_kv)

static struct device_attribute *const fw_ud_attrs[] = {
	&dev_attr_ud_address,
	&dev_attr_ud_length,
	&dev_attr_ignore_driver,
};


fw_attr(host, struct hpsb_host, node_count, int, "%d\n")
fw_attr(host, struct hpsb_host, selfid_count, int, "%d\n")
fw_attr(host, struct hpsb_host, nodes_active, int, "%d\n")
fw_attr(host, struct hpsb_host, in_bus_reset, int, "%d\n")
fw_attr(host, struct hpsb_host, is_root, int, "%d\n")
fw_attr(host, struct hpsb_host, is_cycmst, int, "%d\n")
fw_attr(host, struct hpsb_host, is_irm, int, "%d\n")
fw_attr(host, struct hpsb_host, is_busmgr, int, "%d\n")

static struct device_attribute *const fw_host_attrs[] = {
	&dev_attr_host_node_count,
	&dev_attr_host_selfid_count,
	&dev_attr_host_nodes_active,
	&dev_attr_host_in_bus_reset,
	&dev_attr_host_is_root,
	&dev_attr_host_is_cycmst,
	&dev_attr_host_is_irm,
	&dev_attr_host_is_busmgr,
};


static ssize_t fw_show_drv_device_ids(struct device_driver *drv, char *buf)
{
	struct hpsb_protocol_driver *driver;
	const struct ieee1394_device_id *id;
	int length = 0;
	char *scratch = buf;

	driver = container_of(drv, struct hpsb_protocol_driver, driver);
	id = driver->id_table;
	if (!id)
		return 0;

	for (; id->match_flags != 0; id++) {
		int need_coma = 0;

		if (id->match_flags & IEEE1394_MATCH_VENDOR_ID) {
			length += sprintf(scratch, "vendor_id=0x%06x", id->vendor_id);
			scratch = buf + length;
			need_coma++;
		}

		if (id->match_flags & IEEE1394_MATCH_MODEL_ID) {
			length += sprintf(scratch, "%smodel_id=0x%06x",
					  need_coma++ ? "," : "",
					  id->model_id);
			scratch = buf + length;
		}

		if (id->match_flags & IEEE1394_MATCH_SPECIFIER_ID) {
			length += sprintf(scratch, "%sspecifier_id=0x%06x",
					  need_coma++ ? "," : "",
					  id->specifier_id);
			scratch = buf + length;
		}

		if (id->match_flags & IEEE1394_MATCH_VERSION) {
			length += sprintf(scratch, "%sversion=0x%06x",
					  need_coma++ ? "," : "",
					  id->version);
			scratch = buf + length;
		}

		if (need_coma) {
			*scratch++ = '\n';
			length++;
		}
	}

	return length;
}
static DRIVER_ATTR(device_ids,S_IRUGO,fw_show_drv_device_ids,NULL);


fw_drv_attr(name, const char *, "%s\n")

static struct driver_attribute *const fw_drv_attrs[] = {
	&driver_attr_drv_name,
	&driver_attr_device_ids,
};


static void nodemgr_create_drv_files(struct hpsb_protocol_driver *driver)
{
	struct device_driver *drv = &driver->driver;
	int i;

	for (i = 0; i < ARRAY_SIZE(fw_drv_attrs); i++)
		if (driver_create_file(drv, fw_drv_attrs[i]))
			goto fail;
	return;
fail:
	HPSB_ERR("Failed to add sysfs attribute");
}


static void nodemgr_remove_drv_files(struct hpsb_protocol_driver *driver)
{
	struct device_driver *drv = &driver->driver;
	int i;

	for (i = 0; i < ARRAY_SIZE(fw_drv_attrs); i++)
		driver_remove_file(drv, fw_drv_attrs[i]);
}


static void nodemgr_create_ne_dev_files(struct node_entry *ne)
{
	struct device *dev = &ne->device;
	int i;

	for (i = 0; i < ARRAY_SIZE(fw_ne_attrs); i++)
		if (device_create_file(dev, fw_ne_attrs[i]))
			goto fail;
	return;
fail:
	HPSB_ERR("Failed to add sysfs attribute");
}


static void nodemgr_create_host_dev_files(struct hpsb_host *host)
{
	struct device *dev = &host->device;
	int i;

	for (i = 0; i < ARRAY_SIZE(fw_host_attrs); i++)
		if (device_create_file(dev, fw_host_attrs[i]))
			goto fail;
	return;
fail:
	HPSB_ERR("Failed to add sysfs attribute");
}


static struct node_entry *find_entry_by_nodeid(struct hpsb_host *host,
					       nodeid_t nodeid);

static void nodemgr_update_host_dev_links(struct hpsb_host *host)
{
	struct device *dev = &host->device;
	struct node_entry *ne;

	sysfs_remove_link(&dev->kobj, "irm_id");
	sysfs_remove_link(&dev->kobj, "busmgr_id");
	sysfs_remove_link(&dev->kobj, "host_id");

	if ((ne = find_entry_by_nodeid(host, host->irm_id)) &&
	    sysfs_create_link(&dev->kobj, &ne->device.kobj, "irm_id"))
		goto fail;
	if ((ne = find_entry_by_nodeid(host, host->busmgr_id)) &&
	    sysfs_create_link(&dev->kobj, &ne->device.kobj, "busmgr_id"))
		goto fail;
	if ((ne = find_entry_by_nodeid(host, host->node_id)) &&
	    sysfs_create_link(&dev->kobj, &ne->device.kobj, "host_id"))
		goto fail;
	return;
fail:
	HPSB_ERR("Failed to update sysfs attributes for host %d", host->id);
}

static void nodemgr_create_ud_dev_files(struct unit_directory *ud)
{
	struct device *dev = &ud->device;
	int i;

	for (i = 0; i < ARRAY_SIZE(fw_ud_attrs); i++)
		if (device_create_file(dev, fw_ud_attrs[i]))
			goto fail;
	if (ud->flags & UNIT_DIRECTORY_SPECIFIER_ID)
		if (device_create_file(dev, &dev_attr_ud_specifier_id))
			goto fail;
	if (ud->flags & UNIT_DIRECTORY_VERSION)
		if (device_create_file(dev, &dev_attr_ud_version))
			goto fail;
	if (ud->flags & UNIT_DIRECTORY_VENDOR_ID) {
		if (device_create_file(dev, &dev_attr_ud_vendor_id))
			goto fail;
		if (ud->vendor_name_kv &&
		    device_create_file(dev, &dev_attr_ud_vendor_name_kv))
			goto fail;
	}
	if (ud->flags & UNIT_DIRECTORY_MODEL_ID) {
		if (device_create_file(dev, &dev_attr_ud_model_id))
			goto fail;
		if (ud->model_name_kv &&
		    device_create_file(dev, &dev_attr_ud_model_name_kv))
			goto fail;
	}
	return;
fail:
	HPSB_ERR("Failed to add sysfs attribute");
}


static int nodemgr_bus_match(struct device * dev, struct device_driver * drv)
{
	struct hpsb_protocol_driver *driver;
	struct unit_directory *ud;
	const struct ieee1394_device_id *id;

	/* We only match unit directories */
	if (dev->platform_data != &nodemgr_ud_platform_data)
		return 0;

	ud = container_of(dev, struct unit_directory, device);
	if (ud->ne->in_limbo || ud->ignore_driver)
		return 0;

	/* We only match drivers of type hpsb_protocol_driver */
	if (drv == &nodemgr_mid_layer_driver)
		return 0;

	driver = container_of(drv, struct hpsb_protocol_driver, driver);
	id = driver->id_table;
	if (!id)
		return 0;

	for (; id->match_flags != 0; id++) {
		if ((id->match_flags & IEEE1394_MATCH_VENDOR_ID) &&
		    id->vendor_id != ud->vendor_id)
			continue;

		if ((id->match_flags & IEEE1394_MATCH_MODEL_ID) &&
		    id->model_id != ud->model_id)
			continue;

		if ((id->match_flags & IEEE1394_MATCH_SPECIFIER_ID) &&
		    id->specifier_id != ud->specifier_id)
			continue;

		if ((id->match_flags & IEEE1394_MATCH_VERSION) &&
		    id->version != ud->version)
			continue;

		return 1;
	}

	return 0;
}


static DEFINE_MUTEX(nodemgr_serialize_remove_uds);

static int match_ne(struct device *dev, void *data)
{
	struct unit_directory *ud;
	struct node_entry *ne = data;

	ud = container_of(dev, struct unit_directory, unit_dev);
	return ud->ne == ne;
}

static void nodemgr_remove_uds(struct node_entry *ne)
{
	struct device *dev;
	struct unit_directory *ud;

	/* Use class_find device to iterate the devices. Since this code
	 * may be called from other contexts besides the knodemgrds,
	 * protect it by nodemgr_serialize_remove_uds.
	 */
	mutex_lock(&nodemgr_serialize_remove_uds);
	for (;;) {
		dev = class_find_device(&nodemgr_ud_class, NULL, ne, match_ne);
		if (!dev)
			break;
		ud = container_of(dev, struct unit_directory, unit_dev);
		put_device(dev);
		device_unregister(&ud->unit_dev);
		device_unregister(&ud->device);
	}
	mutex_unlock(&nodemgr_serialize_remove_uds);
}


static void nodemgr_remove_ne(struct node_entry *ne)
{
	struct device *dev;

	dev = get_device(&ne->device);
	if (!dev)
		return;

	HPSB_DEBUG("Node removed: ID:BUS[" NODE_BUS_FMT "]  GUID[%016Lx]",
		   NODE_BUS_ARGS(ne->host, ne->nodeid), (unsigned long long)ne->guid);
	nodemgr_remove_uds(ne);

	device_unregister(&ne->node_dev);
	device_unregister(dev);

	put_device(dev);
}

static int remove_host_dev(struct device *dev, void *data)
{
	if (dev->bus == &ieee1394_bus_type)
		nodemgr_remove_ne(container_of(dev, struct node_entry,
				  device));
	return 0;
}

static void nodemgr_remove_host_dev(struct device *dev)
{
	device_for_each_child(dev, NULL, remove_host_dev);
	sysfs_remove_link(&dev->kobj, "irm_id");
	sysfs_remove_link(&dev->kobj, "busmgr_id");
	sysfs_remove_link(&dev->kobj, "host_id");
}


static void nodemgr_update_bus_options(struct node_entry *ne)
{
#ifdef CONFIG_IEEE1394_VERBOSEDEBUG
	static const u16 mr[] = { 4, 64, 1024, 0};
#endif
	quadlet_t busoptions = be32_to_cpu(ne->csr->bus_info_data[2]);

	ne->busopt.irmc		= (busoptions >> 31) & 1;
	ne->busopt.cmc		= (busoptions >> 30) & 1;
	ne->busopt.isc		= (busoptions >> 29) & 1;
	ne->busopt.bmc		= (busoptions >> 28) & 1;
	ne->busopt.pmc		= (busoptions >> 27) & 1;
	ne->busopt.cyc_clk_acc	= (busoptions >> 16) & 0xff;
	ne->busopt.max_rec	= 1 << (((busoptions >> 12) & 0xf) + 1);
	ne->busopt.max_rom	= (busoptions >> 8) & 0x3;
	ne->busopt.generation	= (busoptions >> 4) & 0xf;
	ne->busopt.lnkspd	= busoptions & 0x7;

	HPSB_VERBOSE("NodeMgr: raw=0x%08x irmc=%d cmc=%d isc=%d bmc=%d pmc=%d "
		     "cyc_clk_acc=%d max_rec=%d max_rom=%d gen=%d lspd=%d",
		     busoptions, ne->busopt.irmc, ne->busopt.cmc,
		     ne->busopt.isc, ne->busopt.bmc, ne->busopt.pmc,
		     ne->busopt.cyc_clk_acc, ne->busopt.max_rec,
		     mr[ne->busopt.max_rom],
		     ne->busopt.generation, ne->busopt.lnkspd);
}


static struct node_entry *nodemgr_create_node(octlet_t guid,
				struct csr1212_csr *csr, struct hpsb_host *host,
				nodeid_t nodeid, unsigned int generation)
{
	struct node_entry *ne;

	ne = kzalloc(sizeof(*ne), GFP_KERNEL);
	if (!ne)
		goto fail_alloc;

	ne->host = host;
	ne->nodeid = nodeid;
	ne->generation = generation;
	ne->needs_probe = true;

	ne->guid = guid;
	ne->guid_vendor_id = (guid >> 40) & 0xffffff;
	ne->csr = csr;

	memcpy(&ne->device, &nodemgr_dev_template_ne,
	       sizeof(ne->device));
	ne->device.parent = &host->device;
	dev_set_name(&ne->device, "%016Lx", (unsigned long long)(ne->guid));

	ne->node_dev.parent = &ne->device;
	ne->node_dev.class = &nodemgr_ne_class;
	dev_set_name(&ne->node_dev, "%016Lx", (unsigned long long)(ne->guid));

	if (device_register(&ne->device))
		goto fail_devreg;
	if (device_register(&ne->node_dev))
		goto fail_classdevreg;
	get_device(&ne->device);

	nodemgr_create_ne_dev_files(ne);

	nodemgr_update_bus_options(ne);

	HPSB_DEBUG("%s added: ID:BUS[" NODE_BUS_FMT "]  GUID[%016Lx]",
		   (host->node_id == nodeid) ? "Host" : "Node",
		   NODE_BUS_ARGS(host, nodeid), (unsigned long long)guid);

	return ne;

fail_classdevreg:
	device_unregister(&ne->device);
fail_devreg:
	kfree(ne);
fail_alloc:
	HPSB_ERR("Failed to create node ID:BUS[" NODE_BUS_FMT "]  GUID[%016Lx]",
		 NODE_BUS_ARGS(host, nodeid), (unsigned long long)guid);

	return NULL;
}

static int match_ne_guid(struct device *dev, void *data)
{
	struct node_entry *ne;
	u64 *guid = data;

	ne = container_of(dev, struct node_entry, node_dev);
	return ne->guid == *guid;
}

static struct node_entry *find_entry_by_guid(u64 guid)
{
	struct device *dev;
	struct node_entry *ne;

	dev = class_find_device(&nodemgr_ne_class, NULL, &guid, match_ne_guid);
	if (!dev)
		return NULL;
	ne = container_of(dev, struct node_entry, node_dev);
	put_device(dev);

	return ne;
}

struct match_nodeid_parameter {
	struct hpsb_host *host;
	nodeid_t nodeid;
};

static int match_ne_nodeid(struct device *dev, void *data)
{
	int found = 0;
	struct node_entry *ne;
	struct match_nodeid_parameter *p = data;

	if (!dev)
		goto ret;
	ne = container_of(dev, struct node_entry, node_dev);
	if (ne->host == p->host && ne->nodeid == p->nodeid)
		found = 1;
ret:
	return found;
}

static struct node_entry *find_entry_by_nodeid(struct hpsb_host *host,
					       nodeid_t nodeid)
{
	struct device *dev;
	struct node_entry *ne;
	struct match_nodeid_parameter p;

	p.host = host;
	p.nodeid = nodeid;

	dev = class_find_device(&nodemgr_ne_class, NULL, &p, match_ne_nodeid);
	if (!dev)
		return NULL;
	ne = container_of(dev, struct node_entry, node_dev);
	put_device(dev);

	return ne;
}


static void nodemgr_register_device(struct node_entry *ne, 
	struct unit_directory *ud, struct device *parent)
{
	memcpy(&ud->device, &nodemgr_dev_template_ud,
	       sizeof(ud->device));

	ud->device.parent = parent;

	dev_set_name(&ud->device, "%s-%u", dev_name(&ne->device), ud->id);

	ud->unit_dev.parent = &ud->device;
	ud->unit_dev.class = &nodemgr_ud_class;
	dev_set_name(&ud->unit_dev, "%s-%u", dev_name(&ne->device), ud->id);

	if (device_register(&ud->device))
		goto fail_devreg;
	if (device_register(&ud->unit_dev))
		goto fail_classdevreg;
	get_device(&ud->device);

	nodemgr_create_ud_dev_files(ud);

	return;

fail_classdevreg:
	device_unregister(&ud->device);
fail_devreg:
	HPSB_ERR("Failed to create unit %s", dev_name(&ud->device));
}	


/* This implementation currently only scans the config rom and its
 * immediate unit directories looking for software_id and
 * software_version entries, in order to get driver autoloading working. */
static struct unit_directory *nodemgr_process_unit_directory
	(struct node_entry *ne, struct csr1212_keyval *ud_kv,
	 unsigned int *id, struct unit_directory *parent)
{
	struct unit_directory *ud;
	struct unit_directory *ud_child = NULL;
	struct csr1212_dentry *dentry;
	struct csr1212_keyval *kv;
	u8 last_key_id = 0;

	ud = kzalloc(sizeof(*ud), GFP_KERNEL);
	if (!ud)
		goto unit_directory_error;

	ud->ne = ne;
	ud->ignore_driver = ignore_drivers;
	ud->address = ud_kv->offset + CSR1212_REGISTER_SPACE_BASE;
	ud->directory_id = ud->address & 0xffffff;
	ud->ud_kv = ud_kv;
	ud->id = (*id)++;

	/* inherit vendor_id from root directory if none exists in unit dir */
	ud->vendor_id = ne->vendor_id;

	csr1212_for_each_dir_entry(ne->csr, kv, ud_kv, dentry) {
		switch (kv->key.id) {
		case CSR1212_KV_ID_VENDOR:
			if (kv->key.type == CSR1212_KV_TYPE_IMMEDIATE) {
				ud->vendor_id = kv->value.immediate;
				ud->flags |= UNIT_DIRECTORY_VENDOR_ID;
			}
			break;

		case 