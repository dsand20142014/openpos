		spin_lock(&sdev->ud.lock);
		if (sdev->ud.status != SDEV_ST_USED) {
			spin_unlock(&sdev->ud.lock);
			return -EINVAL;
		}
		spin_unlock(&sdev->ud.lock);

		usbip_event_add(&sdev->ud, SDEV_EVENT_DOWN);
	}

	return count;
}
static DEVICE_ATTR(usbip_sockfd, S_IWUSR, NULL, store_sockfd);

static int stub_add_files(struct device *dev)
{
	int err = 0;

	err = device_create_file(dev, &dev_attr_usbip_status);
	if (err)
		goto err_status;

	err = device_create_file(dev, &dev_attr_usbip_sockfd);
	if (err)
		goto err_sockfd;

	err = device_create_file(dev, &dev_attr_usbip_debug);
	if (err)
		goto err_debug;

	return 0;

err_debug:
	device_remove_file(dev, &dev_attr_usbip_sockfd);

err_sockfd:
	device_remove_file(dev, &dev_attr_usbip_status);

err_status:
	return err;
}

static void stub_remove_files(struct device *dev)
{
	device_remove_file(dev, &dev_attr_usbip_status);
	device_remove_file(dev, &dev_attr_usbip_sockfd);
	device_remove_file(dev, &dev_attr_usbip_debug);
}



/*-------------------------------------------------------------------------*/

/* Ev