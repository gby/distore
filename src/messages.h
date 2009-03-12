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

#include <ght_hash_table.h>

#include "hash.h"

#define PROTO_MAGIC 0xC60C1DC1
#define PROTO_VERSION 0

/* Update current announce (the one that is broadcasted) according
 * to getCurrentVersions().
 * May fail with negative return value due to insufficient memory.
 */
int rebuildCurrentAnnounce(ght_hash_table_t * currentVersions);

/* Update specific version in current announce
 */
void updateCurrentAnnounce(unsigned int id, unsigned long version);

struct NetVersion {
	unsigned long version;
	struct sockaddr_in ip;
};

/* Return most current versions available from recieved announces.
 * Hash keys are versions' ids and hash values are NetVersion structs.
 */
ght_hash_table_t * getAvailableVersions();

/* Read announce from socket. Data is stored to memory and may be accessed elsewhere by calling getCurrentAnnounce() */
void readAnnounce(int fd);

/* Send announce using given socket to all targets specified in config */
void sendAnnounce(int fd);

#endif
