/*	$OpenBSD: uvm_anon.c,v 1.44 2015/08/21 16:04:35 visa Exp $	*/
/*	$NetBSD: uvm_anon.c,v 1.10 2000/11/25 06:27:59 chs Exp $	*/

/*
 * Copyright (c) 1997 Charles D. Cranor and Washington University.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * uvm_anon.c: uvm anon ops
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/pool.h>
#include <sys/kernel.h>
#include <sys/atomic.h>

#include <uvm/uvm.h>
#include <uvm/uvm_swap.h>

struct pool uvm_anon_pool;

/*
 * allocate anons
 */
void
uvm_anon_init(void)
{
	pool_init(&uvm_anon_pool, sizeof(struct vm_anon), 0, 0,
	    PR_WAITOK, "anonpl", NULL);
	pool_sethiwat(&uvm_anon_pool, uvmexp.free / 16);
}

/*
 * allocate an anon
 */
struct vm_anon *
uvm_analloc(void)
{
	struct vm_anon *anon;

	anon = pool_get(&uvm_anon_pool, PR_NOWAIT);
	if (anon) {
		mtx_init(&anon->an_lock, IPL_NONE);
		anon->an_ref = 1;
		anon->an_page = NULL;
		anon->an_swslot = 0;
		mtx_enter(&anon->an_lock);
	}
	return(anon);
}

/*
 * uvm_anfree: free a single anon structure
 *
 * => caller must remove anon from its amap before calling (if it was in
 *	an amap).
 * => anon must be locked and have a zero reference count.
 * => we may lock the pageq's.
 */
void
uvm_anfree(struct vm_anon *anon)
{
	struct vm_page *pg;

	UVM_ASSERT_ANONLOCKED(anon);
	KASSERT(anon->an_ref == 0);

	/* get page */
	pg = anon->an_page;

	/*
	 * if we have a resident page, we must dispose of it before freeing
	 * the anon.
	 */
	if (pg) {
		/*
		 * if page is busy then we just mark it as released (who ever
		 * has it busy must check for this when they wake up). if the
		 * page is not busy then we can free it now.
		 */
		if ((pg->pg_flags & PG_BUSY) != 0) {
			/* tell them to dump it when done */
			atomic_setbits_int(&pg->pg_flags, PG_RELEASED);
			return;
		} 
		pmap_page_protect(pg, PROT_NONE);
		uvm_lock_pageq();	/* lock out pagedaemon */
		uvm_pagefree(pg);	/* bye bye */
		uvm_unlock_pageq();	/* free the daemon */
	}
	if (pg == NULL && anon->an_swslot != 0) {
		/* this page is no longer only in swap. */
		simple_lock(&uvm.swap_data_lock);
		KASSERT(uvmexp.swpgonly > 0);
		uvmexp.swpgonly--;
		simple_unlock(&uvm.swap_data_lock);
	}

	/* free any swap resources. */
	uvm_anon_dropswap(anon);

	/*
	 * now that we've stripped the data areas from the anon, free the anon
	 * itself!
	 */
	KASSERT(anon->an_page == NULL);
	KASSERT(anon->an_swslot == 0);

	mtx_leave(&anon->an_lock);

	pool_put(&uvm_anon_pool, anon);
}

/*
 * uvm_anon_dropswap:  release any swap resources from this anon.
 * 
 * => anon must be locked 
 */
void
uvm_anon_dropswap(struct vm_anon *anon)
{
	UVM_ASSERT_ANONLOCKED(anon);

	if (anon->an_swslot == 0)
		return;

	uvm_swap_free(anon->an_swslot, 1);
	anon->an_swslot = 0;
}

/*
 * fetch an anon's page.
 *
 * => anon must be locked, and is unlocked upon return.
 * => returns TRUE if pagein was aborted due to lack of memory.
 */

boolean_t
uvm_anon_pagein(struct vm_anon *anon)
{
	struct vm_page *pg;
	struct uvm_object *uobj;
	int rv;

	UVM_ASSERT_ANONLOCKED(anon);
	rv = uvmfault_anonget(NULL, NULL, anon);
	/*
	 * if rv == VM_PAGER_OK, anon is still locked, else anon
	 * is unlocked
	 */

	switch (rv) {
	case VM_PAGER_OK:
		UVM_ASSERT_ANONLOCKED(anon);
		break;
	case VM_PAGER_ERROR:
	case VM_PAGER_REFAULT:
		UVM_ASSERT_ANONUNLOCKED(anon);

		/*
		 * nothing more to do on errors.
		 * VM_PAGER_REFAULT can only mean that the anon was freed,
		 * so again there's nothing to do.
		 */
		return FALSE;
	default:
#ifdef DIAGNOSTIC
		panic("anon_pagein: uvmfault_anonget -> %d", rv);
#else
		return FALSE;
#endif
	}

	/*
	 * ok, we've got the page now.
	 * mark it as dirty, clear its swslot and un-busy it.
	 */
	pg = anon->an_page;
	uobj = pg->uobject;
	uvm_swap_free(anon->an_swslot, 1);
	anon->an_swslot = 0;
	atomic_clearbits_int(&pg->pg_flags, PG_CLEAN);

	/* deactivate the page (to put it on a page queue) */
	uvm_lock_pageq();
	uvm_pagedeactivate(pg);
	uvm_unlock_pageq();

	/*
	 * unlock the anon and we're done.
	 */

	mtx_leave(&anon->an_lock);
	if (uobj) {
		mtx_leave(&uobj->vmobjlock);
	}
	return FALSE;
}
