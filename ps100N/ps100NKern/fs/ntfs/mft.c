/**
 * mft.c - NTFS kernel mft record operations. Part of the Linux-NTFS project.
 *
 * Copyright (c) 2001-2006 Anton Altaparmakov
 * Copyright (c) 2002 Richard Russon
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/buffer_head.h>
#include <linux/swap.h>

#include "attrib.h"
#include "aops.h"
#include "bitmap.h"
#include "debug.h"
#include "dir.h"
#include "lcnalloc.h"
#include "malloc.h"
#include "mft.h"
#include "ntfs.h"

/**
 * map_mft_record_page - map the page in which a specific mft record resides
 * @ni:		ntfs inode whose mft record page to map
 *
 * This maps the page in which the mft record of the ntfs inode @ni is situated
 * and returns a pointer to the mft record within the mapped page.
 *
 * Return value needs to be checked with IS_ERR() and if that is true PTR_ERR()
 * contains the negative error code returned.
 */
static inline MFT_RECORD *map_mft_record_page(ntfs_inode *ni)
{
	loff_t i_size;
	ntfs_volume *vol = ni->vol;
	struct inode *mft_vi = vol->mft_ino;
	struct page *page;
	unsigned long index, end_index;
	unsigned ofs;

	BUG_ON(ni->page);
	/*
	 * The index into the page cache and the offset within the page cache
	 * page of the wanted mft record. FIXME: We need to check for
	 * overflowing the unsigned long, but I don't think we would ever get
	 * here if the volume was that big...
	 */
	index = (u64)ni->mft_no << vol->mft_record_size_bits >>
			PAGE_CACHE_SHIFT;
	ofs = (ni->mft_no << vol->mft_record_size_bits) & ~PAGE_CACHE_MASK;

	i_size = i_size_read(mft_vi);
	/* The maximum valid index into the page cache for $MFT's data. */
	end_index = i_size >> PAGE_CACHE_SHIFT;

	/* If the wanted index is out of bounds the mft record doesn't exist. */
	if (unlikely(index >= end_index)) {
		if (index > end_index || (i_size & ~PAGE_CACHE_MASK) < ofs +
				vol->mft_record_size) {
			page = ERR_PTR(-ENOENT);
			ntfs_error(vol->sb, "Attemt to read mft record 0x%lx, "
					"which is beyond the end of the mft.  "
					"This is probably a bug in the ntfs "
					"driver.", ni->mft_no);
			goto err_out;
		}
	}
	/* Read, map, and pin the page. */
	page = ntfs_map_page(mft_vi->i_mapping, index);
	if (likely(!IS_ERR(page))) {
		/* Catch multi sector transfer fixup errors. */
		if (likely(ntfs_is_mft_recordp((le32*)(page_address(page) +
				ofs)))) {
			ni->page = page;
			ni->page_ofs = ofs;
			return page_address(page) + ofs;
		}
		ntfs_error(vol->sb, "Mft record 0x%lx is corrupt.  "
				"Run chkdsk.", ni->mft_no);
		ntfs_unmap_page(page);
		page = ERR_PTR(-EIO);
		NVolSetErrors(vol);
	}
err_out:
	ni->page = NULL;
	ni->page_ofs = 0;
	return (void*)page;
}

/**
 * map_mft_record - map, pin and lock an mft record
 * @ni:		ntfs inode whose MFT record to map
 *
 * First, take the mrec_lock mutex.  We might now be sleeping, while waiting
 * for the mutex if it was already locked by someone else.
 *
 * The page of the record is mapped using map_mft_record_page() before being
 * returned to the caller.
 *
 * This in turn uses ntfs_map_page() to get the page containing the wanted mft
 * record (it in turn calls read_cache_page() which reads it in from disk if
 * necessary, increments the use count on the page so that it cannot disappear
 * under us and returns a reference to the page cache page).
 *
 * If read_cache_page() invokes ntfs_readpage() to load the page from disk, it
 * sets PG_locked and clears PG_uptodate on the page. Once I/O has completed
 * and the post-read mst fixups on each mft record in the page have been
 * performed, the page gets PG_uptodate set and PG_locked cleared (this is done
 * in our asynchronous I/O completion handler end_buffer_read_mft_async()).
 * ntfs_map_page() waits for PG_locked to become clear and checks if
 * PG_uptodate is set and returns an error code if not. This provides
 * sufficient protection against races when reading/using the page.
 *
 * However there is the write mapping to think about. Doing the above described
 * checking here will be fine, because when initiating the write we will set
 * PG_locked and clear PG_uptodate making sure nobody is touching the page
 * contents. Doing the locking this way means that the commit to disk code in
 * the page cache code paths is automatically sufficiently locked with us as
 * we will not touch a page that has been locked or is not uptodate. The only
 * locking problem then is them locking the page while we are accessing it.
 *
 * So that code will end up having to own the mrec_lock of all mft
 * records/inodes present in the page before I/O can proceed. In that case we
 * wouldn't need to bother with PG_locked and PG_uptodate as nobody will be
 * accessing anything without owning the mrec_lock mutex.  But we do need to
 * use them because of the read_cache_page() invocation and the code becomes so
 * much simpler this way that it is well worth it.
 *
 * The mft record is now ours and we return a pointer to it. You need to check
 * the returned pointer with IS_ERR() and if that is true, PTR_ERR() will return
 * the error code.
 *
 * NOTE: Caller is responsible for setting the mft record dirty before calling
 * unmap_mft_record(). This is obviously only necessary if the caller really
 * modified the mft record...
 * Q: Do we want to recycle one of the VFS inode state bits instead?
 * A: No, the inode ones mean we want to change the mft record, not we want to
 * write it out.
 */
MFT_RECORD *map_mft_record(ntfs_inode *ni)
{
	MFT_RECORD *m;

	ntfs_debug("Entering for mft_no 0x%lx.", ni->mft_no);

	/* Make sure the ntfs inode doesn't go away. */
	atomic_inc(&ni->count);

	/* Serialize access to this mft record. */
	mutex_lock(&ni->mrec_lock);

	m = map_mft_record_page(ni);
	if (likely(!IS_ERR(m)))
		return m;

	mutex_unlock(&ni->mrec_lock);
	atomic_dec(&ni->count);
	ntfs_error(ni->vol->sb, "Failed with error code %lu.", -PTR_ERR(m));
	return m;
}

/**
 * unmap_mft_record_page - unmap the page in which a specific mft record resides
 * @ni:		ntfs inode whose mft record page to unmap
 *
 * This unmaps the page in which the mft record of the ntfs inode @ni is
 * situated and returns. This is a NOOP if highmem is not configured.
 *
 * The unmap happens via ntfs_unmap_page() which in turn decrements the use
 * count on the page thus releasing it from the pinned state.
 *
 * We do not actually unmap the page from memory of course, as that will be
 * done by the page cache code itself when memory pressure increases or
 * whatever.
 */
static inline void unmap_mft_record_page(ntfs_inode *ni)
{
	BUG_ON(!ni->page);

	// TODO: If dirty, blah...
	ntfs_unmap_page(ni->page);
	ni->page = NULL;
	ni->page_ofs = 0;
	return;
}

/**
 * unmap_mft_record - release a mapped mft record
 * @ni:		ntfs inode whose MFT record to unmap
 *
 * We release the page mapping and the mrec_lock mutex which unmaps the mft
 * record and releases it for others to get hold of. We also release the ntfs
 * inode by decrementing the ntfs inode reference count.
 *
 * NOTE: If caller has modified the mft record, it is imperative to set the mft
 * record dirty BEFORE calling unmap_mft_record().
 */
