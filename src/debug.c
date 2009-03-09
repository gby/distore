/*****************************************************************************
 *
 *    Copyright (C) 2009 Codefidence Ltd www.codefidence.com
 *
 *    This file is a part of Distore.
 *
 *    Distore is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Distore is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Distore.  If not, see <http://www.gnu.org/licenses/>.
 * 
 ****************************************************************************/

#include <stdio.h>
#include <stdarg.h>

#include "debug.h"

extern int dbg_msglevel; /* the higher, the more messages... */

#ifndef DEBUG
/* Nothing. dmesg has been "defined away" in debug.h already. */
#else
void dmesg(int level, char* format, ...) {
	va_list args;
	char *prefix;

	if (level>dbg_msglevel)
		return;

	switch(level) {
		case DBG_ERROR: prefix="-E- "; break;
		case DBG_WARN: prefix="-W- "; break;
		case DBG_INFO: prefix="-I- "; break;
		case DBG_DEBUG: prefix="-D- "; break;
		case DBG_TRACE: prefix="-T- "; break;
		default: prefix="---"; break;
	}
	fprintf(stderr, "%s", prefix);

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#endif /* NDEBUG */
        
