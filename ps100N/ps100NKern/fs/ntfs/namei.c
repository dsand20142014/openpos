/*
 * namei.c - NTFS kernel directory inode operations. Part of the Linux-NTFS
 *	     project.
 *
 * Copyright (c) 2001-2006 Anton Altaparmakov
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

#include <linux/dcache.h>
#include <linux/exportfs.h>
#include <linux/security.h>

#include "attrib.h"
#include "debug.h"
#include "dir.h"
#include "mft.h"
#include "ntfs.h"

/**
 * ntfs_lookup - find the inode represented by a dentry in a directory inode
 * @dir_ino:	directory inode in which to look for the inode
 * @dent:	dentry representing the inode to look for
 * @nd:		lookup nameidata
 *
 * In short, ntfs_lookup() looks for the inode represented by the dentry @dent
 * in the directory inode @dir_ino and if found attaches the inode to the
 * dentry @dent.
 *
 * In more detail, the dentry @dent specifies which inode to look for by
 * supplying the name of the inode in @dent->d_name.name. ntfs_lookup()
 * converts the name to Unicode and walks the contents of the directory inode
 * @dir_ino looking for the converted Unicode name. If the name is found in the
 * directory, the corresponding inode is loaded by calling ntfs_iget() on its
 * inode number and the inode is associated with the dentry @dent via a call to
 * d_splice_alias().
 *
 * If the name is not found in the directory, a NULL inode is inserted into the
 * dentry @dent via a call to d_add(). The dentry is then termed a negative
 * dentry.
 *
 * Only if an actual error occurs, do we return an error via ERR_PTR().
 *
 * In order to handle the case insensitivity issues of NTFS with regards to the
 * dcache and the dcache requiring only one dentry per directory, we deal with
 * dentry aliases that only differ in case in ->ntfs_lookup() while maintaining
 * a case sensitive dcache. This means that we get the full benefit of dcache
 * speed when the file/directory is looked up with the same case as returned by
 * ->ntfs_readdir() but that a lookup for any other case (or for the short file
 * name) will not find anything in dcache and will enter ->ntfs_lookup()
 * instead, where we search the directory for a fully matching file name
 * (including case) and if that is not found, we search for a file name that
 * matches with different case and if that has non-POSIX semantics we return
 * that. We actually do only one search (case sensitive) and keep tabs on
 * whether we have found a case insensitive match in the process.
 *
 * To simplify matters for us, we do not treat the short vs long filenames as
 * two hard links but instead if the lookup matches a short filename, we
 * return the dentry for the corresponding long filename instead.
 *
 * There are three cases we need to distinguish here:
 *
 * 1) @dent perfectly matches (i.e. including case) a directory entry with a
 *    file name in the WIN32 or POSIX namespaces. In this case
 *    ntfs_lookup_inode_by_name() will return with name set to NULL and we
 *    just d_splice_alias() @dent.
 * 2) @dent matches (not including case) a directory entry with a file name in
 *    the WIN32 namespace. In this case ntfs_lookup_inode_by_name() will return
 *    with name set to point to a kmalloc()ed ntfs_name structure containing
 *    the properly cased little endian Unicode name. We convert the name to the
 *    current NLS code page, search if a dentry with this name already exists
 *    and if so return that instead of @dent.  At this point things are
 *    complicated by the possibility of 'disconnected' dentries due to NFS
 *    which we deal with appropriately (see the code comments).  The VFS will
 *    then destroy the old @dent and use the one we returned.  If a dentry is
 *    not found, we allocate a new one, d_splice_alias() it, and return it as
 *    above.
 * 3) @dent matches either perfectly or not (i.e. we don't care about case) a
 *    directory entry with a file name in the DOS namespace. In this case
 *    ntfs_lookup_inode_by_name() will return with name set to point to a
 *    kmalloc()ed ntfs_name structure containing the mft reference (cpu endian)
 *    of the inode. We use the mft reference to read the inode and to find the
 *    file name in the WIN32 namespace corresponding to the matched short file
 *    name. We then convert the name to the current NLS code page, and proceed
 *    searching for a dentry with this name, etc, as in case 2), above.
 *
 * Locking: Caller must hold i_mutex on the directory.
 */
