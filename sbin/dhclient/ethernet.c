/*	$OpenBSD: ethernet.c,v 1.3 2004/02/04 12:16:56 henning Exp $	*/

/* Packet assembly code, originally contributed by Archie Cobbs. */

/*
 * Copyright (c) 1995, 1996 The Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */

#include "dhcpd.h"

#include <netinet/if_ether.h>
#define ETHER_HEADER_SIZE (ETHER_ADDR_LEN * 2 + sizeof (u_int16_t))

/* Assemble an hardware header... */
/* XXX currently only supports ethernet; doesn't check for other types. */

void
assemble_ethernet_header(struct interface_info *interface, unsigned char *buf,
    int *bufix, struct hardware *to)
{
	struct ether_header eh;

	if (to && to -> hlen == 6) /* XXX */
		memcpy(eh.ether_dhost, to -> haddr, sizeof eh.ether_dhost);
	else
		memset(eh.ether_dhost, 0xff, sizeof (eh.ether_dhost));
	if (interface->hw_address.hlen == sizeof(eh.ether_shost))
		memcpy(eh.ether_shost, interface->hw_address.haddr,
		    sizeof(eh.ether_shost));
	else
		memset(eh.ether_shost, 0x00, sizeof(eh.ether_shost));

	eh.ether_type = htons(ETHERTYPE_IP);

	memcpy(&buf[*bufix], &eh, ETHER_HEADER_SIZE);
	*bufix += ETHER_HEADER_SIZE;
}

/* Decode a hardware header... */

ssize_t
decode_ethernet_header(struct interface_info *interface, unsigned char *buf,
    int bufix, struct hardware *from)
{
	struct ether_header eh;

	memcpy(&eh, buf + bufix, ETHER_HEADER_SIZE);

	memcpy(from->haddr, eh.ether_shost, sizeof(eh.ether_shost));
	from->htype = ARPHRD_ETHER;
	from->hlen = sizeof eh.ether_shost;

 	return sizeof eh;
}
