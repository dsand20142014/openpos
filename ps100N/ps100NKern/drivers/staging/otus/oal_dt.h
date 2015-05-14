inder */

			partitions[3].name       = gpmi_remainder_name;
			partitions[3].offset     = nand->chipsize +
						   gpd->boot_area_size_in_bytes;
			partitions[3].size       = MTDPART_SIZ_FULL;
			partitions[3].mask_flags = 0;

			/* Construct and register the partitions. */

			add_mtd_partitions(mtd, partitions, 4);

			/* Find the remainder partitions. */

			for (i = 0; i < MAX_MTD_DEVICES; i++) {
				search_mtd = get_mtd_device(0, i);
				if (!search_mtd)
					continue;
				if (search_mtd == ERR_PTR(-ENODEV))
					continue;
				if (search_mtd->name == gpmi_0_remainder_name)
					gpmi_0_remainder_mtd = search_mtd;
				if (search_mtd->name == gpmi_remainder_name)
					gpmi_remainder_mtd = search_mtd;
			}

			if (!gpmi_0_remainder_mtd || !gpmi_remainder_mtd) {
				dev_emerg(&g->dev->dev, "Can't find remainder "
								"partitions");
				BUG();
			}

			/* Concatenate the remainders and register. */

			concatenate[0] = gpmi_0_remainder_mtd;
			concatenate[1] = gpmi_remainder_mtd;

			g->general_use_mtd = mtd_concat_create(concatenate,
							2, "gpmi-general-use");

			add_mtd_device(g->general_use_mtd);

		}

	}

	/*
	 * When control arrives here, we've done whatever partitioning and
	 * concatenation we needed to protect the boot area, and we have
	 * identified a single MTD that represents the "general use" portion of
	 * the medium. Check if the user wants to partition the general use MTD
	 * further.
	 */

	/* Check for dynamic partitioning information. */

	if (gpd->partition_source_types) {
		r = parse_mtd_partitions(mtd, gpd->partition_source_types,
							&g->partitions, 0);
		if (r > 0)
			g->partition_count = r;
		else {
			g->partitions      = 0;
			g->partition_count = 0;
		}
	}

	/* Fall back to platform partitions? */

	if (!g->partition_count && gpd->partitions && gpd->partition_count) {
		g->partitions      = gpd->partitions;
		g->partition_count = gpd->partition_count;
	}

	/* If we have partitions, implement them. */

	if (g->partitions) {
		pr_info("Applying partitions to the general use area.\n");
		add_mtd_partitions(g->general_use_mtd,
					g->partitions, g->partition_count);
	}

	/*
	 * Check if we're supposed to register the MTD that represents
	 * the entire medium.
	 */

	if (add_mtd_entire) {
		pr_info("Registering the full NAND Flash medium MTD.\n");
		add_mtd_device(mtd);
	}

#endif

	/* If control arrives here, everything went well. */

	return 0;

}

/**
 * gpmi_un