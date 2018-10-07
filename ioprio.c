/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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

#include "defs.h"
#include "xstring.h"

#include "xlat/ioprio_who.h"
#include "xlat/ioprio_class.h"

#define IOPRIO_CLASS_SHIFT	(13)
#define IOPRIO_PRIO_MASK	((1ul << IOPRIO_CLASS_SHIFT) - 1)

#define IOPRIO_PRIO_CLASS(mask)	((mask) >> IOPRIO_CLASS_SHIFT)
#define IOPRIO_PRIO_DATA(mask)	((mask) & IOPRIO_PRIO_MASK)

static const char *
sprint_ioprio(unsigned int ioprio)
{
	static char outstr[256];
	char class_buf[64];
	unsigned int class, data;

	class = IOPRIO_PRIO_CLASS(ioprio);
	data = IOPRIO_PRIO_DATA(ioprio);
	sprintxval(class_buf, sizeof(class_buf), ioprio_class, class,
		   "IOPRIO_CLASS_???");
	xsprintf(outstr, "IOPRIO_PRIO_VALUE(%s, %d)", class_buf, data);

	return outstr;
}

void
print_ioprio(unsigned int ioprio)
{
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		tprintf("%#x", ioprio);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	const char *str = sprint_ioprio(ioprio);

	(xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE
		? tprints_comment : tprints)(str);
}

static void
print_ioprio_who(const char * prefix, int which, int who)
{
	switch (which) {
	case IOPRIO_WHO_USER:
		printuid(prefix, who);
		break;
	case IOPRIO_WHO_PROCESS:
	case IOPRIO_WHO_PGRP:
	default:
		tprintf("%s%d", prefix, who);
	}
}

SYS_FUNC(ioprio_get)
{
	if (entering(tcp)) {
		/* int which */
		printxval(ioprio_who, tcp->u_arg[0], "IOPRIO_WHO_???");
		/* int who */
		print_ioprio_who(", ", tcp->u_arg[0], tcp->u_arg[1]);
		return 0;
	} else {
		if (syserror(tcp))
			return 0;

		tcp->auxstr = sprint_ioprio(tcp->u_rval);
		return RVAL_STR;
	}
}

SYS_FUNC(ioprio_set)
{
	/* int which */
	printxval(ioprio_who, tcp->u_arg[0], "IOPRIO_WHO_???");
	/* int who */
	print_ioprio_who(", ", tcp->u_arg[0], tcp->u_arg[1]);
	/* int ioprio */
	tprints(", ");
	tprints(sprint_ioprio(tcp->u_arg[2]));

	return RVAL_DECODED;
}