void unmap_mft_record(ntfs_inode *ni)
{
	struct page *page = ni->page;

	BUG_ON(!page);

	ntfs_debug("Entering for mft_no 0x%lx.", ni->mft_no);

	unmap_mft_record_page(ni);
	mutex_unlock(&ni->mrec_lock);
	atomic_dec(&ni->count);
	/*
	 * If pure ntfs_inode, i.e. no vfs inode attached, we leave it to
	 * ntfs_clear_extent_inode() in the eO "Got %s and don't know how to handle...\n",
					response_name (data));
			break;

		default:
			if (lk->keycode[data] != KEY_RESERVED) {
				if (!test_bit (lk->keycode[data], lk->dev->key))
					input_report_key (lk->dev, lk->keycode[data], 1);
				else
					input_report_key (lk->dev, lk->keycode[data], 0);
				input_sync (lk->dev);
                        } else
                                printk (KERN_WARNING "%s: Unknown key with "
						"scancode 0x%02x on %s.\n",
						__FILE__, data, lk->name);
	}

	return IRQ_HANDLED;
}

/*
 * lkkbd_event() handles events from the input module.
 */
static int
lkkbd_event (struct input_dev *dev, unsigned int type, unsigned int code,
		int value)
{
	struct lkkbd *lk = input_get_drvdata (dev);
	unsigned char leds_on = 0;
	unsigned char leds_off = 0;

	switch (type) {
		case EV_LED:
			CHECK_LED (lk, leds_on, leds_off, LED_CAPSL, LK_LED_SHIFTLOCK);
			CHECK_LED (lk, leds_on, leds_off, LED_COMPOSE, LK_LED_COMPOSE);
			CHECK_LED (lk, leds_on, leds_off, LED_SCROLLL, LK_LED_SCROLLLOCK);
			CHECK_LED (lk, leds_on, leds_off, LED_SLEEP, LK_LED_WAIT);
			if (leds_on != 0) {
				lk->serio->write (lk->serio, LK_CMD_LED_ON);
				lk->serio->write (lk->serio, leds_on);
			}
			if (leds_off != 0) {
				lk->serio->write (lk->serio, LK_CMD_LED_OFF);
				lk->serio->write (lk->serio, leds_off);
			}
			return 0;

		case EV_SND:
			switch (code) {
				case SND_CLICK:
					if (value == 0) {
						DBG ("%s: Deactivating key clicks\n", __func__);
						lk->serio->write (lk->serio, LK_CMD_DISABLE_KEYCLICK);
						lk->serio->write (lk->serio, LK_CMD_DISABLE_CTRCLICK);
					} else {
						DBG ("%s: Activating key clicks\n", __func__);
						lk->serio->write (lk->serio, LK_CMD_ENABLE_KEYCLICK);
						lk->serio->write (lk->serio, volume_to_hw (lk->keyclick_volume));
						lk->serio->write (lk->serio, LK_CMD_ENABLE_CTRCLICK);
						lk->serio->write (lk->serio, volume_to_hw (lk->ctrlclick_volume));
					}
					return 0;

				case SND_BELL:
					if (value != 0)
						lk->serio->write (lk->serio, LK_CMD_SOUND_BELL);

					return 0;
			}
			break;

		default:
			printk (KERN_ERR "%s (): Got unknown type %d, code %d, value %d\n",
					__func__, type, code, value);
	}

	return -1;
}

/*
 * lkkbd_reinit() sets leds and beeps to a state the computer remembers they
 * were in.
 */
static void
lkkbd_reinit (struct work_struct *work)
{
	struct lkkbd *lk = container_of(work, struct lkkbd, tq);
	int division;
	unsigned char leds_on = 0;
	unsigned char leds_off = 0;

	/* Ask for ID */
	lk->serio->write (lk->serio, LK_CMD_REQUEST_ID);

	/* Reset parameters */
	lk->serio->write (lk->serio, LK_CMD_SET_DEFAULTS);

	/* Set LEDs */
	CHECK_LED (lk, leds_on, leds_off, LED_CAPSL, LK_LED_SHIFTLOCK);
	CHECK_LED (lk, leds_on, leds_off, LED_COMPOSE, LK_LED_COMPOSE);
	CHECK_LED (lk, leds_on, leds_off, LED_SCROLLL, LK_LED_SCROLLLOCK);
	CHECK_LED (lk, leds_on, leds_off, LED_SLEEP, LK_LED_WAIT);
	if (leds_on != 0) {
		lk->serio->write (lk->serio, LK_CMD_LED_ON);
		lk->serio->write (lk->serio, leds_on);
	}
	if (leds_off != 0) {
		lk->serio->write (lk->serio, LK_CMD_LED_OFF);
		lk->serio->write (lk->serio, leds_off);
	}

	/*
	 * Try to activate extended LK401 mode. This command will
	 * only work with a LK401 keyboard and grants access to
	 * LAlt, RAlt, RCompose and RShift.
	 */
	lk->serio->write (lk->serio, LK_CMD_ENABLE_LK401);

	/* Set all keys to UPDOWN mode */
	for (division = 1; division <= 14; division++)
		lk->serio->write (lk->serio, LK_CMD_SET_MODE (LK_MODE_UPDOWN,
					division));

	/* Enable bell and set volume */
	lk->serio->write (lk->serio, LK_CMD_ENABLE_BELL);
	lk->serio->write (lk->serio, volume_to_hw (lk->bell_volume));

	/* Enable/disable keyclick (and possibly set volume) */
	if (test_bit (SND_CLICK, lk->dev->snd)) {
		lk->serio->write (lk->serio, LK_CMD_ENABLE_KEYCLICK);
		lk->serio->write (lk->serio, volume_to_hw (lk->keyclick_volume));
		lk->serio->write (lk->serio, LK_CMD_ENABLE_CTRCLICK);
		lk->serio->write (lk->serio, volume_to_hw (lk->ctrlclick_volume));
	} else {
		lk->serio->write (lk->serio, LK_CMD_DISABLE_debug("Done 2.");
	*ntfs_ino = ni;
	return m;
unm_err_out:
	unmap_mft_record(ni);
	mutex_unlock(&base_ni->extent_lock);
	atomic_dec(&base_ni->count);
	/*
	 * If the extent inode was not attached to the base inode we need to
	 * release it or we will leak memory.
	 */
	if (destroy_ni)
		ntfs_clear_extent_inode(ni);
	return m;
}

#ifdef NTFS_RW

/**
 * __mark_mft_record_dirty - set the mft record and the page containing it dirty
 * @ni:		ntfs inode describing the mapped mft record
 *
 * Internal function.  Users should call mark_mft_record_dirty() instead.
 *
 * Set the mapped (extent) mft record of the (base or extent) ntfs inode @ni,
 * as well as the page containing the mft record, dirty.  Also, mark the base
 * vfs inode dirty.  This ensures that any changes to the mft record are
 * written out to disk.
 *
 * NOTE:  We only set I_DIRTY_SYNC and I_DIRTY_DATASYNC (and not I_DIRTY_PAGES)
 * on the base vfs inode, because even though file data may have been modified,
 * it is dirty in the inode meta data rather than the data page cache of the
 * inode, and thus there are no data pages that need writing out.  Therefore, a
 * full mark_inode_dirty() is overkill.  A mark_inode_dirty_sync(), on the
 * other hand, is not sufficient, because I_DIRTY_DATASYNC needs to be set to
 * ensure ->write_inode is called from generic_osync_inode() and this needs to
 * happen or the file data would not necessarily hit the device synchronously,
 * even though the vfs inode has the O_SYNC flag set.  Also, I_DIRTY_DATASYNC
 * simply "feels" better than just I_DIRTY_SYNC, since the file data has not
 * actually hit the block device yet, which is not what I_DIRTY_SYNC on its own
 * would suggest.
 */
void __mark_mft_record_dirty(ntfs_inode *ni)
{
	ntfs_inode *base_ni;

	ntfs_debug("Entering for inode 0x%lx.", ni->mft_no);
	BUG_ON(NInoAttr(ni));
	mark_ntfs_record_dirty(ni->page, ni->page_ofs);
	/* Determine the base vfs inode and mark it dirty, too. */
	mutex_lock(&ni->extent_lock);
	if (likely(ni->nr_extents >= 0))
		base_ni = ni;
	else
		base_ni = ni->ext.base_ntfs_ino;
	mutex_unlock(&ni->extent_lock);
	__mark_inode_dirty(VFS_I(base_ni), I_DIRTY_SYNC | I_DIRTY_DATASYNC);
}

static const char *ntfs_please_email = "Please email "
		"linux-ntfs-dev@lists.sourceforge.net and say that you saw "
		"this message.  Thank you.";

/**
 * ntfs_sync_mft_mirror_umount - synchronise an mft record to the mft mirror
 * @vol:	ntfs volume on which the mft record to synchronize resides
 * @mft_no:	mft record number of mft record to synchronize
 * @m:		mapped, mst protected (extent) mft record to synchronize
 *
 * Write the mapped, mst protected (extent) mft record @m with mft record
 * number @mft_no to the mft mirror ($MFTMirr) of the ntfs volume @vol,
 * bypassing the page cache and the $MFTMirr inode itself.
 *
 * This function is only for use at umount time when the mft mirror inode has
 * already been disposed off.  We BUG() if we are called while the mft mirror
 * inode is still attached to the volume.
 *
 * On success return 0.  On error return -errno.
 *
 * NOTE:  This function is not implemented yet as I am not convinced it can
 * actually be triggered considering the sequence of commits we do in super.c::
 * ntfs_put_super().  But just in case we provide this place holder as the
 * alternative would be either to BUG() or to get a NULL pointer dereference
 * and Oops.
 */
static int ntfs_sync_mft_mirror_umount(ntfs_volume *vol,
		const unsigned long mft_no, MFT_RECORD *m)
{
	BUG_ON(vol->mftmirr_ino);
	ntfs_error(vol->sb, "Umount time mft mirror syncing is not "
			"implemented yet.  %s", ntfs_please_email);
	return -EOPNOTSUPP;
}

/**
 * ntfs_sync_mft_mirror - synchronize an mft record to the mft mirror
 * @vol:	ntfs volume on which the mft record to synchronize resides
 * @mft_no:	mft record number of mft record to synchronize
 * @m:		mapped, mst protected (extent) mft record to synchronize
 * @sync:	if true, wait for i/o completion
 *
 * Write the mapped, mst protected (extent) mft record @m with mft record
 * number @mft_no to the mft mirror ($MFTMirr) of the ntfs volume @vol.
 *
 * On success return 0.  On error return -errno and set the volume errors flag
 * in the ntfs volume @vol.
 *
 * NOTE:  We always perform synchronous i/o and ignore the @sync parameter.
 *
 * TODO:  If @sync is false, want to do truly asynchronous i/o, i.e. just
 * schedule i/o via ->writepage or do it via kntfsd or whatever.
 */
int ntfs_sync_mft_mirror(ntfs_volume *vol, const unsigned long mft_no,
		MFT_RECORD *m, int sync)
{
	struct page *page;
	unsigned int blocksize = vol->sb->s_blocksize;
	int max_bhs = vol->mft_record_size / blocksize;
	struct buffer_head *bhs[max_bhs];
	struct buffer_head *bh, *head;
	u8 *kmirr;
	runlist_element *rl;
	unsigned int block_start, block_end, m_start, m_end, page_ofs;
	int i_bhs, nr_bhs, err = 0;
	unsigned char blocksize_bits = vol->sb->s_blocksize_bits;

	ntfs_debug("Entering for inode 0x%lx.", mft_no);
	BUG_ON(!max_bhs);
	if (unlikely(!vol->mftmirr_ino)) {
		/* This could happen during umount... */
		err = ntfs_sync_mft_mirror_umount(vol, mft_no, m);
		if (likely(!err))
			return err;
		goto err_out;
	}
	/* Get the page containing the mirror copy of the mft record @m. */
	page = ntfs_map_page(vol->mftmirr_ino->i_mapping, mft_no >>
			(PAGE_CACHE_SHIFT - vol->mft_record_size_bits));
	if (IS_ERR(page)) {
		ntfs_error(vol->sb, "Failed to map mft mirror page.");
		err = PTR_ERR(page);
		goto err_out;
	}
	lock_page(page);
	BUG_ON(!PageUptodate(page));
	ClearPageUptodate(page);
	/* Offset of the mft mirror record inside the page. */
	page_ofs = (mft_no << vol->mft_record_size_bits) & ~PAGE_CACHE_MASK;
	/* The address in the page of the mirror copy of the mft record @m. */
	kmirr = page_address(page) + page_ofs;
	/* Copy the mst protected mft record to the mirror. */
	memcpy(kmirr, m, vol->mft_record_size);
	/* Create uptodate buffers if not present. */
	if (unlikely(!page_has_buffers(page))) {
		struct buffer_head *tail;

		bh = head = alloc_page_buffers(page, blocksize, 1);
		do {
			set_buffer_uptodate(bh);
			tail = bh;
			bh = bh->b_this_page;
		} while (bh);
		tail->b_this_page = head;
		attach_page_buffers(page, head);
	}
	bh = head = page_buffers(page);
	BUG_ON(!bh);
	rl = NULL;
	nr_bhs = 0;
	block_start = 0;
	m_start = kmirr - (u8*)page_address(page);
	m_end = m_start + vol->mft_record_size;
	do {
		block_end = block_start + blocksize;
		/* If the buffer is outside the mft record, skip it. */
		if (block_end <= m_start)
			continue;
		if (unlikely(block_start >= m_end))
			break;
		/* Need to map the buffer if it is not mapped already. */
		if (unlikely(!buffer_mapped(bh))) {
			VCN vcn;
			LCN lcn;
			unsigned int vcn_ofs;

			bh->b_bdev = vol->sb->s_bdev;
			/* Obtain the vcn and offset of the current block. */
			vcn = ((VCN)mft_no << vol->mft_record_size_bits) +
					(block_start - m_start);
			vcn_ofs = vcn & vol->cluster_size_mask;
			vcn >>= vol->cluster_size_bits;
			if (!rl) {
				down_read(&NTFS_I(vol->mftmirr_ino)->
						runlist.lock);
				rl = NTFS_I(vol->mftmirr_ino)->runlist.rl;
				/*
				 * $MFTMirr always has the whole of its runlist
				 * in memory.
				 */
				BUG_ON(!rl);
			}
			/* Seek to element containing target vcn. */
			while (rl->length && rl[1].vcn <= vcn)
				rl++;
			lcn = ntfs_rl_vcn_to_lcn(rl, vcn);
			/* For $MFTMirr, only lcn >= 0 is a successful remap. */
			if (likely(lcn >= 0)) {
				/* Setup buffer head to correct block. */
				bh->b_blocknr = ((lcn <<
						vol->cluster_size_bits) +
						vcn_ofs) >> blocksize_bits;
				set_buffer_mapped(bh);
			} else {
				bh->b_blocknr = -1;
				ntfs_error(vol->sb, "Cannot write mft mirror "
						"record 0x%lx because its "
						"location on disk could not "
						"be determined (error code "
						"%lli).", mft_no,
						(long long)lcn);
				err = -EIO;
			}
		}
		BUG_ON(!buffer_uptodate(bh));
		BUG_ON(!nr_bhs && (m_start != block_start));
		BUG_ON(nr_bhs >= max_bhs);
		bhs[nr_bhs++] = bh;
		BUG_ON((nr_bhs >= max_bhs) && (m_end != block_end));
	} while (block_start = block_end, (bh = bh->b_this_page) != head);
	if (unlikely(rl))
		up_read(&NTFS_I(vol->mftmirr_ino)->runlist.lock);
	if (likely(!err)) {
		/* Lock buffers and start synchronous write i/o on them. */
		for (i_bhs = 0; i_bhs < nr_bhs; i_bhs++) {
			struct buffer_head *tbh = bhs[i_bhs];

			if (!trylock_buffer(tbh))
				BUG();
			BUG_ON(!buffer_uptodate(tbh));
			clear_buffer_dirty(tbh);
			get_bh(tbh);
			tbh->b_end_io = end_buffer_write_sync;
			submit_bh(WRITE, tbh);
		}
		/* Wait on i/o completion of buffers. */
		for (i_bhs = 0; i_bhs < nr_bhs; i_bhs++) {
			struct buffer_head *tbh = bhs[i_bhs];

			wait_on_buffer(tbh);
			if (unlikely(!buffer_uptodate(tbh))) {
				err = -EIO;
				/*
				 * Set the buffer uptodate so the page and
				 * buffer states do not become out of sync.
				 */
				set_buffer_uptodate(tbh);
			}
		}
	} else /* if (unlikely(err)) */ {
		/* Clean the buffers. */
		for (i_bhs = 0; i_bhs < nr_bhs; i_bhs++)
			clear_buffer_dirty(bhs[i_bhs]);
	}
	/* Current state: all buffers are clean, unlocked, and uptodate. */
	/* Remove the mst protection fixups again. */
	post_write_mst_fixup((NTFS_RECORD*)kmirr);
	flush_dcache_page(page);
	SetPageUptodate(page);
	unlock_page(page);
	ntfs_unmap_page(page);
	if (likely(!err)) {
		ntfs_debug("Done.");
	} else {
		ntfs_error(vol->sb, "I/O error while writing mft mirror "
				"record 0x%lx!", mft_no);
err_out:
		ntfs_error(vol->sb, "Failed to synchronize $MFTMirr (error "
				"code %i).  Volume will be left marked dirty "
				"on umount.  Run ntfsfix on the partition "
				"after umounting to correct this.", -err);
		NVolSetErrors(vol);
	}
	return err;
}

/**
 * write_mft_record_nolock - write out a mapped (extent) mft record
 * @ni:		ntfs inode describing the mapped (extent) mft record
 * @m:		mapped (extent) mft record to write
 * @sync:	if true, wait for i/o completion
 *
 * Write the mapped (extent) mft record @m described by the (regular or extent)
 * ntfs inode @ni to backing store.  If the mft record @m has a counterpart in
 * the mft mirror, that is also updated.
 *
 * We only write the mft record if the ntfs inode @ni is dirty and the first
 * buffer belonging to its mft record is dirty, too.  We ignore the dirty state
 * of subsequent buffers because we could have raced with
 * fs/ntfs/aops.c::mark_ntfs_record_dirty().
 *
 * On success, clean the mft record and return 0.  On error, leave the mft
 * record dirty and return -errno.
 *
 * NOTE:  We always perform synchronous i/o and ignore the @sync parameter.
 * However, if the mft record has a counterpart in the mft mirror and @sync is
 * true, we write the mft record, wait for i/o completion, and only then write
 * the mft mirror copy.  This ensures that if the system crashes either the mft
 * or the mft mirror will contain a self-consistent mft record @m.  If @sync is
 * false on the other hand, we start i/o on both and then wait for completion
 * on them.  This provides a speedup but no longer guarantees that you will end
 * up with a self-consistent mft record in the case of a crash but if you asked
 * for asynchronous writing you probably do not care about that anyway.
 *
 * TODO:  If @sync is false, want to do truly asynchronous i/o, i.e. just
 * schedule i/o via ->writepage or do it via kntfsd or whatever.
 */
int write_mft_record_nolock(ntfs_inode *ni, MFT_RECORD *m, int sync)
{
	ntfs_volume *vol = ni->vol;
	struct page *page = ni->page;
	unsigned int blocksize = vol->sb->s_blocksize;
	unsigned char blocksize_bits = vol->sb->s_blocksize_bits;
	int max_bhs = vol->mft_record_size / blocksize;
	struct buffer_head *bhs[max_bhs];
	struct buffer_head *bh, *head;
	runlist_element *rl;
	unsigned int block_start, block_end, m_start, m_end;
	int i_bhs, nr_bhs, err = 0;

	ntfs_debug("Entering for inode 0x%lx.", ni->mft_no);
	BUG_ON(NInoAttr(ni));
	BUG_ON(!max_bhs);
	BUG_ON(!PageLocked(page));
	/*
	 * If the ntfs_inode is clean no need to do anything.  If it is dirty,
	 * mark it as clean now so that it can be redirtied later on if needed.
	 * There is no danger of races since the caller is holding the locks
	 * for the mft record @m and the page it is in.
	 */
	if (!NInoTestClearDirty(ni))
		goto done;
	bh = head = page_buffers(page);
	BUG_ON(!bh);
	rl = NULL;
	nr_bhs = 0;
	block_start = 0;
	m_start = ni->page_ofs;
	m_end = m_start + vol->mft_record_size;
	do {
		block_end = block_start + blocksize;
		/* If the buffer is outside the mft record, skip it. */
		if (block_end <= m_start)
			continue;
		if (unlikely(block_start >= m_end))
			break;
		/*
		 * If this block is not the first one in the record, we ignore
		 * the buffer's dirty state because we could have raced with a
		 * parallel mark_ntfs_record_dirty().
		 */
		if (block_start == m_start) {
			/* This block is the first one in the record. */
			if (!buffer_dirty(bh)) {
				BUG_ON(nr_bhs);
				/* Clean records are not written out. */
				break;
			}
		}
		/* Need to map the buffer if it is not mapped already. */
		if (unlikely(!buffer_mapped(bh))) {
			VCN vcn;
			LCN lcn;
			unsigned int vcn_ofs;

			bh->b_bdev = vol->sb->s_bdev;
			/* Obtain the vcn and offset of the current block. */
			vcn = ((VCN)ni->mft_no << vol->mft_record_size_bits) +
					(block_start - m_start);
			vcn_ofs = vcn & vol->cluster_size_mask;
			vcn >>= vol->cluster_size_bits;
			if (!rl) {
				down_read(&NTFS_I(vol->mft_ino)->runlist.lock);
				rl = NTFS_I(vol->mft_ino)->runlist.rl;
				BUG_ON(!rl);
			}
			/* Seek to element containing target vcn. */
			while (rl->length && rl[1].vcn <= vcn)
				rl++;
			lcn = ntfs_rl_vcn_to_lcn(rl, vcn);
			/* For $MFT, only lcn >= 0 is a successful remap. */
			if (likely(lcn >= 0)) {
				/* Setup buffer head to correct block. */
				bh->b_blocknr = ((lcn <<
						vol->cluster_size_bits) +
						vcn_ofs) >> blocksize_bits;
				set_buffer_mapped(bh);
			} else {
				bh->b_blocknr = -1;
				ntfs_error(vol->sb, "Cannot write mft record "
						"0x%lx because its location "
						"on disk could not be "
						"determined (error code %lli).",
						ni->mft_no, (long long)lcn);
				err = -EIO;
			}
		}
		BUG_ON(!buffer_uptodate(bh));
		BUG_ON(!nr_bhs && (m_start != block_start));
		BUG_ON(nr_bhs >= max_bhs);
		bhs[nr_bhs++] = bh;
		BUG_ON((nr_bhs >= max_bhs) && (m_end != block_end));
	} while (block_start = block_end, (bh = bh->b_this_page) != head);
	if (unlikely(rl))
		up_read(&NTFS_I(vol->mft_ino)->runlist.lock);
	if (!nr_bhs)
		goto done;
	if (unlikely(err))
		goto cleanup_out;
	/* Apply the mst protection fixups. */
	err = pre_write_mst_fixup((NTFS_RECORD*)m, vol->mft_record_size);
	if (err) {
		ntfs_error(vol->sb, "Failed to apply mst fixups!");
		goto cleanup_out;
	}
	flush_dcache_mft_record_page(ni);
	/* Lock buffers and start synchronous write i/o on them. */
	for (i_bhs = 0; i_bhs < nr_bhs; i_bhs++) {
		struct buffer_head *tbh = bhs[i_bhs];

		if (!trylock_buffer(tbh))
			BUG();
		BUG_ON(!buffer_uptodate(tbh));
		clear_buffer_dirty(tbh);
		get_bh(tbh);
		tbh->b_end_io = end_buffer_write_sync;
		submit_bh(WRITE, tbh);
	}
	/* Synchronize the mft mirror now if not @sync. */
	if (!sync && ni->mft_no < vol->mftmirr_size)
		ntfs_sync_mft_mirror(vol, ni->mft_no, m, sync);
	/* Wait on i/o completion of buffers. */
	for (i_bhs = 0; i_bhs < nr_bhs; i_bhs++) {
		struct buffer_head *tbh = bhs[i_bhs];

		wait_on_buffer(tbh);
		if (unlikely(!buffer_uptodate(tbh))) {
			err = -EIO;
			/*
			 * Set the buffer uptodate so the page and buffer
			 * states do not become out of sync.
			 */
			if (PageUptodate(page))
				set_buffer_uptodate(tbh);
		}
	}
	/* If @sync, now synchronize the mft mirror. */
	if (sync && ni->mft_no < vol->mftmirr_size)
		ntfs_sync_mft_mirror(vol, ni->mft_no, m, sync);
	/* Remove the mst protection fixups again. */
	post_write_mst_fixup((NTFS_RECORD*)m);
	flush_dcache_mft_record_page(ni);
	if (unlikely(err)) {
		/* I/O error during writing.  This is really bad! */
		ntfs_error(vol->sb, "I/O error while writing mft record "
				"0x%lx!  Marking base inode as bad.  You "
				"should unmount the volume and run chkdsk.",
				ni->mft_no);
		goto err_out;
	}
done:
	ntfs_debug("Done.");
	return 0;
cleanup_out:
	/* Clean the buffers. */
	for (i_bhs = 0; i_bhs < nr_bhs; i_bhs++)
		clear_buffer_dirty(bhs[i_bhs]);
err_out:
	/*
	 * Current state: all buffers are clean, unlocked, and uptodate.
	 * The caller should mark the base inode as bad so that no more i/o
	 * happens.  ->clear_inode() will still be invoked so all extent inodes
	 * and other allocated memory will be freed.
	 */
	if (err == -ENOMEM) {
		ntfs_error(vol->sb, "Not enough memory to write mft record.  "
				"Redirtying so the write is retried later.");
		mark_mft_record_dirty(ni);
		err = 0;
	} else
		NVolSetErrors(vol);
	return err;
}

/**
 * ntfs_may_write_mft_record - check if an mft record may be written out
 * @vol:	[IN]  ntfs volume on which the mft record to check resides
 * @mft_no:	[IN]  mft record number of the mft record to check
 * @m:		[IN]  mapped mft record to check
 * @locked_ni:	[OUT] caller has to unlock this ntfs inode if one is returned
 *
 * Check if the mapped (base or extent) mft record @m with mft record number
 * @mft_no belonging to the ntfs volume @vol may be written out.  If necessary
 * and possible the ntfs inode of the mft record is locked and the base vfs
 * inode is pinned.  The locked ntfs inode is then returned in @locked_ni.  The
 * caller is responsible for unlocking the ntfs inode and unpinning the base
 * vfs inode.
 *
 * Return 'true' if the mft record may be written out and 'false' if not.
 *
 * The caller has locked the page and cleared the uptodate flag on it which
 * means that we can safely write out any dirty mft records that do not have
 * their inodes in icache as determined by ilookup5() as anyone
 * opening/creating such an inode would block when attempting to map the mft
 * record in read_cache_page() until we are finished with the write out.
 *
 * Here is a description of the tests we perform:
 *
 * If the inode is found in icache we know the mft record must be a base mft
 * record.  If it is dirty, we do not write it and return 'false' as the vfs
 * inode write paths will result in the access times being updated which would
 * cause the base mft record to be redirtied and written out again.  (We know
 * the access time update will modify the base mft record because Windows
 * chkdsk complains if the standard information attribute is not in the base
 * mft record.)
 *
 * If the inode is in icache and not dirty, we attempt to lock the mft record
 * and if we find the lock was already taken, it is not safe to write the mft
 * record and we return 'false'.
 *
 * If we manage to obtain the lock we have exclusive access to the mft record,
 * which also allows us safe writeout of the mft record.  We then set
 * @locked_ni to the locked ntfs inode and return 'true'.
 *
 * Note we cannot just lock the mft record and sleep while waiting for the lock
 * because this would deadlock due to lock reversal (normally the mft record is
 * locked before the page is locked but we already have the page locked here
 * when we try to lock the mft record).
 *
 * If the inode is not in icache we need to perform further checks.
 *
 * If the mft record is not a FILE record or it is a base mft record, we can
 * safely write it and return 'true'.
 *
 * We now know the mft record is an extent mft record.  We check if the inode
 * corresponding to its base mft record is in icache and obtain a reference to
 * it if it is.  If it is not, we can safely write it and return 'true'.
 *
 * We now have the base inode for the extent mft record.  We check if it has an
 * ntfs inode for the extent mft record attached and if not it is safe to write
 * the extent mft record and we return 'true'.
 *
 * The ntfs inode for the extent mft record is attached to the base inode so we
 * attempt to lock the extent mft record and if we find the lock was already
 * taken, it is not safe to write the extent mft record and we return 'false'.
 *
 * If we manage to obtain the lock we have exclusive access to the extent mft
 * record, which also allows us safe writeout of the extent mft record.  We
 * set the ntfs inode of the extent mft record clean and then set @locked_ni to
 * the now locked ntfs inode and return 'true'.
 *
 * Note, the reason for actually writing dirty mft records here and not just
 * relying on the vfs inode dirty code paths is that we can have mft records
 * modified without them ever having actual inodes in memory.  Also we can have
 * dirty mft records with clean ntfs inodes in memory.  None of the described
 * cases would result in the dirty mft records being written out if we only
 * relied on the vfs inode dirty code paths.  And these cases can really occur
 * during allocation of new mft records and in particular when the
 * initialized_size of the $MFT/$DATA attribute is extended and the new space
 * is initialized using ntfs_mft_record_format().  The clean inode can then
 * appear if the mft record is reused for a new inode before it got written
 * out.
 */
bool ntfs_may_write_mft_record(ntfs_volume *vol, const unsigned long mft_no,
		const MFT_RECORD *m, ntfs_inode **locked_ni)
{
	struct super_block *sb = vol->sb;
	struct inode *mft_vi = vol->mft_ino;
	struct inode *vi;
	ntfs_inode *ni, *eni, **extent_nis;
	int i;
	ntfs_attr na;

	ntfs_debug("Entering for inode 0x%lx.", mft_no);
	/*
	 * Normally we do not return a locked inode so set @locked_ni to NULL.
	 */
	BUG_ON(!locked_ni);
	*locked_ni = NULL;
	/*
	 * Check if the inode corresponding to this mft record is in the VFS
	 * inode cache and obtain a reference to it if it is.
	 */
	ntfs_debug("Looking for inode 0x%lx in icache.", mft_no);
	na.mft_no = mft_no;
	na.name = NULL;
	na.name_len = 0;
	na.type = AT_UNUSED;
	/*
	 * Optimize inode 0, i.e. $MFT itself, since we have it in memory and
	 * we get here for it rather often.
	 */
	if (!mft_no) {
		/* Balance the below iput(). */
		vi = igrab(mft_vi);
		BUG_ON(vi != mft_vi);
	} else {
		/*
		 * Have to use ilookup5_nowait() since ilookup5() waits for the
		 * inode lock which causes ntfs to deadlock when a concurrent
		 * inode write via the inode dirty code paths and the page
		 * dirty code path of the inode dirty code path when writing
		 * $MFT occurs.
		 */
		vi = ilookup5_nowait(sb, mft_no, (test_t)ntfs_test_inode, &na);
	}
	if (vi) {
		ntfs_debug("Base inode 0x%lx is in icache.", mft_no);
		/* The inode is in icache. */
		ni = NTFS_I(vi);
		/* Take a reference to the ntfs inode. */
		atomic_inc(&ni->count);
		/* If the inode is dirty, do not write this record. */
		if (NInoDirty(ni)) {
			ntfs_debug("Inode 0x%lx is dirty, do not write it.",
					mft_no);
			atomic_dec(&ni->count);
			iput(vi);
			return false;
		}
		ntfs_debug("Inode 0x%lx is not dirty.", mft_no);
		/* The inode is not dirty, try to take the mft record lock. */
		if (unlikely(!mutex_trylock(&ni->mrec_lock))) {
			ntfs_debug("Mft record 0x%lx is already locked, do "
					"not write it.", mft_no);
			atomic_dec(&ni->count);
			iput(vi);
			return false;
		}
		ntfs_debug("Managed to lock mft record 0x%lx, write it.",
				mft_no);
		/*
		 * The write has to occur while we hold the mft record lock so
		 * return the locked ntfs inode.
		 */
		*locked_ni = ni;
		return true;
	}
	ntfs_debug("Inode 0x%lx is not in icache.", mft_no);
	/* The inode is not in icache. */
	/* Write the record if it is not a mft record (type "FILE"). */
	if (!ntfs_is_mft_record(m->magic)) {
		ntfs_debug("Mft record 0x%lx is not a FILE record, write it.",
				mft_no);
		return true;
	}
	/* Write the mft record if it is a base inode. */
	if (!m->base_mft_record) {
		ntfs_debug("Mft record 0x%lx is a base record, write it.",
				mft_no);
		return true;
	}
	/*
	 * This is an extent mft record.  Check if the inode corresponding to
	 * its base mft record is in icache and obtain a reference to it if it
	 * is.
	 */
	na.mft_no = MREF_LE(m->base_mft_record);
	ntfs_debug("Mft record 0x%lx is an extent record.  Looking for base "
			"inode 0x%lx in icache.", mft_no, na.mft_no);
	if (!na.mft_no) {
		/* Balance the below iput(). */
		vi = igrab(mft_vi);
		BUG_ON(vi != mft_vi);
	} else
		vi = ilookup5_nowait(sb, na.mft_no, (test_t)ntfs_test_inode,
				&na);
	if (!vi) {
		/*
		 * The base inode is not in icache, write this extent mft
		 * record.
		 */
		ntfs_debug("Base inode 0x%lx is not in icache, write the "
				"extent record.", na.mft_no);
		return true;
	}
	ntfs_debug(one will
 * be allocated.
 */
int uwb_dev_add(struct uwb_dev *uwb_dev, struct device *parent_dev,
		struct uwb_rc *parent_rc)
{
	int result;
	struct device *dev;

	BUG_ON(uwb_dev == NULL);
	BUG_ON(parent_dev == NULL);
	BUG_ON(parent_rc == NULL);

	mutex_lock(&uwb_dev->mutex);
	dev = &uwb_dev->dev;
	uwb_dev->rc = parent_rc;
	result = __uwb_dev_sys_add(uwb_dev, parent_dev);
	if (result < 0)
		printk(KERN_ERR "UWB: unable to register dev %s with sysfs: %d\n",
		       dev_name(dev), result);
	mutex_unlock(&uwb_dev->mutex);
	return result;
}


void uwb_dev_rm(struct uwb_dev *uwb_dev)
{
	mutex_lock(&uwb_dev->mutex);
	__uwb_dev_sys_rm(uwb_dev);
	mutex_unlock(&uwb_dev->mutex);
}


static
int __uwb_dev_try_get(struct device *dev, void *__target_uwb_dev)
{
	struct uwb_dev *target_uwb_dev = __target_uwb_dev;
	struct uwb_dev *uwb_dev = to_uwb_dev(dev);
	if (uwb_dev == target_uwb_dev) {
		uwb_dev_get(uwb_dev);
		return 1;
	} else
		return 0;
}


/**
 * Given a UWB device descriptor, validate and refcount it
 *
 * @returns NULL if the device does not exist or is quiescing; the ptr to
 *               it otherwise.
 */
struct uwb_dev *uwb_dev_try_get(struct uwb_rc *rc, struct uwb_dev *uwb_dev)
{
	if (uwb_dev_for_each(rc, __uwb_dev_try_get, uwb_dev))
		return uwb_dev;
	else
		return NULL;
}
EXPORT_SYMBOL_GPL(uwb_dev_try_get);


/**
 * Remove a device from the system [grunt for other functions]
 */
int __uwb_dev_offair(struct uwb_dev *uwb_dev, struct uwb_rc *rc)
{
	struct device *dev = &uwb_dev->dev;
	char macbuf[UWB_ADDR_STRSIZE], devbuf[UWB_ADDR_STRSIZE];

	uwb_mac_addr_print(macbuf, sizeof(macbuf), &uwb_dev->mac_addr);
	uwb_dev_addr_print(devbuf, sizeof(devbuf), &uwb_dev->dev_addr);
	dev_info(dev, "uwb device (mac %s dev %s) disconnected from %s %s\n",
		 macbuf, devbuf,
		 rc ? rc->uwb_dev.dev.parent->bus->name : "n/a",
		 rc ? dev_name(rc->uwb_dev.dev.parent) : "");
	uwb_dev_rm(uwb_dev);
	list_del(&uwb_dev->bce->node);
	uwb_bce_put(uwb_dev->bce);
	uwb_dev_put(uwb_dev);	/* for the creation in _onair() */

	return 0;
}


/**
 * A device went off the air, clean up after it!
 *
 * This is called by the UWB Daemon (through the beacon purge function
 * uwb_bcn_cache_purge) when it is detected that a device has been in
 * radio silence for a while.
 *
 * If this device is actually a local radio controller we don't need
 * to go through the offair process, as it is not registered as that.
 *
 * NOTE: uwb_bcn_cache.mutex is held!
 */
void uwbd_dev_offair(struct uwb_beca_e *bce)
{
	struct uwb_dev *uwb_dev;

	uwb_dev = bce->uwb_dev;
	if (uwb_dev) {
		uwb_notify(uwb_dev->rc, uwb_dev, UWB_NOTIF_OFFAIR);
		__uwb_dev_offair(uwb_dev, uwb_dev->rc);
	}
}


/**
 * A device went on the air, start it up!
 *
 * This is called by the UWB Daemon when it is detected that a device
 * has popped up in the radio range of the radio controller.
 *
 * It will just create the freaking device, register the beacon and
 * stuff and yatla, done.
 *
 *
 * NOTE: uwb_beca.mutex is held, bce->mutex is held
 */
void uwbd_dev_onair(struct uwb_rc *rc, struct uwb_beca_e *bce)
{
	int result;
	struct device *dev = &rc->uwb_dev.dev;
	struct uwb_dev *uwb_dev;
	char macbuf[UWB_ADDR_STRSIZE], devbuf[UWB_ADDR_STRSIZE];

	uwb_mac_addr_print(macbuf, sizeof(macbuf), bce->mac_addr);
	uwb_dev_addr_print(devbuf, sizeof(devbuf), &bce->dev_addr);
	uwb_dev = kzalloc(sizeof(struct uwb_dev), GFP_KERNEL);
	if (uwb_dev == NULL) {
		dev_err(dev, "new device %s: Cannot allocate memory\n",
			macbuf);
		return;
	}
	uwb_dev_init(uwb_dev);		/* This sets refcnt to one, we own it */
	uwb_dev->mac_addr = *bce->mac_addr;
	uwb_dev->dev_addr = bce->dev_addr;
	dev_set_name(&uwb_dev->dev, macbuf);
	result = uwb_dev_add(uwb_dev, &rc->uwb_dev.dev, rc);
	if (result < 0) {
		dev_err(dev, "new device %s: cannot instantiate device\n",
			macbuf);
		goto error_dev_add;
	}
	/* plug the beacon cache */
	bce->uwb_dev = uwb_dev;
	uwb_dev->bce = bce;
	uwb_bce_get(bce);		/* released in uwb_dev_sys_release() */
	dev_info(dev, "uwb device (mac %s dev %s) connected to %s %s\n",
		 macbuf, devbuf, rc->uwb_dev.dev.parent->bumap search: pass %u, pass_start 0x%llx, "
			"pass_end 0x%llx, data_pos 0x%llx.", pass,
			(long long)pass_start, (long long)pass_end,
			(long long)data_pos);
	/* Loop until a free mft record is found. */
	for (; pass <= 2;) {
		/* Cap size to pass_end. */
		ofs = data_pos >> 3;
		page_ofs = ofs & ~PAGE_CACHE_MASK;
		size = PAGE_CACHE_SIZE - page_ofs;
		ll = ((pass_end + 7) >> 3) - ofs;
		if (size > ll)
			size = ll;
		size <<= 3;
		/*
		 * If we are still within the active pass, search the next page
		 * for a zero bit.
		 */
		if (size) {
			page = ntfs_map_page(mftbmp_mapping,
					ofs >> PAGE_CACHE_SHIFT);
			if (IS_ERR(page)) {
				ntfs_error(vol->sb, "Failed to read mft "
						"bitmap, aborting.");
				return PTR_ERR(page);
			}
			buf = (u8*)page_address(page) + page_ofs;
			bit = data_pos & 7;
			data_pos &= ~7ull;
			ntfs_debug("Before inner for loop: size 0x%x, "
					"data_pos 0x%llx, bit 0x%llx", size,
					(long long)data_pos, (long long)bit);
			for (; bit < size && data_pos + bit < pass_end;
					bit &= ~7ull, bit += 8) {
				byte = buf + (bit >> 3);
				if (*byte == 0xff)
					continue;
				b = ffz((unsigned long)*byte);
				if (b < 8 && b >= (bit & 7)) {
					ll = data_pos + (bit & ~7ull) + b;
					if (unlikely(ll > (1ll << 32))) {
						ntfs_unmap_page(page);
						return -ENOSPC;
					}
					*byte |= 1 << b;
					flush_dcache_page(page);
					set_page_dirty(page);
					ntfs_unmap_page(page);
					ntfs_debug("Done.  (Found and "
							"allocated mft record "
							"0x%llx.)",
							(long long)ll);
					return ll;
				}
			}
			ntfs_debug("After inner for loop: size 0x%x, "
					"data_pos 0x%llx, bit 0x%llx", size,
					(long long)data_pos, (long long)bit);
			data_pos += size;
			ntfs_unmap_page(page);
			/*
			 * If the end of the pass has not been reached yet,
			 * continue searching the mft bitmap for a zero bit.
			 */
			if (data_pos < pass_end)
				continue;
		}
		/* Do the next pass. */
		if (++pass == 2) {
			/*
			 * Starting the second pass, in which we scan the first
			 * part of the zone which we omitted earlier.
			 */
			pass_end = pass_start;
			data_pos = pass_start = 24;
			ntfs_debug("pass %i, pass_start 0x%llx, pass_end "
					"0x%llx.", pass, (long long)pass_start,
					(long long)pass_end);
			if (data_pos >= pass_end)
				break;
		}
	}
	/* No free mft records in currently initialized mft bitmap. */
	ntfs_debug("Done.  (No free mft records left in currently initialized "
			"mft bitmap.)");
	return -ENOSPC;
}

/**
 * ntfs_mft_bitmap_extend_allocation_nolock - extend mft bitmap by a cluster
 * @vol:	volume on which to extend the mft bitmap attribute
 *
 * Extend the mft bitmap attribute on the ntfs volume @vol by one cluster.
 *
 * Note: Only changes allocated_size, i.e. does not touch initialized_size or
 * data_size.
 *
 * Return 0 on success and -errno on error.
 *
 * Locking: - Caller must hold vol->mftbmp_lock for writing.
 *	    - This function takes NTFS_I(vol->mftbmp_ino)->runlist.lock for
 *	      writing and releases it before returning.
 *	    - This function takes vol->lcnbmp_lock for writing and releases it
 *	      before returning.
 */
static int ntfs_mft_bitmap_extend_allocation_nolock(ntfs_volume *vol)
{
	LCN lcn;
	s64 ll;
	unsigned long flags;
	struct page *page;
	ntfs_inode *mft_ni, *mftbmp_ni;
	runlist_element *rl, *rl2 = NULL;
	ntfs_attr_search_ctx *ctx = NULL;
	MFT_RECORD *mrec;
	ATTR_RECORD *a = NULL;
	int ret, mp_size;
	u32 old_alen = 0;
	u8 *b, tb;
	struct {
		u8 added_cluster:1;
		u8 added_run:1;
		u8 mp_rebuilt:1;
	} status = { 0, 0, 0 };

	ntfs_debug("Extending mft bitmap allocation.");
	mft_ni = NTFS_I(vol->mft_ino);
	mftbmp_ni = NTFS_I(vol->mftbmp_ino);
	/*
	 * Determine the last lcn of the mft bitmap.  The allocated size of the
	 * mft bitmap cannot be zero so we are ok to do this.
	 */
	down_write(&mftbmp_ni->runlist.lock);
	read_lock_irqsave(&mftbmp_ni->size_lock, flags);
	ll = mftbmp_ni->allocated_size;
	read_unlock_irqrestore(&mftbmp_ni->size_lock, flags);
	rl = ntfs_attr_find_vcn_nolock(mftbmp_ni,
			(ll - 1) >> vol->cluster_size_bits, NULL);
	if (unlikely(IS_ERR(rl) || !rl->length || rl->lcn < 0)) {
		up_write(&mftbmp_ni->runlist.lock);
		ntfs_error(vol->sb, "Failed to determine last allocated "
				"cluster of mft bitmap attribute.");
		if (!IS_ERR(rl))
			ret = -EIO;
		else
			ret = PTR_ERR(rl);
		return ret;
	}
	lcn = rl->lcn + rl->length;
	ntfs_debug("Last lcn of mft bitmap attribute is 0x%llx.",
			(long long)lcn);
	/*
	 * Attempt to get the cluster following the last allocated cluster by
	 * hand as it may be in the MFT zone so the allocator would not give it
	 * to us.
	 */
	ll = lcn >> 3;
	page = ntfs_map_page(vol->lcnbmp_ino->i_mapping,
			ll >> PAGE_CACHE_SHIFT);
	if (IS_ERR(page)) {
		up_write(&mftbmp_ni->runlist.lock);
		ntfs_error(vol->sb, "Failed to read from lcn bitmap.");
		return PTR_ERR(page);
	}
	b = (u8*)page_address(page) + (ll & ~PAGE_CACHE_MASK);
	tb = 1 << (lcn & 7ull);
	down_write(&vol->lcnbmp_lock);
	if (*b != 0xff && !(*b & tb)) {
		/* Next cluster is free, allocate it. */
		*b |= tb;
		flush_dcache_page(page);
		set_page_dirty(page);
		up_write(&vol->lcnbmp_lock);
		ntfs_unmap_page(page);
		/* Update the mft bitmap runlist. */
		rl->length++;
		rl[1].vcn++;
		status.added_cluster = 1;
		ntfs_debug("Appending one cluster to mft bitmap.");
	} else {
		up_write(&vol->lcnbmp_lock);
		ntfs_unmap_page(page);
		/* Allocate a cluster from the DATA_ZONE. */
		rl2 = ntfs_cluster_alloc(vol, rl[1].vcn, 1, lcn, DATA_ZONE,
				true);
		if (IS_ERR(rl2)) {
			up_write(&mftbmp_ni->runlist.lock);
			ntfs_error(vol->sb, "Failed to allocate a cluster for "
					"the mft bitmap.");
			return PTR_ERR(rl2);
		}
		rl = ntfs_runlists_merge(mftbmp_ni->runlist.rl, rl2);
		if (IS_ERR(rl)) {
			up_write(&mftbmp_ni->runlist.lock);
			ntfs_error(vol->sb, "Failed to merge runlists for mft "
					"bitmap.");
			if (ntfs_cluster_free_from_rl(vol, rl2)) {
				ntfs_error(vol->sb, "Failed to dealocate "
						"allocated cluster.%s", es);
				NVolSetErrors(vol);
			}
			ntfs_free(rl2);
			return PTR_ERR(rl);
		}
		mftbmp_ni->runlist.rl = rl;
		status.added_run = 1;
		ntfs_debug("Adding one run to mft bitmap.");
		/* Find the last run in the new runlist. */
		for (; rl[1].length; rl++)
			;
	}
	/*
	 * Update the attribute record as well.  Note: @rl is the last
	 * (non-terminator) runlist element of mft bitmap.
	 */
	mrec = map_mft_record(mft_ni);
	if (IS_ERR(mrec)) {
		ntfs_error(vol->sb, "Failed to map mft record.");
		ret = PTR_ERR(mrec);
		goto undo_alloc;
	}
	ctx = ntfs_attr_get_search_ctx(mft_ni, mrec);
	if (unlikely(!ctx)) {
		ntfs_error(vol->sb, "Failed to get search context.");
		ret = -ENOMEM;
		goto undo_alloc;
	}
	ret = ntfs_attr_lookup(mftbmp_ni->type, mftbmp_ni->name,
			mftbmp_ni->name_len, CASE_SENSITIVE, rl[1].vcn, NULL,
			0, ctx);
	if (unlikely(ret)) {
		ntfs_error(vol->sb, "Failed to find last attribute extent of "
				"mft bitmap attribute.");
		if (ret == -ENOENT)
			ret = -EIO;
		goto undo_alloc;
	}
	a = ctx->attr;
	ll = sle64_to_cpu(a->data.non_resident.lowest_vcn);
	/* Search back for the previous last allocated cluster of mft bitmap. */
	for (rl2 = rl; rl2 > mftbmp_ni->runlist.rl; rl2--) {
		if (ll >= rl2->vcn)
			break;
	}
	BUG_ON(ll < rl2->vcn);
	BUG_ON(ll >= rl2->vcn + rl2->length);
	/* Get the size for the new mapping pairs array for this extent. */
	mp_size = ntfs_get_size_for_mapping_pairs(vol, rl2, ll, -1);
	if (unlikely(mp_size <= 0)) {
		ntfs_error(vol->sb, "Get size for mapping pairs failed for "
				"mft bitmap attribute extent.");
		ret = mp_size;
		if (!ret)
			ret = -EIO;
		goto undo_alloc;
	}
	/* Expand the attribute record if necessary. */
	old_alen = le32_to_cpu(a->length);
	ret = ntfs_attr_record_resize(ctx->mrec, a, mp_size +
			le16_to_cpu(a->data.non_resident.mapping_pairs_offset));
	if (unlikely(ret)) {
		if (ret != -ENOSPC) {
			ntfs_error(vol->sb, "Failed to resize attribute "
					"record for mft bitmap attribute.");
			goto undo_alloc;
		}
		// TODO: Deal with this by moving this extent to a new mft
		// record or by starting a new extent in a new mft record or by
		// moving other attributes out of this mft record.
		// Note: It will need to be a special mft record and if none of
		// those are available it gets rather complicated...
		ntfs_error(vol->sb, "Not enough space in this mft record to "
				"accomodate extended mft bitmap attribute "
				"extent.  Cannot handle this yet.");
		ret = -EOPNOTSUPP;
		goto undo_alloc;
	}
	status.mp_rebuilt = 1;
	/* Generate the mapping pairs array directly into the attr record. */
	ret = ntfs_mapping_pairs_build(vol, (u8*)a +
			le16_to_cpu(a->data.non_resident.mapping_pairs_offset),
			mp_size, rl2, ll, -1, NULL);
	if (unlikely(ret)) {
		ntfs_error(vol->sb, "Failed to build mapping pairs array for "
				"mft bitmap attribute.");
		goto undo_alloc;
	}
	/* Update the highest_vcn. */
	a->data.non_resident.highest_vcn = cpu_to_sle64(rl[1].vcn - 1);
	/*
	 * We now have extended the mft bitmap allocated_size by one cluster.
	 * Reflect this in the ntfs_inode structure and the attribute record.
	 */
	if (a->data.non_resident.lowest_vcn) {
		/*
		 * We are not in the first attribute extent, switch to it, but
		 * first ensure the changes will make it to disk later.
		 */
		flush_dcache_mft_record_page(ctx->ntfs_ino);
		mark_mft_record_dirty(ctx->ntfs_ino);
		ntfs_attr_reinit_search_ctx(ctx);
		ret = ntfs_attr_lookup(mftbmp_ni->type, mftbmp_ni->name,
				mftbmp_ni->name_len, CASE_SENSITIVE, 0, NULL,
				0, ctx);
		if (unlikely(ret)) {
			ntfs_error(vol->sb, "Failed to find first attribute "
					"extent of mft bitmap attribute.");
			goto restore_undo_alloc;
		}
		a = ctx->attr;
	}
	write_lock_irqsave(&mftbmp_ni->size_lock, flags);
	mftbmp_ni->allocated_size += vol->cluster_size;
	a->data.non_resident.allocated_size =
			cpu_to_sle64(mftbmp_ni->allocated_size);
	write_unlock_irqrestore(&mftbmp_ni->size_lock, flags);
	/* Ensure the changes make it to disk. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(mft_ni);
	up_write(&mftbmp_ni->runlist.lock);
	ntfs_debug("Done.");
	return 0;
restore_undo_alloc:
	ntfs_attr_reinit_search_ctx(ctx);
	if (ntfs_attr_lookup(mftbmp_ni->type, mftbmp_ni->name,
			mftbmp_ni->name_len, CASE_SENSITIVE, rl[1].vcn, NULL,
			0, ctx)) {
		ntfs_error(vol->sb, "Failed to find last attribute extent of "
				"mft bitmap attribute.%s", es);
		write_lock_irqsave(&mftbmp_ni->size_lock, flags);
		mftbmp_ni->allocated_size += vol->cluster_size;
		write_unlock_irqrestore(&mftbmp_ni->size_lock, flags);
		ntfs_attr_put_search_ctx(ctx);
		unmap_mft_record(mft_ni);
		up_write(&mftbmp_ni->runlist.lock);
		/*
		 * The only thing that is now wrong is ->allocated_size of the
		 * base attribute extent which chkdsk should be able to fix.
		 */
		NVolSetErrors(vol);
		return ret;
	}
	a = ctx->attr;
	a->data.non_resident.highest_vcn = cpu_to_sle64(rl[1].vcn - 2);
undo_alloc:
	if (status.added_cluster) {
		/* Truncate the last run in the runlist by one cluster. */
		rl->length--;
		rl[1].vcn--;
	} else if (status.added_run) {
		lcn = rl->lcn;
		/* Remove the last run from the runlist. */
		rl->lcn = rl[1].lcn;
		rl->length = 0;
	}
	/* Deallocate the cluster. */
	down_write(&vol->lcnbmp_lock);
	if (ntfs_bitmap_clear_bit(vol->lcnbmp_ino, lcn)) {
		ntfs_error(vol->sb, "Failed to free allocated cluster.%s", es);
		NVolSetErrors(vol);
	}
	up_write(&vol->lcnbmp_lock);
	if (status.mp_rebuilt) {
		if (ntfs_mapping_pairs_build(vol, (u8*)a + le16_to_cpu(
				a->data.non_resident.mapping_pairs_offset),
				old_alen - le16_to_cpu(
				a->data.non_resident.mapping_pairs_offset),
				rl2, ll, -1, NULL)) {
			ntfs_error(vol->sb, "Failed to restore mapping pairs "
					"array.%s", es);
			NVolSetErrors(vol);
		}
		if (ntfs_attr_record_resize(ctx->mrec, a, old_alen)) {
			ntfs_error(vol->sb, "Failed to restore attribute "
					"record.%s", es);
			NVolSetErrors(vol);
		}
		flush_dcache_mft_record_page(ctx->ntfs_ino);
		mark_mft_record_dirty(ctx->ntfs_ino);
	}
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (!IS_ERR(mrec))
		unmap_mft_record(mft_ni);
	up_write(&mftbmp_ni->runlist.lock);
	return ret;
}

/**
 * ntfs_mft_bitmap_extend_initialized_nolock - extend mftbmp initialized data
 * @vol:	volume on which to extend the mft bitmap attribute
 *
 * Extend the initialized portion of the mft bitmap attribute on the ntfs
 * volume @vol by 8 bytes.
 *
 * Note:  Only changes initialized_size and data_size, i.e. requires that
 * allocated_size is big enough to fit the new initialized_size.
 *
 * Return 0 on success and -error on error.
 *
 * Locking: Caller must hold vol->mftbmp_lock for writing.
 */
static int ntfs_mft_bitmap_extend_initialized_nolock(ntfs_volume *vol)
{
	s64 old_data_size, old_initialized_size;
	unsigned long flags;
	struct inode *mftbmp_vi;
	ntfs_inode *mft_ni, *mftbmp_ni;
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *mrec;
	ATTR_RECORD *a;
	int ret;

	ntfs_debug("Extending mft bitmap initiailized (and data) size.");
	mft_ni = NTFS_I(vol->mft_ino);
	mftbmp_vi = vol->mftbmp_ino;
	mftbmp_ni = NTFS_I(mftbmp_vi);
	/* Get the attribute record. */
	mrec = map_mft_record(mft_ni);
	if (IS_ERR(mrec)) {
		ntfs_error(vol->sb, "Failed to map mft record.");
		return PTR_ERR(mrec);
	}
	ctx = ntfs_attr_get_search_ctx(mft_ni, mrec);
	if (unlikely(!ctx)) {
		ntfs_error(vol->sb, "Failed to get search context.");
		ret = -ENOMEM;
		goto unm_err_out;
	}
	ret = ntfs_attr_lookup(mftbmp_ni->type, mftbmp_ni->name,
			mftbmp_ni->name_len, CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (unlikely(ret)) {
		ntfs_error(vol->sb, "Failed to find first attribute extent of "
				"mft bitmap attribute.");
		if (ret == -ENOENT)
			ret = -EIO;
		goto put_err_out;
	}
	a = ctx->attr;
	write_lock_irqsave(&mftbmp_ni->size_lock, flags);
	old_data_size = i_size_read(mftbmp_vi);
	old_initialized_size = mftbmp_ni->initialized_size;
	/*
	 * We can simply update the initialized_size before filling the space
	 * with zeroes because the caller is holding the mft bitmap lock for
	 * writing which ensures that no one else is trying to access the data.
	 */
	mftbmp_ni->initialized_size += 8;
	a->data.non_resident.initialized_size =
			cpu_to_sle64(mftbmp_ni->initialized_size);
	if (mftbmp_ni->initialized_size > old_data_size) {
		i_size_write(mftbmp_vi, mftbmp_ni->initialized_size);
		a->data.non_resident.data_size =
				cpu_to_sle64(mftbmp_ni->initialized_size);
	}
	write_unlock_irqrestore(&mftbmp_ni->size_lock, flags);
	/* Ensure the changes make it to disk. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(mft_ni);
	/* Initialize the mft bitmap attribute value with zeroes. */
	ret = ntfs_attr_set(mftbmp_ni, old_initialized_size, 8, 0);
	if (likely(!ret)) {
		ntfs_debug("Done.  (Wrote eight initialized bytes to mft "
				"bitmap.");
		return 0;
	}
	ntfs_error(vol->sb, "Failed to write to mft bitmap.");
	/* Try to recover from the error. */
	mrec = map_mft_record(mft_ni);
	if (IS_ERR(mrec)) {
		ntfs_error(vol->sb, "Failed to map mft record.%s", es);
		NVolSetErrors(vol);
		return ret;
	}
	ctx = ntfs_attr_get_search_ctx(mft_ni, mrec);
	if (unlikely(!ctx)) {
		ntfs_error(vol->sb, "Failed to get search context.%s", es);
		NVolSetErrors(vol);
		goto unm_err_out;
	}
	if (ntfs_attr_lookup(mftbmp_ni->type, mftbmp_ni->name,
			mftbmp_ni->name_len, CASE_SENSITIVE, 0, NULL, 0, ctx)) {
		ntfs_error(vol->sb, "Failed to find first attribute extent of "
				"mft bitmap attribute.%s", es);
		NVolSetErrors(vol);
put_err_out:
		ntfs_attr_put_search_ctx(ctx);
unm_err_out:
		unmap_mft_record(mft_ni);
		goto err_out;
	}
	a = ctx->attr;
	write_lock_irqsave(&mftbmp_ni->size_lock, flags);
	mftbmp_ni->initialized_size = old_initialized_size;
	a->data.non_resident.initialized_size =
			cpu_to_sle64(old_initialized_size);
	if (i_size_read(mftbmp_vi) != old_data_size) {
		i_size_write(mftbmp_vi, old_data_size);
		a->data.non_resident.data_size = cpu_to_sle64(old_data_size);
	}
	write_unlock_irqrestore(&mftbmp_ni->size_lock, flags);
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(mft_ni);
#ifdef DEBUG
	read_lock_irqsave(&mftbmp_ni->size_lock, flags);
	ntfs_debug("Restored status of mftbmp: allocated_size 0x%llx, "
			"data_size 0x%llx, initialized_size 0x%llx.",
			(long long)mftbmp_ni->allocated_size,
			(long long)i_size_read(mftbmp_vi),
			(long long)mftbmp_ni->initialized_size);
	read_unlock_irqrestore(&mftbmp_ni->size_lock, flags);
#endif /* DEBUG */
err_out:
	return ret;
}

/**
 * ntfs_mft_data_extend_allocation_nolock - extend mft data attribute
 * @vol:	volume on which to extend the mft data attribute
 *
 * Extend the mft data attribute on the ntfs volume @vol by 16 mft records
 * worth of clusters or if not enough space for this by one mft record worth
 * of clusters.
 *
 * Note:  Only changes allocated_size, i.e. does not touch initialized_size or
 * data_size.
 *
 * Return 0 on success and -errno on error.
 *
 * Locking: - Caller must hold vol->mftbmp_lock for writing.
 *	    - This function takes NTFS_I(vol->mft_ino)->runlist.lock for
 *	      writing and releases it before returning.
 *	    - This function calls functions which take vol->lcnbmp_lock for
 *	      writing and release it before returning.
 */
static int ntfs_mft_data_extend_allocation_nolock(ntfs_volume *vol)
{
	LCN lcn;
	VCN old_last_vcn;
	s64 min_nr, nr, ll;
	unsigned long flags;
	ntfs_inode *mft_ni;
	runlist_element *rl, *rl2;
	ntfs_attr_search_ctx *ctx = NULL;
	MFT_RECORD *mrec;
	ATTR_RECORD *a = NULL;
	int ret, mp_size;
	u32 old_alen = 0;
	bool mp_rebuilt = false;

	ntfs_debug("Extending mft data allocation.");
	mft_ni = NTFS_I(vol->mft_ino);
	/*
	 * Determine the preferred allocation location, i.e. the last lcn of
	 * the mft data attribute.  The allocated size of the mft data
	 * attribute cannot be zero so we are ok to do this.
	 */
	down_write(&mft_ni->runlist.lock);
	read_lock_irqsave(&mft_ni->size_lock, flags);
	ll = mft_ni->allocated_size;
	read_unlock_irqrestore(&mft_ni->size_lock, flags);
	rl = ntfs_attr_find_vcn_nolock(mft_ni,
			(ll - 1) >> vol->cluster_size_bits, NULL);
	if (unlikely(IS_ERR(rl) || !rl->length || rl->lcn < 0)) {
		up_write(&mft_ni->runlist.lock);
		ntfs_error(vol->sb, "Failed to determine last allocated "
				"cluster of mft data attribute.");
		if (!IS_ERR(rl))
			ret = -EIO;
		else
			ret = PTR_ERR(rl);
		return ret;
	}
	lcn = rl->lcn + rl->length;
	ntfs_debug("Last lcn of mft data attribute is 0x%llx.", (long long)lcn);
	/* Minimum allocation is one mft record worth of clusters. */
	min_nr = vol->mft_record_size >> vol->cluster_size_bits;
	if (!min_nr)
		min_nr = 1;
	/* Want to allocate 16 mft records worth of clusters. */
	nr = vol->mft_record_size << 4 >> vol->cluster_size_bits;
	if (!nr)
		nr = min_nr;
	/* Ensure we do not go above 2^32-1 mft records. */
	read_lock_irqsave(&mft_ni->size_lock, flags);
	ll = mft_ni->allocated_size;
	read_unlock_irqrestore(&mft_ni->size_lock, flags);
	if (unlikely((ll + (nr << vol->cluster_size_bits)) >>
			vol->mft_record_size_bits >= (1ll << 32))) {
		nr = min_nr;
		if (unlikely((ll + (nr << vol->cluster_size_bits)) >>
				vol->mft_record_size_bits >= (1ll << 32))) {
			ntfs_warning(vol->sb, "Cannot allocate mft record "
					"because the maximum number of inodes "
					"(2^32) has already been reached.");
			up_write(&mft_ni->runlist.lock);
			return -ENOSPC;
		}
	}
	ntfs_debug("Trying mft data allocation with %s cluster count %lli.",
			nr > min_nr ? "default" : "minimal", (long long)nr);
	old_last_vcn = rl[1].vcn;
	do {
		rl2 = ntfs_cluster_alloc(vol, old_last_vcn, nr, lcn, MFT_ZONE,
				true);
		if (likely(!IS_ERR(rl2)))
			break;
		if (PTR_ERR(rl2) != -ENOSPC || nr == min_nr) {
			ntfs_error(vol->sb, "Failed to allocate the minimal "
					"number of clusters (%lli) for the "
					"mft data attribute.", (long long)nr);
			up_write(&mft_ni->runlist.lock);
			return PTR_ERR(rl2);
		}
		/*
		 * There is not enough space to do the allocation, but there
		 * might be enough space to do a minimal allocation so try that
		 * before failing.
		 */
		nr = min_nr;
		ntfs_debug("Retrying mft data allocation with minimal cluster "
				"count %lli.", (long long)nr);
	} while (1);
	rl = ntfs_runlists_merge(mft_ni->runlist.rl, rl2);
	if (IS_ERR(rl)) {
		up_write(&mft_ni->runlist.lock);
		ntfs_error(vol->sb, "Failed to merge runlists for mft data "
				"attribute.");
		if (ntfs_cluster_free_from_rl(vol, rl2)) {
			ntfs_error(vol->sb, "Failed to dealocate clusters "
					"from the mft data attribute.%s", es);
			NVolSetErrors(vol);
		}
		ntfs_free(rl2);
		return PTR_ERR(rl);
	}
	mft_ni->runlist.rl = rl;
	ntfs_debug("Allocated %lli clusters.", (long long)nr);
	/* Find the last run in the new runlist. */
	for (; rl[1].length; rl++)
		;
	/* Update the attribute record as well. */
	mrec = map_mft_record(mft_ni);
	if (IS_ERR(mrec)) {
		ntfs_error(vol->sb, "Failed to map mft record.");
		ret = PTR_ERR(mrec);
		goto undo_alloc;
	}
	ctx = ntfs_attr_get_search_ctx(mft_ni, mrec);
	if (unlikely(!ctx)) {
		ntfs_error(vol->sb, "Failed to get search context.");
		ret = -ENOMEM;
		goto undo_alloc;
	}
	ret = ntfs_attr_lookup(mft_ni->type, mft_ni->name, mft_ni->name_len,
			CASE_SENSITIVE, rl[1].vcn, NULL, 0, ctx);
	if (unlikely(ret)) {
		ntfs_error(vol->sb, "Failed to find last attribute extent of "
				"mft data attribute.");
		if (ret == -ENOENT)
			ret = -EIO;
		goto undo_alloc;
	}
	a = ctx->attr;
	ll = sle64_to_cpu(a->data.non_resident.lowest_vcn);
	/* Search back for the previous last allocated cluster of mft bitmap. */
	for (rl2 = rl; rl2 > mft_ni->runlist.rl; rl2--) {
		if (ll >= rl2->vcn)
			break;
	}
	BUG_ON(ll < rl2->vcn);
	BUG_ON(ll >= rl2->vcn + rl2->length);
	/* Get the size for the new mapping pairs array for this extent. */
	mp_size = ntfs_get_size_for_mapping_pairs(vol, rl2, ll, -1);
	if (unlikely(mp_size <= 0)) {
		ntfs_error(vol->sb, "Get size for mapping pairs failed for "
				"mft data attribute extent.");
		ret = mp_size;
		if (!ret)
			ret = -EIO;
		goto undo_alloc;
	}
	/* Expand the attribute record if necessary. */
	old_alen = le32_to_cpu(a->length);
	ret = ntfs_attr_record_resize(ctx->mrec, a, mp_size +
			le16_to_cpu(a->data.non_resident.mapping_pairs_offset));
	if (unlikely(ret)) {
		if (ret != -ENOSPC) {
			ntfs_error(vol->sb, "Failed to resize attribute "
					"record for mft data attribute.");
			goto undo_alloc;
		}
		// TODO: Deal with this by moving this extent to a new mft
		// record or by starting a new extent in a new mft record or by
		// moving other attributes out of this mft record.
		// Note: Use the special reserved mft records and ensure that
		// this extent is not required to find the mft record in
		// question.  If no free special records left we would need to
		// move an existing record away, insert ours in its place, and
		// then place the moved record into the newly allocated space
		// and we would then need to update all references to this mft
		// record appropriately.  This is rather complicated...
		ntfs_error(vol->sb, "Not enough space in this mft record to "
				"accomodate extended mft data attribute "
				"extent.  Cannot handle this yet.");
		ret = -EOPNOTSUPP;
		goto undo_alloc;
	}
	mp_rebuilt = true;
	/* Generate the mapping pairs array directly into the attr record. */
	ret = ntfs_mapping_pairs_build(vol, (u8*)a +
			le16_to_cpu(a->data.non_resident.mapping_pairs_offset),
			mp_size, rl2, ll, -1, NULL);
	if (unlikely(ret)) {
		ntfs_error(vol->sb, "Failed to build mapping pairs array of "
				"mft data attribute.");
		goto undo_alloc;
	}
	/* Update the highest_vcn. */
	a->data.non_resident.highest_vcn = cpu_to_sle64(rl[1].vcn - 1);
	/*
	 * We now have extended the mft data allocated_size by nr clusters.
	 * Reflect this in the ntfs_inode structure and the attribute record.
	 * @rl is the last (non-terminator) runlist element of mft data
	 * attribute.
	 */
	if (a->data.non_resident.lowest_vcn) {
		/*
		 * We are not in the first attribute extent, switch to it, but
		 * first ensure the changes will make it to disk later.
		 */
		flush_dcache_mft_record_page(ctx->ntfs_ino);
		mark_mft_record_dirty(ctx->ntfs_ino);
		ntfs_attr_reinit_search_ctx(ctx);
		ret = ntfs_attr_lookup(mft_ni->type, mft_ni->name,
				mft_ni->name_len, CASE_SENSITIVE, 0, NULL, 0,
				ctx);
		if (unlikely(ret)) {
			ntfs_error(vol->sb, "Failed to find first attribute "
					"extent of mft data attribute.");
			goto restore_undo_alloc;
		}
		a = ctx->attr;
	}
	write_lock_irqsave(&mft_ni->size_lock, flags);
	mft_ni->allocated_size += nr << vol->cluster_size_bits;
	a->data.non_resident.allocated_size =
			cpu_to_sle64(mft_ni->allocated_size);
	write_unlock_irqrestore(&mft_ni->size_lock, flags);
	/* Ensure the changes make it to disk. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(mft_ni);
	up_write(&mft_ni->runlist.lock);
	ntfs_debug("Done.");
	return 0;
restore_undo_alloc:
	ntfs_attr_reinit_search_ctx(ctx);
	if (ntfs_attr_lookup(mft_ni->type, mft_ni->name, mft_ni->name_len,
			CASE_SENSITIVE, rl[1].vcn, NULL, 0, ctx)) {
		ntfs_error(vol->sb, "Failed to find last attribute extent of "
				"mft data attribute.%s", es);
		write_lock_irqsave(&mft_ni->size_lock, flags);
		mft_ni->allocated_size += nr << vol->cluster_size_bits;
		write_unlock_irqrestore(&mft_ni->size_lock, flags);
		ntfs_attr_put_search_ctx(ctx);
		unmap_mft_record(mft_ni);
		up_write(&mft_ni->runlist.lock);
		/*
		 * The only thing that is now wrong is ->allocated_size of the
		 * base attribute extent which chkdsk should be able to fix.
		 */
		NVolSetErrors(vol);
		return ret;
	}
	ctx->attr->data.non_resident.highest_vcn =
			cpu_to_sle64(old_last_vcn - 1);
undo_alloc:
	if (ntfs_cluster_free(mft_ni, old_last_vcn, -1, ctx) < 0) {
		ntfs_error(vol->sb, "Failed to free clusters from mft data "
				"attribute.%s", es);
		NVolSetErrors(vol);
	}
	a = ctx->attr;
	if (ntfs_rl_truncate_nolock(vol, &mft_ni->runlist, old_last_vcn)) {
		ntfs_error(vol->sb, "Failed to truncate mft data attribute "
				"runlist.%s", es);
		NVolSetErrors(vol);
	}
	if (mp_rebuilt && !IS_ERR(ctx->mrec)) {
		if (ntfs_mapping_pairs_build(vol, (u8*)a + le16_to_cpu(
				a->data.non_resident.mapping_pairs_offset),
				old_alen - le16_to_cpu(
				a->data.non_resident.mapping_pairs_offset),
				rl2, ll, -1, NULL)) {
			ntfs_error(vol->sb, "Failed to restore mapping pairs "
					"array.%s", es);
			NVolSetErrors(vol);
		}
		if (ntfs_attr_record_resize(ctx->mrec, a, old_alen)) {
			ntfs_error(vol->sb, "Failed to restore attribute "
					"record.%s", es);
			NVolSetErrors(vol);
		}
		flush_dcache_mft_record_page(ctx->ntfs_ino);
		mark_mft_record_dirty(ctx->ntfs_ino);
	} else if (IS_ERR(ctx->mrec)) {
		ntfs_error(vol->sb, "Failed to restore attribute search "
				"context.%s", es);
		NVolSetErrors(vol);
	}
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (!IS_ERR(mrec))
		unmap_mft_record(mft_ni);
	up_write(&mft_ni->runlist.lock);
	return ret;
}

/**
 * ntfs_mft_record_layout - layout an mft record into a memory buffer
 * @vol:	volume to which the mft record will belong
 * @mft_no:	mft reference specifying the mft record number
 * @m:		destination buffer of size >= @vol->mft_record_size bytes
 *
 * Layout an empty, unused mft record with the mft record number @mft_no into
 * the buffer @m.  The volume @vol is needed because the mft record structure
 * was modified in NTFS 3.1 so we need to know which volume version this mft
 * record will be used on.
 *
 * Return 0 on success and -errno on error.
 */
static int ntfs_mft_record_layout(const ntfs_volume *vol, const s64 mft_no,
		MFT_RECORD *m)
{
	ATTR_RECORD *a;

	ntfs_debug("Entering for mft record 0x%llx.", (long long)mft_no);
	if (mft_no >= (1ll << 32)) {
		ntfs_error(vol->sb, "Mft record number 0x%llx exceeds "
				"maximum of 2^32.", (long long)mft_no);
		return -ERANGE;
	}
	/* Start by clearing the whole mft record to gives us a clean slate. */
	memset(m, 0, vol->mft_record_size);
	/* Aligned to 2-byte boundary. */
	if (vol->major_ver < 3 || (vol->major_ver == 3 && !vol->minor_ver))
		m->usa_ofs = cpu_to_le16((sizeof(MFT_RECORD_OLD) + 1) & ~1);
	else {
		m->usa_ofs = cpu_to_le16((sizeof(MFT_RECORD) + 1) & ~1);
		/*
		 * Set the NTFS 3.1+ specific fields while we know that the
		 * volume version is 3.1+.
		 */
		m->reserved = 0;
		m->mft_record_number = cpu_to_le32((u32)mft_no);
	}
	m->magic = magic_FILE;
	if (vol->mft_record_size >= NTFS_BLOCK_SIZE)
		m->usa_count = cpu_to_le16(vol->mft_record_size /
				NTFS_BLOCK_SIZE + 1);
	else {
		m->usa_count = cpu_to_le16(1);
		ntfs_warning(vol->sb, "Sector size is bigger than mft record "
				"size.  Setting usa_count to 1.  If chkdsk "
				"reports this as corruption, please email "
				"linux-ntfs-dev@lists.sourceforge.net stating "
				"that you saw this message and that the "
				"modified filesystem created was corrupt.  "
				"Thank you.");
	}
	/* Set the update sequence number to 1. */
	*(le16*)((u8*)m + le16_to_cpu(m->usa_ofs)) = cpu_to_le16(1);
	m->lsn = 0;
	m->sequence_number = cpu_to_le16(1);
	m->link_count = 0;
	/*
	 * Place the attributes straight after the update sequence array,
	 * aligned to 8-byte boundary.
	 */
	m->attrs_offset = cpu_to_le16((le16_to_cpu(m->usa_ofs) +
			(le16_to_cpu(m->usa_count) << 1) + 7) & ~7);
	m->flags = 0;
	/*
	 * Using attrs_offset plus eight bytes (for the termination attribute).
	 * attrs_offset is already aligned to 8-byte boundary, so no need to
	 * align again.
	 */
	m->bytes_in_use = cpu_to_le32(le16_to_cpu(m->attrs_offset) + 8);
	m->bytes_allocated = cpu_to_le32(vol->mft_record_size);
	m->base_mft_record = 0;
	m->next_attr_instance = 0;
	/* Add the termination attribute. */
	a = (ATTR_RECORD*)((u8*)m + le16_to_cpu(m->attrs_offset));
	a->type = AT_END;
	a->length = 0;
	ntfs_debug("Done.");
	return 0;
}

/**
 * ntfs_mft_record_format - format an mft record on an ntfs volume
 * @vol:	volume on which to format the mft record
 * @mft_no:	mft record number to format
 *
 * Format the mft record @mft_no in $MFT/$DATA, i.e. lay out an empty, unused
 * mft record into the appropriate place of the mft data attribute.  This is
 * used when extending the mft data attribute.
 *
 * Return 0 on success and -errno on error.
 */
static int ntfs_mft_record_format(const ntfs_volume *vol, const s64 mft_no)
{
	loff_t i_size;
	struct inode *mft_vi = vol->mft_ino;
	struct page *page;
	MFT_RECORD *m;
	pgoff_t index, end_index;
	unsigned int ofs;
	int err;

	ntfs_debug("Entering for mft record 0x%llx.", (long long)mft_no);
	/*
	 * The index into the page cache and the offset within the page cache
	 * page of the wanted mft record.
	 */
	index = mft_no << vol->mft_record_size_bits >> PAGE_CACHE_SHIFT;
	ofs = (mft_no << vol->mft_record_size_bits) & ~PAGE_CACHE_MASK;
	/* The maximum valid index into the page cache for $MFT's data. */
	i_size = i_size_read(mft_vi);
	end_index = i_size >> PAGE_CACHE_SHIFT;
	if (unlikely(index >= end_index)) {
		if (unlikely(index > end_index || ofs + vol->mft_record_size >=
				(i_size & ~PAGE_CACHE_MASK))) {
			ntfs_error(vol->sb, "Tried to format non-existing mft "
					"record 0x%llx.", (long long)mft_no);
			return -ENOENT;
		}
	}
	/* Read, map, and pin the page containing the mft record. */
	page = ntfs_map_page(mft_vi->i_mapping, index);
	if (IS_ERR(page)) {
		ntfs_error(vol->sb, "Failed to map page containing mft record "
				"to format 0x%llx.", (long long)mft_no);
		return PTR_ERR(page);
	}
	lock_page(page);
	BUG_ON(!PageUptodate(page));
	ClearPageUptodate(page);
	m = (MFT_RECORD*)((u8*)page_address(page) + ofs);
	err = ntfs_mft_record_layout(vol, mft_no, m);
	if (unlikely(err)) {
		ntfs_error(vol->sb, "Failed to layout mft record 0x%llx.",
				(long long)mft_no);
		SetPageUptodate(page);
		unlock_page(page);
		ntfs_unmap_page(page);
		return err;
	}
	flush_dcache_page(page);
	SetPageUptodate(page);
	unlock_page(page);
	/*
	 * Make sure the mft record is written out to disk.  We could use
	 * ilookup5() to check if an inode is in icache and so on but this is
	 * unnecessary as ntfs_writepage() will write the dirty record anyway.
	 */
	mark_ntfs_record_dirty(page, ofs);
	ntfs_unmap_page(page);
	ntfs_debug("Done.");
	return 0;
}

/**
 * ntfs_mft_record_alloc - allocate an mft record on an ntfs volume
 * @vol:	[IN]  volume on which to allocate the mft record
 * @mode:	[IN]  mode if want a file or directory, i.e. base inode or 0
 * @base_ni:	[IN]  open base inode if allocating an extent mft record or NULL
 * @mrec:	[OUT] on successful return this is the mapped mft record
 *
 * Allocate an mft record in $MFT/$DATA of an open ntfs volume @vol.
 *
 * If @base_ni is NULL make the mft record a base mft record, i.e. a file or
 * direvctory inode, and allocate it at the default allocator position.  In
 * this case @mode is the file mode as given to us by the caller.  We in
 * particular use @mode to distinguish whether a file or a directory is being
 * created (S_IFDIR(mode) and S_IFREG(mode), respectively).
 *
 * If @base_ni is not NULL make the allocated mft record an extent record,
 * allocate it starting at the mft record after the base mft record and attach
 * the allocated and opened ntfs inode to the base inode @base_ni.  In this
 * case @mode must be 0 as it is meaningless for extent inodes.
 *
 * You need to check the return value with IS_ERR().  If false, the function
 * was successful and the return value is the now opened ntfs inode of the
 * allocated mft record.  *@mrec is then set to the allocated, mapped, pinned,
 * and locked mft record.  If IS_ERR() is true, the function failed and the
 * error code is obtained from PTR_ERR(return value).  *@mrec is undefined in
 * this case.
 *
 * Allocation strategy:
 *
 * To find a free mft record, we scan the mft bitmap for a zero bit.  To
 * optimize this we start scanning at the place specified by @base_ni or if
 * @base_ni is NULL we start where we last stopped and we perform wrap around
 * when we reach the end.  Note, we do not try to allocate mft records below
 * number 24 because numbers 0 to 15 are the defined system files anyway and 16
 * to 24 are special in that they are used for storing extension mft records
 * for the $DATA attribute of $MFT.  This is required to avoid the possibility
 * of creating a runlist with a circular dependency which once written to disk
 * can never be read in again.  Windows will only use records 16 to 24 for
 * normal files if the volume is completely out of space.  We never use them
 * which means that when the volume is really out of space we cannot create any
 * more files while Windows can still create up to 8 small files.  We can start
 * doing this at some later time, it does not matter much for now.
 *
 * When scanning the mft bitmap, we only search up to the last allocated mft
 * record.  If there are no free records left in the range 24 to number of
 * allocated mft records, then we extend the $MFT/$DATA attribute in order to
 * create free mft records.  We extend the allocated size of $MFT/$DATA by 16
 * records at a time or one cluster, if cluster size is above 16kiB.  If there
 * is not sufficient space to do this, we try to extend by a single mft record
 * or one cluster, if cluster size is above the mft record size.
 *
 * No matter how many mft records we allocate, we initialize only the first
 * allocated mft record, incrementing mft data size and initialized size
 * accordingly, open an ntfs_inode for it and return it to the caller, unless
 * there are less than 24 mft records, in which case we allocate and initialize
 * mft records until we reach record 24 which we consider as the first free mft
 * record for use by normal files.
 *
 * If during any stage we overflow the initialized data in the mft bitmap, we
 * extend the initialized size (and data size) by 8 bytes, allocating another
 * cluster if required.  The bitmap data size has to be at least equal to the
 * number of mft records in the mft, but it can be bigger, in which case the
 * superflous bits are padded with zeroes.
 *
 * Thus, when we return successfully (IS_ERR() is false), we will have:
 *	- initialized / extended the mft bitmap if necessary,
 *	- initialized / extended the mft data if necessary,
 *	- set the bit corresponding to the mft record being allocated in the
 *	  mft bitmap,
 *	- opened an ntfs_inode for the allocated mft record, and we will have
 *	- returned the ntfs_inode as well as the allocated mapped, pinned, and
 *	  locked mft record.
 *
 * On error, the volume will be left in a consistent state and no record will
 * be allocated.  If rolling back a partial operation fails, we may leave some
 * inconsistent metadata in which case we set NVolErrors() so the volume is
 * left dirty when unmounted.
 *
 * Note, this function cannot make use of most of the normal functions, like
 * for example for attribute resizing, etc, because when the run list overflows
 * the base mft record and an attribute list is used, it is very important that
 * the extension mft records used to store the $DATA attribute of $MFT can be
 * reached without having to read the information contained inside them, as
 * this would make it impossible to find them in the first place after the
 * volume is unmounted.  $MFT/$BITMAP probably does not need to follow this
 * rule because the bitmap is not essential for finding the mft records, but on
 * the other hand, handling the bitmap in this special way would make life
 * easier because otherwise there might be circular invocations of functions
 * when reading the bitmap.
 */
ntfs_inode *ntfs_mft_record_alloc(ntfs_volume *vol, const int mode,
		ntfs_inode *base_ni, MFT_RECORD **mrec)
{
	s64 ll, bit, old_data_initialized, old_data_size;
	unsigned long flags;
	struct inode *vi;
	struct page *page;
	ntfs_inode *mft_ni, *mftbmp_ni, *ni;
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *m;
	ATTR_RECORD *a;
	pgoff_t index;
	unsigned int ofs;
	int err;
	le16 seq_no, usn;
	bool record_formatted = false;

	if (base_ni) {
		ntfs_debug("Entering (allocating an extent mft record for "
				"base mft record 0x%llx).",
				(long long)base_ni->mft_no);
		/* @mode and @base_ni are mutually exclusive. */
		BUG_ON(mode);
	} else
		ntfs_debug("Entering (allocating a base mft record).");
	if (mode) {
		/* @mode and @base_ni are mutually exclusive. */
		BUG_ON(base_ni);
		/* We only support creation of normal files and directories. */
		if (!S_ISREG(mode) && !S_ISDIR(mode))
			return ERR_PTR(-EOPNOTSUPP);
	}
	BUG_ON(!mrec);
	mft_ni = NTFS_I(vol->mft_ino);
	mftbmp_ni = NTFS_I(vol->mftbmp_ino);
	down_write(&vol->mftbmp_lock);
	bit = ntfs_mft_bitmap_find_and_alloc_free_rec_nolock(vol, base_ni);
	if (bit >= 0) {
		ntfs_debug("Found and allocated free record (#1), bit 0x%llx.",
				(long long)bit);
		goto have_alloc_rec;
	}
	if (bit != -ENOSPC) {
		up_write(&vol->mftbmp_lock);
		return ERR_PTR(bit);
	}
	/*
	 * No free mft records left.  If the mft bitmap already covers more
	 * than the currently used mft records, the next records are all free,
	 * so we can simply allocate the first unused mft record.
	 * Note: We also have to make sure that the mft bitmap at least covers
	 * the first 24 mft records as they are special and whilst they may not
	 * be in use, we do not allocate from them.
	 */
	read_lock_irqsave(&mft_ni->size_lock, flags);
	ll = mft_ni->initialized_size >> vol->mft_record_size_bits;
	read_unlock_irqrestore(&mft_ni->size_lock, flags);
	read_lock_irqsave(&mftbmp_ni->size_lock, flags);
	old_data_initialized = mftbmp_ni->initialized_size;
	read_unlock_irqrestore(&mftbmp_ni->size_lock, flags);
	if (old_data_initialized << 3 > ll && old_data_initialized > 3) {
		bit = ll;
		if (bit < 24)
			bit = 24;
		if (unlikely(bit >= (1ll << 32)))
			goto max_err_out;
		ntfs_debug("Found free record (#2), bit 0x%llx.",
				(long long)bit);
		goto found_free_rec;
	}
	/*
	 * The mft bitmap needs to be expanded until it covers the first unused
	 * mft record that we can allocate.
	 * Note: The smallest mft record we allocate is mft record 24.
	 */
	bit = old_data_initialized << 3;
	if (unlikely(bit >= (1ll << 32)))
		goto max_err_out;
	read_lock_irqsave(&mftbmp_ni->size_lock, flags);
	old_data_size = mftbmp_ni->allocated_size;
	ntfs_debug("Status of mftbmp before extension: allocated_size 0x%llx, "
			"data_size 0x%llx, initialized_size 0x%llx.",
			(long long)old_data_size,
			(long long)i_size_read(vol->mftbmp_ino),
			(long long)old_data_initialized);
	read_unlock_irqrestore(&mftbmp_ni->size_lock, flags);
	if (old_data_initialized + 8 > old_data_size) {
		/* Need to extend bitmap by one more cluster. */
		ntfs_debug("mftbmp: initialized_size + 8 > allocated_size.");
		err = ntfs_mft_bitmap_extend_allocation_nolock(vol);
		if (unlikely(err)) {
			up_write(&vol->mftbmp_lock);
			goto err_out;
		}
#ifdef DEBUG
		read_lock_irqsave(&mftbmp_ni->size_lock, flags);
		ntfs_debug("Status of mftbmp after allocation extension: "
				"allocated_size 0x%llx, data_size 0x%llx, "
				"initialized_size 0x%llx.",
				(long long)mftbmp_ni->allocated_size,
				(long long)i_size_read(vol->mftbmp_ino),
				(long long)mftbmp_ni->initialized_size);
		read_unlock_irqrestore(&mftbmp_ni->size_lock, flags);
#endif /* DEBUG */
	}
	/*
	 * We now have sufficient allocated space, extend the initialized_size
	 * as well as the data_size if necessary and fill the new space with
	 * zeroes.
	 */
	err = ntfs_mft_bitmap_extend_initialized_nolock(vol);
	if (unlikely(err)) {
		up_write(&vol->mftbmp_lock);
		goto err_out;
	}
#ifdef DEBUG
	read_lock_irqsave(&mftbmp_ni->size_lock, flags);
	ntfs_debug("Status of mftbmp after initialized extention: "
			"allocated_size 0x%llx, data_size 0x%llx, "
			"initialized_size 0x%llx.",
			(long long)mftbmp_ni->allocated_size,
			(long long)i_size_read(vol->mftbmp_ino),
			(long long)mftbmp_ni->initialized_size);
	read_unlock_irqrestore(&mftbmp_ni->size_lock, flags);
#endif /* DEBUG */
	ntfs_debug("Found free record (#3), bit 0x%llx.", (long long)bit);
found_free_rec:
	/* @bit is the found free mft record, allocate it in the mft bitmap. */
	ntfs_debug("At found_free_rec.");
	err = ntfs_bitmap_set_bit(vol->mftbmp_ino, bit);
	if (unlikely(err)) {
		ntfs_error(vol->sb, "Failed to allocate bit in mft bitmap.");
		up_write(&vol->mftbmp_lock);
		goto err_out;
	}
	ntfs_debug("Set bit 0x%llx in mft bitmap.", (long long)bit);
have_alloc_rec:
	/*
	 * The mft bitmap is now uptodate.  Deal with mft data attribute now.
	 * Note, we keep hold of the mft bitmap lock for writing until all
	 * modifications to the mft data attribute are complete, too, as they
	 * will impact decisions for mft bitmap and mft record allocation done
	 * by a parallel allocation and if the lock is not maintained a
	 * parallel allocation could allocate the same mft record as this one.
	 */
	ll = (bit + 1) << vol->mft_record_size_bits;
	read_lock_irqsave(&mft_ni->size_lock, flags);
	old_data_initialized = mft_ni->initialized_size;
	read_unlock_irqrestore(&mft_ni->size_lock, flags);
	if (ll <= old_data_initialized) {
		ntfs_debug("Allocated mft record already initialized.");
		goto mft_rec_already_initialized;
	}
	ntfs_debug("Initializing allocated mft record.");
	/*
	 * The mft record is outside the initialized data.  Extend the mft data
	 * attribute until it covers the allocated record.  The loop is only
	 * actually traversed more than once when a freshly formatted volume is
	 * first written to so it optimizes away nicely in the common case.
	 */
	read_lock_irqsave(&mft_ni->size_lock, flags);
	ntfs_debug("Status of mft data before extension: "
			"allocated_size 0x%llx, data_size 0x%llx, "
			"initialized_size 0x%llx.",
			(long long)mft_ni->allocated_size,
			(long long)i_size_read(vol->mft_ino),
			(long long)mft_ni->initialized_size);
	while (ll > mft_ni->allocated_size) {
		read_unlock_irqrestore(&mft_ni->size_lock, flags);
		err = ntfs_mft_data_extend_allocation_nolock(vol);
		if (unlikely(err)) {
			ntfs_error(vol->sb, "Failed to extend mft data "
					"allocation.");
			goto undo_mftbmp_alloc_nolock;
		}
		read_lock_irqsave(&mft_ni->size_lock, flags);
		ntfs_debug("Status of mft data after allocation extension: "
				"allocated_size 0x%llx, data_size 0x%llx, "
				"initialized_size 0x%llx.",
				(long long)mft_ni->allocated_size,
				(long long)i_size_read(vol->mft_ino),
				(long long)mft_ni->initialized_size);
	}
	read_unlock_irqrestore(&mft_ni->size_lock, flags);
	/*
	 * Extend mft data initialized size (and data size of course) to reach
	 * the allocated mft record, formatting the mft records allong the way.
	 * Note: We only modify the ntfs_inode structure as that is all that is
	 * needed by ntfs_mft_record_format().  We will update the attribute
	 * record itself in one fell swoop later on.
	 */
	write_lock_irqsave(&mft_ni->size_lock, flags);
	old_data_initialized = mft_ni->initialized_size;
	old_data_size = vol->mft_ino->i_size;
	while (ll > mft_ni->initialized_size) {
		s64 new_initialized_size, mft_no;
		
		new_initialized_size = mft_ni->initialized_size +
				vol->mft_record_size;
		mft_no = mft_ni->initialized_size >> vol->mft_record_size_bits;
		if (new_initialized_size > i_size_read(vol->mft_ino))
			i_size_write(vol->mft_ino, new_initialized_size);
		write_unlock_irqrestore(&mft_ni->size_lock, flags);
		ntfs_debug("Initializing mft record 0x%llx.",
				(long long)mft_no);
		err = ntfs_mft_record_format(vol, mft_no);
		if (unlikely(err)) {
			ntfs_error(vol->sb, "Failed to format mft record.");
			goto undo_data_init;
		}
		write_lock_irqsave(&mft_ni->size_lock, flags);
		mft_ni->initialized_size = new_initialized_size;
	}
	write_unlock_irqrestore(&mft_ni->size_lock, flags);
	record_formatted = true;
	/* Update the mft data attribute record to reflect the new sizes. */
	m = map_mft_record(mft_ni);
	if (IS_ERR(m)) {
		ntfs_error(vol->sb, "Failed to map mft record.");
		err = PTR_ERR(m);
		goto undo_data_init;
	}
	ctx = ntfs_attr_get_search_ctx(mft_ni, m);
	if (unlikely(!ctx)) {
		ntfs_error(vol->sb, "Failed to get search context.");
		err = -ENOMEM;
		unmap_mft_record(mft_ni);
		goto undo_data_init;
	}
	err = ntfs_attr_lookup(mft_ni->type, mft_ni->name, mft_ni->name_len,
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (unlikely(err)) {
		ntfs_error(vol->sb, "Failed to find first attribute extent of "
				"mft data attribute.");
		ntfs_attr_put_search_ctx(ctx);
		unmap_mft_record(mft_ni);
		goto undo_data_init;
	}
	a = ctx->attr;
	read_lock_irqsave(&mft_ni->size_lock, flags);
	a->data.non_resident.initialized_size =
			cpu_to_sle64(mft_ni->initialized_size);
	a->data.non_resident.data_size =
			cpu_to_sle64(i_size_read(vol->mft_ino));
	read_unlock_irqrestore(&mft_ni->size_lock, flags);
	/* Ensure the changes make it to disk. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(mft_ni);
	read_lock_irqsave(&mft_ni->size_lock, flags);
	ntfs_debug("Status of mft data after mft record initialization: "
			"allocated_size 0x%llx, data_size 0x%llx, "
			"initialized_size 0x%llx.",
			(long long)mft_ni->allocated_size,
			(long long)i_size_read(vol->mft_ino),
			(long long)mft_ni->initialized_size);
	BUG_ON(i_size_read(vol->mft_ino) > mft_ni->allocated_size);
	BUG_ON(mft_ni->initialized_size > i_size_read(vol->mft_ino));
	read_unlock_irqrestore(&mft_ni->size_lock, flags);
mft_rec_already_initialized:
	/*
	 * We can finally drop the mft bitmap lock as the mft data attribute
	 * has been fully updated.  The only disparity left is that the
	 * allocated mft record still needs to be marked as in use to match the
	 * set bit in the mft bitmap but this is actually not a problem since
	 * this mft record is not referenced from anywhere yet and the fact
	 * that it is allocated in the mft bitmap means that no-one will try to
	 * allocate it either.
	 */
	up_write(&vol->mftbmp_lock);
	/*
	 * We now have allocated and initialized the mft record.  Calculate the
	 * index of and the offset within the page cache page the record is in.
	 */
	index = bit << vol->mft_record_size_bits >> PAGE_CACHE_SHIFT;
	ofs = (bit << vol->mft_record_size_bits) & ~PAGE_CACHE_MASK;
	/* Read, map, and pin the page containing the mft record. */
	page = ntfs_map_page(vol->mft_ino->i_mapping, index);
	if (IS_ERR(page)) {
		ntfs_error(vol->sb, "Failed to map page containing allocated "
				"mft record 0x%llx.", (long long)bit);
		err = PTR_ERR(page);
		goto undo_mftbmp_alloc;
	}
	lock_page(page);
	BUG_ON(!PageUptodate(page));
	ClearPageUptodate(page);
	m = (MFT_RECORD*)((u8*)page_address(page) + ofs);
	/* If we just formatted the mft record no need to do it again. */
	if (!record_formatted) {
		/* Sanity check that the mft record is really not in use. */
		if (ntfs_is_file_record(m->magic) &&
				(m->flags & MFT_RECORD_IN_USE)) {
			ntfs_error(vol->sb, "Mft record 0x%llx was marked "
					"free in mft bitmap but is marked "
					"used itself.  Corrupt filesystem.  "
					"Unmount and run chkdsk.",
					(long long)bit);
			err = -EIO;
			SetPageUptodate(page);
			unlock_page(page);
			ntfs_unmap_page(page);
			NVolSetErrors(vol);
			goto undo_mftbmp_alloc;
		}
		/*
		 * We need to (re-)format the mft record, preserving the
		 * sequence number if it is not zero as well as the update
		 * sequence number if it is not zero or -1 (0xffff).  This
		 * means we do not need to care whether or not something went
		 * wrong with the previous mft record.
		 */
		seq_no = m->sequence_number;
		usn = *(le16*)((u8*)m + le16_to_cpu(m->usa_ofs));
		err = ntfs_mft_record_layout(vol, bit, m);
		if (unlikely(err)) {
			ntfs_error(vol->sb, "Failed to layout allocated mft "
					"record 0x%llx.", (long long)bit);
			SetPageUptodate(page);
			unlock_page(page);
			ntfs_unmap_page(page);
			goto undo_mftbmp_alloc;
		}
		if (seq_no)
			m->sequence_number = seq_no;
		if (usn && le16_to_cpu(usn) != 0xffff)
			*(le16*)((u8*)m + le16_to_cpu(m->usa_ofs)) = usn;
	}
	/* Set the mft record itself in use. */
	m->flags |= MFT_RECORD_IN_USE;
	if (S_ISDIR(mode))
		m->flags |= MFT_RECORD_IS_DIRECTORY;
	flush_dcache_page(page);
	SetPageUptodate(page);
	if (base_ni) {
		/*
		 * Setup the base mft record in the extent mft record.  This
		 * completes initialization of the allocated extent mft record
		 * and we can simply use it with map_extent_mft_record().
		 */
		m->base_mft_record = MK_LE_MREF(base_ni->mft_no,
				base_ni->seq_no);
		/*
		 * Allocate an extent inode structure for the new mft record,
		 * attach it to the base inode @base_ni and map, pin, and lock
		 * its, i.e. the allocated, mft record.
		 */
		m = map_extent_mft_record(base_ni, bit, &ni);
		if (IS_ERR(m)) {
			ntfs_error(vol->sb, "Failed to map allocated extent "
					"mft record 0x%llx.", (long long)bit);
			err = PTR_ERR(m);
			/* Set the mft record itself not in use. */
			m->flags &= cpu_to_le16(
					~le16_to_cpu(MFT_RECORD_IN_USE));
			flush_dcache_page(page);
			/* Make sure the mft record is written out to disk. */
			mark_ntfs_record_dirty(page, ofs);
			unlock_page(page);
			ntfs_unmap_page(page);
			goto undo_mftbmp_alloc;
		}
		/*
		 * Make sure the allocated mft record is written out to disk.
		 * No need to set the inode dirty because the caller is going
		 * to do that anyway after finishing with the new extent mft
		 * record (e.g. at a minimum a new attribute will be added to
		 * the mft record.
		 */
		mark_ntfs_record_dirty(page, ofs);
		unlock_page(page);
		/*
		 * Need to unmap the page since map_extent_mft_record() mapped
		 * it as well so we have it mapped twice at the moment.
		 */
		ntfs_unmap_page(page);
	} else {
		/*
		 * Allocate a new VFS inode and set it up.  NOTE: @vi->i_nlink
		 * is set to 1 but the mft record->link_count is 0.  The caller
		 * needs to bear this in mind.
		 */
		vi = new_inode(vol->sb);
		if (unlikely(!vi)) {
			err = -ENOMEM;
			/* Set the mft record itself not in use. */
			m->flags &= cpu_to_le16(
					~le16_to_cpu(MFT_RECORD_IN_USE));
			flush_dcache_page(page);
			/* Make sure the mft record is written out to disk. */
			mark_ntfs_record_dirty(page, ofs);
			unlock_page(page);
			ntfs_unmap_page(page);
			goto undo_mftbmp_alloc;
		}
		vi->i_ino = bit;
		/*
		 * This is for checking whether an inode has changed w.r.t. a
		 * file so that the file can be updated if necessary (compare
		 * with f_version).
		 */
		vi->i_version = 1;

		/* The owner and group come from the ntfs volume. */
		vi->i_uid = vol->uid;
		vi->i_gid = vol->gid;

		/* Initialize the ntfs specific part of @vi. */
		ntfs_init_big_inode(vi);
		ni = NTFS_I(vi);
		/*
		 * Set the appropriate mode, attribute type, and name.  For
		 * directories, also setup the index values to the defaults.
		 */
		if (S_ISDIR(mode)) {
			vi->i_mode = S_IFDIR | S_IRWXUGO;
			vi->i_mode &= ~vol->dmask;

			NInoSetMstProtected(ni);
			ni->type = AT_INDEX_ALLOCATION;
			ni->name = I30;
			ni->name_len = 4;

			ni->itype.index.block_size = 4096;
			ni->itype.index.block_siroid32android_view_MotionEvent_recycleEP7_JNIEnvP8_jobject _ZN7_JNIEnv16CallObjectMethodEP8_jobjectP10_jmethodIDz _ZN7android38android_view_PointerIcon_getSystemIconEP7_JNIEnvP8_jobjecti _ZN7android29android_view_PointerIcon_loadEP7_JNIEnvP8_jobjectS3_PNS_11PointerIconE _ZN8SkBitmap5resetEv _ZN11GraphicsJNI15getNativeBitmapEP7_JNIEnvP8_jobject _ZN8SkBitmapaSERKS_ _ZN7android39android_view_PointerIcon_loadSystemIconEP7_JNIEnvP8_jobjectiPNS_11PointerIconE _ZN7android10uirenderer15AnimatorManager22endAllStagingAnimatorsEv _ZN7android10uirenderer16RenderProperties12updateMatrixEv _ZN8Sk3DView18getCameraLocationZEv _ZN8Sk3DView17setCameraLocationEfff _ZN6SkPath6rewindEv _ZN6SkPath9addCircleEfffNS_9DirectionE _ZN6SkPath5resetEv _ZN6SkPathaSERKS_ _ZN6SkRect14setBoundsCheckEPK7SkPointi _ZN6SkPath12addRoundRectERK6SkRectffNS_9DirectionE _ZN7android10uirenderer15LayerProperties12setFromPaintEPK7SkPaint _ZN7android10uirenderer15LayerProperties5resetEv _ZN7android10uirenderer10RenderNode12getDebugSizeEv _ZN7android10uirenderer10RenderNode6outputEj _ZN7android10uirenderer10RenderNode21setStagingDisplayListEPNS0_15DisplayListDataE _ZN7android10uirenderer10RenderNodeC1Ev strrchr _ZN8SkMatrix5resetEv memcmp _ZN7android10uirenderer10RenderNode11addAnimatorERKNS_2spINS0_22BaseRenderNodeAnimatorEEE _ZNK8SkMatrix15computeTypeMaskEv _ZNK8SkMatrix17invertNonIdentityEPS_ _ZN7android19VirtualLightRefBaseD2Ev _ZTVN7android19VirtualLightRefBaseE _ZN7android19VirtualLightRefBaseD1Ev _ZN7android10uirenderer17AnimationListenerD2Ev _ZN7android10uirenderer17AnimationListenerD1Ev _ZN7android19VirtualLightRefBaseD0Ev _ZN7android10uirenderer17AnimationListenerD0Ev _ZN7android10uirenderer22BaseRenderNodeAnimator15setInterpolatorEPNS0_12InterpolatorE _ZN7android10uirenderer22BaseRenderNodeAnimator13setStartValueEf _ZN7android10uirenderer14RevealAnimatorC1Eiiff _ZN7android10uirenderer27CanvasPropertyPaintAnimatorC1EPNS0_19CanvasPropertyPaintENS1_10PaintFieldEf _ZN7android10uirenderer31CanvasPropertyPrimitiveAnimatorC1EPNS0_23CanvasPropertyPrimitiveEf _ZN7android10uirenderer22RenderPropertyAnimatorC1ENS1_14RenderPropertyEf _ZN7android10uirenderer22BaseRenderNodeAnimator13setStartDelayEl _ZN7android10uirenderer22BaseRenderNodeAnimator11setDurationEl _ZTVN7android23AnimationListenerBridgeE _ZN7android23AnimationListenerBridge19onAnimationFinishedEPNS_10uirenderer22BaseRenderNodeAnimatorE _ZN7android23AnimationListenerBridgeD2Ev _ZN7android23AnimationListenerBridgeD1Ev _ZN7android23AnimationListenerBridgeD0Ev _ZTVN7android10uirenderer17AnimationListenerE _ZN7android15VelocityTrackerD1Ev _ZN7android20VelocityTrackerStateC2EPKc _ZN7android15VelocityTrackerC1EPKc _ZN7android20VelocityTrackerStateC1EPKc _ZN7android20VelocityTrackerState5clearEv _ZN7android15VelocityTracker5clearEv _ZN7android20VelocityTrackerState11addMovementEPKNS_11MotionEventE _ZN7android15VelocityTracker11addMovementEPKNS_11MotionEventE _ZN7android20VelocityTrackerState22computeCurrentVelocityEif _ZNK7android15VelocityTracker11getVelocityEjPfS1_ _ZN7android20VelocityTrackerState11getVelocityEiPfS1_ _ZN7android20VelocityTrackerState12getEstimatorEiPNS_15VelocityTracker9EstimatorE _ZNK7android15VelocityTracker12getEstimatorEjPNS0_9EstimatorE u_charMirror_53 u_getIntPropertyValue_53 u_charDirection_53 ubidi_openSized_53 ubidi_setPara_53 ubidi_close_53 ubidi_getLevelAt_53 ubidi_getParaLevel_53 _ZN6icu_536LocaleC1Ev _ZN6icu_536Locale10setToBogusEv _ZN6icu_536Locale14createFromNameEPKc _ZN6icu_536LocaleaSERKS0_ _ZN6icu_536LocaleD1Ev _ZN6icu_5313BreakIterator18createLineInstanceERKNS_6LocaleER10UErrorCode utext_openUChars_53 utext_close_53 _ZN7android7String8C1EPKtm _ZN7android7String85setToERKS0_ lseek dump_backtrace_to_file __sF fprintf get_malloc_leak_info fwrite fputc qsort free_malloc_leak_info fopen fread fclose fdopen atoll mallinfo getpid fgets memtrack_proc_new memtrack_proc_get memtrack_proc_graphics_pss memtrack_proc_gl_pss memtrack_proc_other_pss memtrack_proc_destroy __sprintf_chk sscanf isspace _ZN7android16otherStats_fieldE _Z20jniThrowExceptionFmtP7_JNIEnvPKcS2_z jniThrowExceptionFmt memtrack_init _ZN7android36android_os_Debug_getLocalObjectCountEP7_JNIEnvP8_jobject _ZN7android36android_os_Debug_getProxyObjectCountEP7_JNIEnvP8_jobject _ZN7android36android_os_Debug_getDeathObjectCountEP7_JNIEnvP8_jobject ashmem_get_size_region ashmem_pin_region ashmem_unpin_region jniSetFileDescriptorOfFD jniCreateFileDescriptor _ZN7android12MessageQueueD2Ev _ZTVN7android12MessageQueueE _ZN7android12MessageQueueD1Ev _ZN7android12MessageQueueD0Ev _ZN7android18NativeMessageQueueD2Ev _ZTVN7android18NativeMessageQueueE _ZN7android18NativeMessageQueueD1Ev _ZN7android18NativeMessageQueueD0Ev _ZNK7android6Looper8isIdlingEv _ZN7android18NativeMessageQueue14raiseExceptionEP7_JNIEnvPKcP11_jthrowable _ZN7android12MessageQueueC2Ev _ZN7android12MessageQueueC1Ev _ZN7android18NativeMessageQueueC2Ev _ZN7android6LooperC1Eb _ZN7android6Looper12setForThreadERKNS_2spIS0_EE _ZN7android18NativeMessageQueueC1Ev _ZN7android18NativeMessageQueue8pollOnceEP7_JNIEnvi _ZN7android6Looper8pollOnceEiPiS1_PPv _ZN7android18NativeMessageQueue4wakeEv _ZN7android6Looper4wakeEv _ZN7android14IPCThreadState4selfEv _ZN7android8String16C1EPKtm _ZNK7android6Parcel16enforceInterfaceERKNS_8String16EPNS_14IPCThreadStateE _ZN7android28set_dalvik_blockguard_policyEP7_JNIEnvi _ZN7android6Parcel19writeInterfaceTokenERKNS_8String16E _ZNK7android6Parcel18hasFileDescriptorsEv _ZN7android6Parcel10appendFromEPKS0_mm _ZN7android23signalExceptionForErrorEP7_JNIEnvP8_jobjectib _ZN7android6Parcel11setDataSizeEm _ZNK7android6Parcel15setDataPositionEm _ZN7android6Parcel12writeInplaceEm _ZNK7android6Parcel8dataSizeEv _ZN7android6ParcelD1Ev _ZN7android6Parcel8freeDataEv _ZN7android6ParcelC1Ev _ZNK7android6Parcel19readString16InplaceEPm _ZNK7android6Parcel10readDoubleEv _ZNK7android6Parcel9readFloatEv _ZNK7android6Parcel9readInt64Ev _ZN7android6Parcel4BlobC2Ev _ZNK7android6Parcel8readBlobEmPNS0_12ReadableBlobE _ZN7android6Parcel4Blob7releaseEv _ZN7android6Parcel4BlobD2Ev _ZNK7android6Parcel9dataAvailEv _ZNK7android6Parcel11readInplaceEm _ZN7android6Parcel13writeString16EPKtm _ZN7android6Parcel11writeDoubleEd _ZN7android6Parcel10writeFloatEf _ZN7android6Parcel10writeInt64El _ZN7android6Parcel9writeBlobEmPNS0_12WritableBlobE _ZN7android6Parcel15restoreAllowFdsEb _ZN7android6Parcel12pushAllowFdsEb _ZN7android6Parcel15setDataCapacityEm _ZNK7android6Parcel12dataCapacityEv _ZNK7android6Parcel12dataPositionEv _ZNK7android6Parcel12objectsCountEv _ZNK7android6Parcel4dataEv _ZN7android22createJavaParcelObjectEP7_JNIEnv _ZN7android23recycleJavaParcelObjectEP7_JNIEnvP8_jobject security_getenforce getpidcon freecon getcon security_get_boolean_names security_setenforce getpeercon selinux_android_restorecon security_get_boolean_active security_set_boolean security_commit_booleans getfilecon setfscreatecon setfilecon selinux_check_access selinux_set_callback is_selinux_enabled _ZN7android19elapsedRealtimeNanoEv gettimeofday clock_gettime _ZN7android15elapsedRealtimeEv _ZN7android12uptimeMillisEv strtol _ZN7android27add_sysprop_change_callbackEPFvvEi property_set atrace_set_tracing_enabled atrace_set_debuggable atrace_is_ready android_atomic_acquire_load atrace_enabled_tags atrace_setup atrace_marker_fd _ZN7android7String810lockBufferEm _ZN7android7String812unlockBufferEv _ZNK7android6VectorINS_7String8EE10do_destroyEPvm uevent_next_event _ZN7android6VectorINS_7String8EED2Ev _ZTVN7android6VectorINS_7String8EEE _ZN7android6VectorINS_7String8EED1Ev _ZN7android6VectorINS_7String8EED0Ev _ZNK7android6VectorINS_7String8EE12do_constructEPvm _ZNK7android6VectorINS_7String8EE7do_c