static struct dentry *ntfs_lookup(struct inode *dir_ino, struct dentry *dent,
		struct nameidata *nd)
{
	ntfs_volume *vol = NTFS_SB(dir_ino->i_sb);
	struct inode *dent_inode;
	ntfschar *uname;
	ntfs_name *name = NULL;
	MFT_REF mref;
	unsigned long dent_ino;
	int uname_len;

	ntfs_debug("Looking up %s in directory inode 0x%lx.",
			dent->d_name.name, dir_ino->i_ino);
	/* Convert the name of the dentry to Unicode. */
	uname_len = ntfs_nlstoucs(vol, dent->d_name.name, dent->d_name.len,
			&uname);
	if (uname_len < 0) {
		if (uname_len != -ENAMETOOLONG)
			ntfs_error(vol->sb, "Failed to convert name to "
					"Unicode.");
		return ERR_PTR(uname_len);
	}
	mref = ntfs_lookup_inode_by_name(NTFS_I(dir_ino), uname, uname_len,
			&name);
	kmem_cache_free(ntfs_name_cache, uname);
	if (!IS_ERR_MREF(mref)) {
		dent_ino = MREF(mref);
		ntfs_debug("Found inode 0x%lx. Calling ntfs_iget.", dent_ino);
		dent_inode = ntfs_iget(vol->sb, dent_ino);
		if (likely(!IS_ERR(dent_inode))) {
			/* Consistency check. */
			if (is_bad_inode(dent_inode) || MSEQNO(mref) ==
					NTFS_I(dent_inode)->seq_no ||
					dent_ino == FILE_MFT) {
				/* Perfect WIN32/POSIX match. -- Case 1. */
				if (!name) {
					ntfs_debug("Done.  (Case 1.)");
					return d_splice_alias(dent_inode, dent);
				}
				/*
				 * We are too indented.  Handle imperfect
				 * matches and short file names further below.
				 */
				goto handle_name;
			}
			ntfs_error(vol->sb, "Found stale reference to inode "
					"0x%lx (reference sequence number = "
					"0x%x, inode sequence number = 0x%x), "
					"returning -EIO. Run chkdsk.",
					dent_ino, MSEQNO(mref),
					NTFS_I(dent_inode)->seq_no);
			iput(dent_inode);
			dent_inode = ERR_PTR(-EIO);
		} else
			ntfs_error(vol->sb, "ntfs_iget(0x%lx) failed with "
					"error code %li.", dent_ino,
					PTR_ERR(dent_inode));
		kfree(name);
		/* Return the error code. */
		return (struct dentry *)dent_inode;
	}
	/* It is guaranteed that @name is no longer allocated at this point. */
	if (MREF_ERR(mref) == -ENOENT) {
		ntfs_debug("Entry was not found, adding negative dentry.");
		/* The dcache will handle negative entries. */
		d_add(dent, NULL);
		ntfs_debug("Done.");
		return NULL;
	}
	ntfs_error(vol->sb, "ntfs_lookup_ino_by_name() failed with error "
			"code %i.", -MREF_ERR(mref));
	return ERR_PTR(MREF_ERR(mref));
	// TODO: Consider moving this lot to a separate function! (AIA)
handle_name:
   {
	MFT_RECORD *m;
	ntfs_attr_search_ctx *ctx;
	ntfs_inode *ni = NTFS_I(dent_inode);
	int err;
	struct qstr nls_name;

	nls_name.name = NULL;
	if (name->type != FILE_NAME_DOS) {			/* Case 2. */
		ntfs_debug("Case 2.");
		nls_name.len = (unsigned)ntfs_ucstonls(vol,
				(ntfschar*)&name->name, name->len,
				(unsigned char**)&nls_name.name, 0);
		kfree(name);
	} else /* if (name->type == FILE_NAME_DOS) */ {		/* Case 3. */
		FILE_NAME_ATTR *fn;

		ntfs_debug("Case 3.");
		kfree(name);

		/* Find the WIN32 name corresponding to the matched DOS name. */
		ni = NTFS_I(dent_inode);
		m = map_mfINDX( 	                 (   ё   и      РРРР  . a         Ш9     Ђ j     ј9     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     Ш9     Ђ j     ј9     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     Ђ j     ј9     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     Ђ j     ј9     `Lcщ;ЋеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     ј9     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     ј9     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     ј9     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     ј9     `Lcщ;ЋеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     ј9     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19Ќ                       i p u _ s t i l l . c . s v n - b a s e                     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     Ш9     Ђ j     ј9     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     ј9     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     ј9     `Lcщ;ЋРеТ
ЈО€’cщ;Ћ-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     Ђ j     ј9     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     `Lcщ;ЋРеТ
ЈО€’cщ;ЋР-o19ЌР                       i p u _ s t i l l . c . s v n - b a s e                     Uщ;ЋР‘Vк@ЌР P      јC               m t 9 v 1 1 1 . h . s v n - b a s e   Ѕ9     x h     ј9     ђЏ@щ;ЋРHEЭ
ЈОмBщ;ЋРг<AЌР        ё             m x 2 7 _ c s i . c . s v n - b a s e У9     x h     ј9     H!`щ;ЋРзЛЮ
ЈОоH`щ;ЋРЎwJAЌР        }               m x 2 7 _ c s i . h . s v n - b a s e Е9     x h     ј9     p!Vщ;ЋР8Ъ
ЈО
UVщ;ЋРµJH@ЌР 0      S(               m x 2 7 _ p r p . h . s v n - b a s e Ж9     Ђ l     ј9     €ђVщ;ЋРьг
ЈОљвVщ;ЋРУ1чAЌР p      a               m x 2 7 _ p r p h w . c . s v n - b a s e     С9     Ђ l     ј9     љ _щ;ЋР(чЬ
ЈО
o_щ;ЋРЏл@ЌР p      g               m x 2 7 _ p r p s w c . s v n - b a s e     Й9     € x     ј9     6ЋZщ;ЋР\ЉШ
ЈО”9[щ;ЋРf/А?ЌР      =              m x c _ v 4 l 2 _ c a p t u r e . c . s v n - b a s e Щ9     x d     ј9     fТcщ;ЋР4 в
ЈО`7dщ;ЋР^ЅNAЌР ђ      	‹               o v 2 6 4 0 . c . s v n - b a s e     И9     x d     ј9     d¶Yщ;ЋРђФ
ЈО4OZщ;ЋР»?ЌР `      9Z               o v 2 6 5 9 . c . s v n - b a s e     Л9     x d     ј9     \щ;ЋРЅЧ
ЈОВ7\щ;ЋРчї?ЌР       Ў
               o v 2 6 5 9 . h . s v n - b s e     Н9     Ђ l     ј9     ]щ;ЋР”кв
ЈОrm]щ;ЋР~ґВAЌР p      1l               o v 2 6 5 9 _ r e g . c . s v n - b a s e     Ц9     x d     ј9     eaщ;ЋРСHТ
ЈОTхaщ;ЋРµъ4ЌР а      yЫ               o v 3 6 4 0 . c . s v n - b a s e     Ъ9     Ђ p     ј9     Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e                                                                                                                             INDX( 	                (   ё   и           _ e               Ъ9     Ђ p     ј9     Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e               Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e               ј9     Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e               Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и              s e n s o r _ c l o c k . c . s v n - b a s e               Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e               Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e               Ђ p     ј9     Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e               Ђ p     ј9     Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и              s e n s o r _ c l o c k . c . s v n - b a s e               ј9     Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e               ј9     Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e               Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e               Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r  c l o c k . c . s v n - b a s e               Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e               Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e               Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s e               Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r _ c l o c k . c . s v n - b a s                Жqdщ;ЋР*†Ъ
ЈОа“dщ;ЋР%\I@ЌР       и               s e n s o r 