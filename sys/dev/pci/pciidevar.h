/*	$OpenBSD: pciidevar.h,v 1.9 2004/10/17 17:58:30 grange Exp $	*/
/*	$NetBSD: pciidevar.h,v 1.6 2001/01/12 16:04:00 bouyer Exp $	*/

/*
 * Copyright (c) 1998 Christopher G. Demetriou.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Christopher G. Demetriou
 *	for the NetBSD Project.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
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

#ifndef _DEV_PCI_PCIIDEVAR_H_
#define _DEV_PCI_PCIIDEVAR_H_

/*
 * PCI IDE driver exported software structures.
 *
 * Author: Christopher G. Demetriou, March 2, 1998.
 */

struct device;

/*
 * While standard PCI IDE controllers only have 2 channels, it is
 * common for PCI SATA controllers to have more.  Here we define
 * the maximum number of channels that any one PCI IDE device can
 * have.
 */
#define PCIIDE_MAX_CHANNELS	4

/*
 * Functions defined by machine-dependent code.
 */

/* Attach compat interrupt handler, returning handle or NULL if failed. */
#if !defined(pciide_machdep_compat_intr_establish)
void	*pciide_machdep_compat_intr_establish(struct device *,
	    struct pci_attach_args *, int, int (*)(void *), void *);
void	pciide_machdep_compat_intr_disestablish(pci_chipset_tag_t pc,
	    void *);
#endif

#endif	/* !_DEV_PCI_PCIIDEVAR_H_ */
