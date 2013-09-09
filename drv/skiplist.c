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
 *	$Id: //depot/skiplist/main/skiplist.c#3 $
 *
 */

#include "skiplist.h"
#include "skiplist_priv.h"

#include <stdlib.h>
#include <stdio.h>

static int randnum = 0;
static int randfbits = 0;

int
randbits(int nbits)
{
	int i;

	if (randfbits < nbits) {
		randnum = random();
		randfbits = 31;
	}
	randfbits -= nbits;
	i = randnum % (1 << nbits);
	randnum >>= nbits;

	return i;
}

int
genlvl(int maxlvl, int chunk)
{
	int lvl;
	int r;

	for (lvl = 0; (r = randbits(chunk)) && lvl < maxlvl; lvl++);

	if (lvl == maxlvl)
		lvl--;

	return lvl;
}

int
findplace(struct skiplist *sl, void *k, struct skiplist_node **btrack)
{
	int r;
	int i;
	struct skiplist_node *cnode, *pnode;
	struct skiplist_node **list;

	for (i = sl->maxlvl - 1; i > sl->curlvl; i--)
		btrack[i] = sl->head;

	pnode = sl->head;
	list = &sl->head->list[0];
	r = 0;

	while (i >= 0) {
		cnode = list[i];
		while ((r = sl->cmp(k, cnode->key)) > 0) {
			pnode = cnode;
			list = &cnode->list[0];
			cnode = list[i];
		}
		btrack[i] = pnode;
		i--;
	}
	return r;
}

void
dumplist(int lvls, struct skiplist_node **list)
{
	int i;

	for (i = 0; i < lvls; i++) {
		if (i)
			printf(", ");
		printf("lvl%d: %p", i, list[i]->list[i]);
	}
	puts("");
}

struct skiplist *
createskiplist(int (*cmp)(void *, void *), int maxlvl, int chunk, void *min,
	       void *max)
{
	struct skiplist *sl;
	struct skiplist_node *end, *h;
	int i;

	sl = malloc(sizeof *sl);

	sl->maxlvl = maxlvl;
	sl->curlvl = -1;
	sl->chunksize = chunk;
	sl->cmp = cmp;
	sl->btlist = malloc(sizeof *sl->btlist * maxlvl);
	end = allocnode(maxlvl);
	h = allocnode(maxlvl);
	end->key = max;
	h->key = min;
	for (i = 0; i < maxlvl; i++) {
		end->list[i] = NULL;
		h->list[i] = end;
	}
	sl->nil = end;
	sl->head = h;

	return sl;
}

int
insertkey(struct skiplist *sl, void *k)
{
	int l;
	struct skiplist_node **btrack, *n, **plist;

	btrack = sl->btlist;
	findplace(sl, k, btrack);

	l = genlvl(sl->maxlvl, sl->chunksize);
	n = allocnode(l);
	n->key = k;
	plist = &n->list[0];

	if (sl->curlvl < l)
		sl->curlvl = l;

	for (; l >= 0; l--) {
		plist[l] = btrack[l]->list[l];
		btrack[l]->list[l] = n;
	}

	return 0;
}

void *
searchkey(struct skiplist *sl, void *k)
{
	int r;
	struct skiplist_node **btrack;

	btrack = sl->btlist;
	r = findplace(sl, k, btrack);

	if (r != 0)
		return NULL;
	else
		return btrack[0]->list[0]->key;
}

void *
deletekey(struct skiplist *sl, void *k)
{
	int r, l;
	struct skiplist_node **btrack, *n;

	btrack = sl->btlist;
	r = findplace(sl, k, btrack);

	if (r != 0)
		return NULL;

	n = btrack[0]->list[0];
	l = 0;
	while (l < sl->maxlvl && btrack[l]->list[l] == n)
		l++;

	for (l--; sl->head->list[l] == n && n->list[l] == sl->nil; l--)
		sl->curlvl--;

	for (; l >= 0; l--)
		btrack[l]->list[l] = n->list[l];

	k = n->key;
	free(n);

	return k;
}

void
printskiplist(struct skiplist *sl)
{
	int i;
	struct skiplist_node *n;

	printf("curlvl: %d, maxlvl: %d, chunksize: %d\n", sl->curlvl,
	    sl->maxlvl, sl->chunksize);

	for (i = sl->curlvl; i >= 0; i--) {
		printf("level %d: ", i);
		n = sl->head->list[i];
		while (n != NULL) {
			printf("%p -> ", n->key);
			n = n->list[i];
		}
		puts("NULL");
	}
}
