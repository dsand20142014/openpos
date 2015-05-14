ruct scsi_host_sg_pool *sgp;

	sgp = scsi_sg_pools + scsi_sgtable_index(nents);
	mempool_free(sgl, sgp->pool);
}

static struct scatterlist *scsi_sg_alloc(unsigned int nents, gfp_t gfp_mask)
{
	struct scsi_host_sg_pool *sgp;

	sgp = scsi_sg_pools + scsi_sgtable_index(nents);
	return mempool_alloc(sgp->pool, gfp_mask);
}

static int scsi_alloc_sgtable(struct scsi_data_buffer *sdb, int nents,
			      gfp_t gfp_mask)
{
	int ret;

	BUG_ON(!nents);

	ret = __sg_alloc_table(&sdb->table, nents, SCSI_MAX_SG_SEGMENTS,
			       gfp_mask, scsi_sg_alloc);
	if (unlikely(ret))
		__sg_free_table(&sdb->table, SCSI_MAX_SG_SEGMENTS,
				scsi_sg_free);

	return ret;
}

static void scsi_free_sgtable(struct scsi_data_buffer *sdb)
