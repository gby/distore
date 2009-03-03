/*****************************************************************************
 *
 *    Copyright (C) 2009 Codefidence Ltd www.codefidence.com
 *
 *    This file is a part of Distore
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

#ifndef DISTORE_DEBUG_H
#define DISTORE_DEBUG_H

#ifndef DEBUG
/* gcc's cpp has extensions; it allows for macros with a variable number of
 *    arguments. We use this extension here to preprocess dmesg away. */
#define dmesg(level, format, args...) ((void)0)
#else

enum {
	DBG_ERROR = 1,
	DBG_WARN,
	DBG_INFO,
	DBG_DEBUG,
	DBG_TRACE
};

void dmesg(int level, char *format, ...);
/* print a message, if it is considered significant enough.
 *       Adapted from [K&R2], p. 174 */
#endif

#endif
        

