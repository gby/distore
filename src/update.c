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

#include <dirent.h>
#include <stdlib.h>
#include <fnmatch.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ght_hash_table.h>

#include "update.h"
#include "config.h"
#include "debug.h"
#include "messages.h"

/* since filter function is a callback, we can not pass parameters to it - 
 * doing in indirect way.
 */
static char * filterPattern = NULL;
int filter(const struct dirent *dir) {
	return ! fnmatch(filterPattern, dir->d_name, 0);
}

unsigned long getVersionFromPath(char *path, char *pattern) {
	struct dirent **namelist;
	int n;
	char *versionPtr, *endPtr;
	unsigned long version = 0;
	int base = 10; /* version is specified in decimal base */

	filterPattern = pattern;
	n = scandir(path, &namelist, filter, versionsort);
	if (n < 0) {
		fprintf(stderr, "Failed to get version from %s/%s. Assuming 0.\n", path, pattern);
	} else if (n > 0) {
	
		dmesg(DBG_DEBUG, "Selected file: %s\n", namelist[n-1]->d_name);

		versionPtr=strrchr(namelist[n-1]->d_name, '.') + 1;

		errno = 0;    /* To distinguish success/failure after call */
		version = strtol(versionPtr, &endPtr, base);


		/* Check for various possible errors */

		if ((errno == ERANGE && (version == LONG_MAX || version == LONG_MIN))
				|| (errno != 0 && version == 0)) {
			fprintf(stderr, "Failed extract version number from %s\n", versionPtr);
			version = 0; 
		}

		if (endPtr == versionPtr) {
			fprintf(stderr, "No digits were found\n");
			version = 0;
		}

		while (n--) {
			free(namelist[n]);
		}
		free(namelist); 

		dmesg(DBG_DEBUG, "Extracted version: %lu from %s/%s\n", (unsigned long) version, path, pattern);
	}

	return version;
}

static ght_hash_table_t * currentVersions = NULL;

ght_hash_table_t * getCurrentVersions() {

	if (currentVersions == NULL) {
		struct Config * distoreConfig = getConfig();
		struct Content * content = NULL;
		ght_iterator_t iterator;
		const void * key = NULL;
		unsigned long * version = NULL;

		currentVersions = ght_create(DEFAULT_HASH_SIZE);
		if (currentVersions == NULL) {
			fprintf(stderr, "Failed to create currentVersions hash table!\n");
			return NULL;
		}
		ght_set_rehash(currentVersions, 1);

		for (content = (struct Content *)ght_first(distoreConfig->contents, &iterator, &key); content;
									content = (struct Content *)ght_next(distoreConfig->contents, &iterator, &key))
		{

			version = (unsigned long *) malloc(sizeof(unsigned long));
			if (version == NULL) {
				fprintf(stderr, "Out of memory! Buy!\n");
				return NULL;
			}

			*version = getVersionFromPath(content->contentDir, content->filePattern);
			if (ght_insert(currentVersions, (void *)version, sizeof(int), (void *)&(content->id)) < 0) {
				fprintf(stderr, "Failed to insert version %u<->%lu to hash table!\n", content->id, *version);
				return NULL;
			}
		}

#ifdef DEBUG
		dmesg(DBG_DEBUG, "Scanned versions:\n");
		for (version = (unsigned long *)ght_first(currentVersions, &iterator, &key); version;
									version = (unsigned long *)ght_next(currentVersions, &iterator, &key))
			dmesg(DBG_DEBUG, "  Id=%u, version=%lu\n", *(int *)key, *version);
		dmesg(DBG_DEBUG, "\n");
#endif /* DEBUG */

	}

	return currentVersions;
}

int doUpdate(unsigned int id, unsigned long version, struct sockaddr_in * addr);

void runUpdateIfNeeded() {
	ght_hash_table_t * availableVersions = getAvailableVersions();
	ght_iterator_t iterator;
	const void * key = NULL;
	unsigned long * version = NULL;

	if (availableVersions == NULL) /* have not got any announce from network (yet) */
		return;

	for (version = (unsigned long *)ght_first(currentVersions, &iterator, &key); version;
								version = (unsigned long *)ght_next(currentVersions, &iterator, &key)) {
		struct NetVersion * netVersion = (struct NetVersion *) ght_get(availableVersions, sizeof(int), key);
		if (netVersion == NULL)
			continue;
		int id = *(int *)key;
		if (netVersion->version > *version)
			if (doUpdate(id, *version, &(netVersion->ip)) == 0) {
				*version = netVersion->version;
				updateCurrentAnnounce(id, *version);
			}
			
	}
}

int doUpdate(unsigned int id, unsigned long version, struct sockaddr_in * addr) {
	char buff[PATH_MAX];
	struct Config * distoreConfig = getConfig();
	struct Content * content = NULL;

	content = (struct Content *) ght_get(distoreConfig->contents, sizeof(int), (void *)&id);

	sprintf(buff, "%s %s %s/%s", content->installScript, 
								inet_ntoa(addr->sin_addr),
								content->contentDir,
								content->filePattern);
	sprintf(strrchr(buff, '.')+1, "%lu", version);

	dmesg(DBG_DEBUG, "Running update installation: %s\n", buff);
	if (system(buff) != 0) {
		fprintf(stderr, "Update installation failed!!!\n");
		return -1;
	}
	
	dmesg(DBG_DEBUG, "Update installation done.\n");

	return 0;
}

