/* $OpenBSD: fuse_subr.c,v 1.9 2015/06/03 19:51:16 reyk Exp $ */
/*
 * Copyright (c) 2013 Sylvestre Gallon <ccna.syl@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fuse_private.h"
#include "debug.h"

struct fuse_vnode *
alloc_vn(struct fuse *f, const char *path, ino_t ino, ino_t parent)
{
	struct fuse_vnode *vn;

	if ((vn = malloc(sizeof(*vn))) == NULL) {
		DPERROR(__func__);
		return (NULL);
	}

	vn->ino = ino;
	vn->parent = parent;
	if (strlcpy(vn->path, path, sizeof(vn->path)) >= sizeof(vn->path)) {
		DPRINTF("%s: strlcpy name too long\n", __func__);
		free(vn);
		return (NULL);
	}

	if (ino == (ino_t)-1) {
		f->max_ino++;
		vn->ino = f->max_ino;
	}

	return (vn);
}

int
set_vn(struct fuse *f, struct fuse_vnode *v)
{
	struct fuse_vn_head *vn_head;
	struct fuse_vnode *vn;

	DPRINTF("%s: create or update vnode %llu@%llu = %s\n", __func__,
	    (unsigned long long)v->ino, (unsigned long long)v->parent,
	    v->path);

	if (tree_set(&f->vnode_tree, v->ino, v) == NULL)
		return (0);

	if (!dict_check(&f->name_tree, v->path)) {
		vn_head = malloc(sizeof(*vn_head));
		if (vn_head == NULL)
			return (0);
		SIMPLEQ_INIT(vn_head);
	} else {
		vn_head = dict_get(&f->name_tree, v->path);
		if (vn_head == NULL)
			return (0);
	}

	SIMPLEQ_FOREACH(vn, vn_head, node) {
		if (v->parent == vn->parent && v->ino == vn->ino)
			return (1);
	}

	SIMPLEQ_INSERT_TAIL(vn_head, v, node);
	dict_set(&f->name_tree, v->path, vn_head);

	return (1);
}

void
remove_vnode_from_name_tree(struct fuse *f, struct fuse_vnode *vn)
{
	struct fuse_vn_head *vn_head;
	struct fuse_vnode *v;
	struct fuse_vnode *lastv;

	vn_head = dict_get(&f->name_tree, vn->path);
	if (vn_head == NULL)
		return;

	lastv = NULL;
	SIMPLEQ_FOREACH(v, vn_head, node) {
		if (v->parent == vn->parent)
			break;

		lastv = v;
	}
	if (v == NULL)
		return;

	/* if we found the vnode remove it */
	if (v == SIMPLEQ_FIRST(vn_head))
		SIMPLEQ_REMOVE_HEAD(vn_head, node);
	else
		SIMPLEQ_REMOVE_AFTER(vn_head, lastv, node);

	/* if the queue is empty we need to remove it from the dict */
	if (SIMPLEQ_EMPTY(vn_head)) {
		vn_head = dict_pop(&f->name_tree, vn->path);
		free(vn_head);
	}
}

struct fuse_vnode *
get_vn_by_name_and_parent(struct fuse *f, uint8_t *xpath, ino_t parent)
{
	struct fuse_vn_head *vn_head;
	struct fuse_vnode *v = NULL;
	const char *path = (const char *)xpath;

	vn_head = dict_get(&f->name_tree, path);

	if (vn_head == NULL)
		goto fail;

	SIMPLEQ_FOREACH(v, vn_head, node)
		if (v->parent == parent)
			return (v);

fail:
	errno = ENOENT;
	return (NULL);
}

char *
build_realname(struct fuse *f, ino_t ino)
{
	struct fuse_vnode *vn;
	char *name = strdup("/");
	char *tmp = NULL;
	int firstshot = 0, ret;

	vn = tree_get(&f->vnode_tree, ino);
	if (!vn || !name) {
		free(name);
		return (NULL);
	}

	while (vn->parent != 0) {
		if (firstshot++)
			ret = asprintf(&tmp, "/%s%s", vn->path, name);
		else
			ret = asprintf(&tmp, "/%s", vn->path);

		if (ret == -1) {
			free(name);
			return (NULL);
		}

		free(name);
		name = tmp;
		tmp = NULL;
		vn = tree_get(&f->vnode_tree, vn->parent);

		if (!vn)
			return (NULL);
	}

	if (ino == (ino_t)0)
		DPRINTF("%s: NULL ino\n", __func__);

	DPRINTF("realname %s\n", name);
	return (name);
}
