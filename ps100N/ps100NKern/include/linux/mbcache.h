nst struct v4l2_subdev_core_ops saa5246a_core_ops = {
	.g_chip_ident = saa5246a_g_chip_ident,
};

static const struct v4l2_subdev_ops saa5246a_ops = {
	.core = &saa5246a_core_ops,
};


static int saa5246a_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int pgbuf;
	int err;
	struct saa5246a_device *t;
	struct v4l2_subdev *sd;

	v4l_info(client, "chip found @ 0x%x (%s)\n",
			client->addr << 1, client->adapter->name);
	v4l_info(client, "VideoText version %d.%d\n",
			MAJOR_VERSION, MINOR_VERSION);
	t = kzalloc(sizeof(*t), GFP_KERNEL);
	if (t == NULL)
		return -ENOMEM;
	sd = &t->sd;
	v4l2_i2c_subdev_init(sd, client, &saa5246a_ops);
	mutex_init(&t->lock);

	/* Now create a video4linux device */
	t->vdev = video_device_alloc();
	if (t->vdev == NULL) {
		kfree(t);
		return -ENOMEM;
	}
	memcpy(t->vdev, &saa_template, sizeof(*t->vdev));

	for (pgbuf = 0; pgbuf < NUM_DAUS; pgbuf++) {
		memset(t->pgbuf[pgbuf], ' ', sizeof(t->pgbuf[0]));
		t->is_searching[pgbuf] = false;
	}
	video_set_drvdata(t->vdev, t);

	/* Register it */
	err = video_register_device(t->vdev, VFL_TYPE_VTX, -1);
	if (err < 0) {
		video_device_release(t->vdev);
		kfree(t);
		return err;
	}
	return 0;
}

static int saa5246a_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct saa5246a_device *t = to_dev(sd);

	video_unregister_device(t->vdev);
	v4l2_device_unregister_subdev(sd);
	kfree(t);
	return 0;
}

static const struct i2c_device_id saa5246a_id[] = {
	{ "saa5246a", 0 },
	{