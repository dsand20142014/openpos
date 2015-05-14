ue.offset) |
	    (transp << info->var.transp.offset);

	switch (info->var.bits_per_pixel) {
	case 16:
		((u32 *) info->pseudo_palette)[regno] = v;
		break;
	case 24:
	case 32:
		((u32 *) info->pseudo_palette)[regno] = v;
		break;
	}
	return 0;
}

static int vmlfb_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	struct vml_info *vinfo = container_of(info, struct vml_info, info);
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	int ret;

	if (vma->vm_pgoff > (~0UL >> PAGE_SHIFT))
		return -EINVAL;
	if (offset + size > vinfo->vram_contig_size)
		return -EINVAL;
	ret = vmlfb_vram_offset(vinfo, offset);
	if (ret)
		return -EINVAL;
	offset += vinfo->vram_start;
	pgprot_val(vma->vm_page_prot) |= _PAGE_PCD;
	pgprot_val(vma->vm_page_prot) &= ~_PAGE_PWT;
	vma->vm_flags |= VM_RESERVED | VM_IO;
	if (remap_pfn_range(vma, vma->vm_start, offset >> PAGE_SHIFT,
						size, vma->vm_page_prot))
		return -EAGAIN;
	return 0;
}

static int vmlfb_sync(struct fb_info *info)
{
	return 0;
}

static int vmlfb_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	return -EINVAL;	/* just to force soft_cursor() call */
}

static struct fb_ops vmlfb_ops = {
	.owner = THIS_MODULE,
	.fb_open = vmlfb_open,
	.fb_release = vmlfb_release,
	.fb_check_var = vmlfb_check_var,
	.fb_set_par = vmlfb_set_par,
	.fb_blank = vmlfb_blank,
	.fb_pan_display = vmlfb_pan_display,
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
	.fb_cursor = vmlfb_cursor,
	.fb_sync = vmlfb_sync,
	.fb_mmap = vmlfb_mmap,
	.fb_setcolreg = vmlfb_setcolreg
};

static struct pci_device_id vml_ids[] = {
	{PCI_DEVICE(PCI_VENDOR_ID_INTEL, VML_DEVICE_VDC)},
	{0}
};

static struct pci_driver vmlfb_pci_driver = {
	.name = "vmlfb",
	.id_table = vml_ids,
	.probe = vml_pci_probe,
	.remove = __devexit_p(vml_pci_remove)
};

static void __exit vmlfb_cleanup(void)
{
	pci_unregister_driver(&vmlfb_pci_driver);
}

static int __init vmlfb_init(void)
{

#ifndef MODULE
	char *option = NULL;

	if (fb_get_options(MODULE_NAME, &option))
		return -ENODEV;
#endif

	printk(KERN_DEBUG MODULE_NAME ": initializing\n");
	mutex_init(&vml_mutex);
	INIT_LIST_HEAD(&global_no_mode);
	INIT_LIST_HEAD(&global_has_mode);

	return pci_register_driver(&vmlfb_pci_driver);
}

int vmlfb_register_subsys(struct vml_sys *sys)
{
	struct vml_info *entry;
	struct list_head *list;
	u32 save_activate;

	mutex_lock(&vml_mutex);
	if (subsys != NULL) {
		subsys->restore(subsys);
	}
	subsys = sys;
	subsys->save(subsys);

	/*
	 * We need to restart list traversal for each item, since we
	 * release the list mutex in the loop.
	 */

	list = global_no_mode.next;
	while (list != &global_no_mode) {
		list_del_init(list);
		entry = list_entry(list, struct vml_info, head);

		/*
		 * First, try the current mode which might not be
		 * completely validated with respect to the pixel clock.
		 */

		i