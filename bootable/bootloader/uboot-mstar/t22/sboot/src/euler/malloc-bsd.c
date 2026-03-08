/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
/*
 * This is a very fast storage allocator.  It allocates blocks of a small
 * number of different sizes, and keeps free lists of each size.  Blocks that
 * don't exactly fit are passed up to the next larger size.  In this
 * implementation, the available sizes are 2^n-4 (or 2^n-10) bytes long. This
 * is designed for use in a virtual memory environment.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>




/*
 * The overhead on a block is at least 4 bytes.  When free, this space
 * contains a pointer to the next free block, and the bottom two bits must be
 * zero.  When in use, the first byte is set to MAGIC, and the second byte is
 * the size index.  The remaining bytes are for alignment. If range checking
 * is enabled then a second word holds the size of the requested block, less
 * 1, rounded up to a multiple of sizeof(RMAGIC). The order of elements is
 * critical: ov_magic must overlay the low order bits of ov_next, and
 * ov_magic can not be a valid ov_next bit pattern.
 */
union overhead
{
  union overhead *ov_next;	/* when free */
  struct
  {
    u_char ovu_magic;		/* magic number */
    u_char ovu_index;		/* bucket # */
  }      ovu;
#define	ov_magic	ovu.ovu_magic
#define	ov_index	ovu.ovu_index
};

#define	MAGIC		0xef	/* magic # on accounting info */


/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information precedes
 * the data area returned to the user.
 */


#define	PAGESZ		4096
#define PAGEBUCKET	9	/* 4096 = 2^12, 9 = 12 - 3 */

#define	NBUCKETS	28	/* 30 , 24 */
#define	malloc_error()	return 0


static union overhead *nextf[NBUCKETS];
extern void *sbrk(unsigned int size);

static int malloc_init;		/* sbrk(0) to align page */

/*
 * Search ``srchlen'' elements of each free list for a block whose header
 * starts at ``freep''.  If srchlen is -1 search the whole list. Return
 * bucket number, or -1 if not found.
 */

static int
findbucket(union overhead *freep, int srchlen)
{
  register union overhead *p;
  register int i, j;

  for (i = 0; i < NBUCKETS; i++)
  {
    for (j = 0, p = nextf[i]; p; p = p->ov_next)
    {
      if (p == freep)
	return (i);
      if (++j == srchlen)
	break;
    }
  }
  return (-1);
}


void *
malloc(nbytes)
  size_t nbytes;
{
  register union overhead *op;
  register int bucket, siz, amt;

  /*
   * Convert amount of memory requested into closest block size stored in
   * hash buckets which satisfies request. Account for space used per block
   * for accounting.
   */

  if (nbytes <= PAGESZ - sizeof(*op))
  {
    amt = 8;			/* size of first bucket */
    siz = nbytes + sizeof(*op);
    bucket = 0;
  }
  else
  {
    amt = PAGESZ;
    siz = nbytes + sizeof(*op) - PAGESZ;
    bucket = PAGEBUCKET;
  }

  while (siz > amt)
  {
    amt <<= 1;
    if (++bucket >= NBUCKETS)
      malloc_error();
  }

  /* If nothing in hash bucket now, request more memory */

  if ((op = nextf[bucket]) == NULL)
  {
    /*
     * First time malloc is called, setup page size and align break pointer
     * so all data will be page aligned.
     */

    if (!malloc_init)
    {
      op = (union overhead *) sbrk(0);
      if ((siz = (int) op & (PAGESZ - 1)))
      {
	sbrk(PAGESZ - siz);
      }
      malloc_init = 1;
    }

    siz = 1 << (bucket + 3);
    amt = PAGESZ;
    if (amt < siz)
      amt += siz;

    op = (union overhead *) sbrk(amt);
    /* no more room! */
    if ((int) op == -1)
      malloc_error();

    /* Add new memory allocated to that on free list for this hash bucket. */

    if (siz < PAGESZ)
    {
      register union overhead *ox, *oy;

      amt /= siz;

      ox = op;

      while (--amt > 0)
      {
	/* force cast to void * to avoid warning
       we have check the pointer is multiple of 4 above */
	ox->ov_next = oy = (union overhead *) (void *)((caddr_t) ox + siz);
	ox = oy;
      }
	ox->ov_next = 0;
    }
    else
    {
	op->ov_next = 0;
    }
  }

  /* remove from linked list */

  nextf[bucket] = op->ov_next;
  op->ov_magic = MAGIC;
  op->ov_index = bucket;
  return ((char *) (op + 1));
}


void
free(cp)
  void *cp;
{
  register union overhead *op, **ptr;

  if (cp == NULL)
    return;

  op = (union overhead *) ((caddr_t) cp - sizeof(union overhead));
  if (op->ov_magic != MAGIC)
    return;			/* sanity */

  ptr = &nextf[op->ov_index];
  op->ov_next = *ptr;		/* also clobbers ov_magic */
  *ptr = op;
}


/*
 * When a program attempts "storage compaction" as mentioned in the old
 * malloc man page, it realloc's an already freed block.  Usually this is the
 * last block it freed; occasionally it might be farther back.  We have to
 * search all the free lists for the block in order to determine its bucket:
 * 1st we make one pass thru the lists checking only the first block in each;
 * if that fails we search ``__realloc_srchlen'' blocks in each list for a
 * match (the variable is extern so the caller can modify it).  If that fails
 * we just copy however many bytes was given to realloc() and hope it's not
 * huge.
 */


#define REALLOC_SRCHLEN  4	/* 4 should be plenty, -1 =>'s whole list */


void *
realloc(cp, nbytes)
  void *cp;
  size_t nbytes;
{
  register u_int onb;
  register int i;
  union overhead *op;
  char *res;
  int was_alloced;

  if (cp == NULL)
    return (malloc(nbytes));

  was_alloced = 0;
  op = (union overhead *) ((caddr_t) cp - sizeof(union overhead));
  if (op->ov_magic == MAGIC)
  {
    was_alloced++;
    i = op->ov_index;
  }
  else
  {
    /*
     * Already free, doing "compaction".
     * 
     * Search for the old block of memory on the free list.  First, check the
     * most common case (last element free'd), then (this failing) the last
     * ``__realloc_srchlen'' items free'd. If all lookups fail, then assume
     * the size of the memory block being realloc'd is the largest possible
     * (so that all "nbytes" of new memory are copied into).  Note that this
     * could cause a memory fault if the old area was tiny, and the moon is
     * gibbous.  However, that is very unlikely.
     */

    if ((i = findbucket(op, 1)) < 0 &&
      (i = findbucket(op, REALLOC_SRCHLEN)) < 0)
      i = NBUCKETS;
  }
  onb = 1 << (i + 3);
  if (onb < PAGESZ)
    onb -= sizeof(*op);
  else
    onb += PAGESZ - sizeof(*op);

  /* avoid the copy if same size block */

  if (was_alloced)
  {
    if (nbytes <= onb)
    {
      if (i)
      {
	i = 1 << (i + 2);
	if (i < PAGESZ)
	  i -= sizeof(*op);
	else
	  i += PAGESZ - sizeof(*op);
      }

      if ((int)nbytes > i)
	return (cp);
    }

    free(cp);
  }

  res = malloc(nbytes);
  if (cp != res)		/* common optimization if "compacting" */
    memcpy(res, cp, (nbytes < onb) ? nbytes : onb);
  return (res);
}


