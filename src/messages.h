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

#ifndef DISTORE_MESSAGES_H
#define DISTORE_MESSAGES_H

#include <netinet/in.h>

#define SHA1_LENGTH 20

#define PROTO_MAGIC 0xC60C1DC1
#define PROTO_VERSION 0

struct ContentVersion {
	unsigned int id;
	unsigned long version;
};

/* Announcement message that each node broadcasts */
struct Announce {
	unsigned int magic;                     /* The fixed magic number 0xC60C1DC1 */
	unsigned int version;                   /* Protocol version number, always 0 */
	unsigned char hash[SHA1_LENGTH];        /* SHA1 sum of the versionsCount and versionsList[] fields together with the pre-shared secret */
	unsigned int versionsCount;             /* How many items are in the below array */
	struct ContentVersion *versionsList ;   /* Actual version of distributed contents */
};

/* Return most recent annount that we've recieved from network */
struct Announce * getCurrentAnnounce();

/* Read announce from socket. Data is stored to memory and may be accessed elsewhere by calling getCurrentAnnounce() */
void readAnnounce(int fd);

/* Send announce using given socket to all targets specified in config */
void sendAnnounce(int fd);

#endif
