/*-
 * Copyright 2006 John-Mark Gurney.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: //depot/skiplist/main/skiplist_priv.h#3 $
 *
 */

#ifndef _SKIPLIST_PRIV_H_
#define _SKIPLIST_PRIV_H_

struct skiplist_node {
	void	*key;
	struct	skiplist_node *list[1];
};

struct skiplist {
	int	curlvl;
	int	maxlvl;
	int	chunksize;
	int	(*cmp)(void *, void *);
	struct	skiplist_node *nil;
	struct	skiplist_node *head;
	struct	skiplist_node **btlist;
};

#define allocsize(lvls)		(sizeof(struct skiplist_node) + \
	(lvls * sizeof(struct skiplist_node *)))
#ifndef DEBUG_ALLOCNODE
#define allocnode(lvls) ((struct skiplist_node *)malloc(allocsize(lvls)))
#else
#define allocnode(lvls)	((fprintf(stderr, "allocnode(%d) = %d\n", lvls, \
	allocsize(lvls))), (struct skiplist_node *)malloc( \
	allocsize(lvls)))
#endif

int findplace(struct skiplist *sl, void *k, struct skiplist_node **btrack);
int randbits(int nbits);
int genlvl(int maxlvl, int chunk);
void dumplist(int lvls, struct skiplist_node **list);

#endif /* _SKIPLIST_PRIV_H_ */
