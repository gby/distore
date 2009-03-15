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

#include <string.h>

#include <iniparser.h>
#include <ght_hash_table.h>

#include "config.h"
#include "debug.h"

static struct Config distoreConfig = {0};

int returnError(const char *section, const char *path) {
	fprintf(stderr, "Cann ot read %s value from %s (empty or missing)\n", section, path);
	return -1;
}

int loadConfig(char *path) {

	char * contentNames = NULL;
	char * newContentName = NULL;
	char * nextContentName = NULL;

	dictionary * newconf = iniparser_load(path);
	if (newconf == NULL) {
		fprintf(stderr, "Could not parse config file %s\n", path);
		return -1;
	}

	distoreConfig.secret = iniparser_getstring(newconf, "Networking:Secret", NULL);
	
	distoreConfig.multicastGroup = iniparser_getstring(newconf, "Networking:MulticastGroup", NULL);
	distoreConfig.unicastTargets = iniparser_getstring(newconf, "Networking:UnicastTargets", NULL);
	if ((distoreConfig.multicastGroup == NULL) && (distoreConfig.unicastTargets == NULL)) {
		fprintf(stderr, "One of MulticastGroup or UnicastTargets has to be defined in %s\n", path);
		return -1;
	}

	distoreConfig.listenPort = iniparser_getint(newconf, "Networking:ListenPort", 0);
	if (distoreConfig.listenPort == 0)
		return returnError("ListenPort", path);

	contentNames = iniparser_getstring(newconf, "Content:Contents", NULL);
	if (contentNames == NULL) 
		return returnError("Contents", path);

	newContentName = (char *) malloc(strlen(contentNames + 1));
	if (newContentName == NULL) {
		fprintf(stderr, "Out of memory! Buy!\n");
		return -1;
	}

	distoreConfig.announcePeriod.tv_sec = (time_t) iniparser_getint(newconf, "Networking:AnnouncePeriod", 0);
	if (distoreConfig.announcePeriod.tv_sec == 0)
		return returnError("AnnouncePeriod", path);

	distoreConfig.checkDoUpdatePeriod.tv_sec = (time_t) iniparser_getint(newconf, "Installation:CheckDoUpdatePeriod", 60);

	{ /* parse all Contents sections */
		distoreConfig.contents = ght_create(DEFAULT_HASH_SIZE);
		if (distoreConfig.contents == NULL) {
			fprintf(stderr, "Failed to create distoreConfig.contents hash table!\n");
			return -1;
		}
		ght_set_rehash(distoreConfig.contents, 1);

		do {
			struct Content * newContent = NULL;
			int    newContentNameLength = 0;

			newContent = (struct Content *) malloc(sizeof(struct Content));
			if (newContent == NULL) {
				fprintf(stderr, "Out of memory! Buy!\n");
				return -1;
			}

			nextContentName = strchrnul(contentNames, ' ');
			newContentNameLength = nextContentName - contentNames;
			strncpy(newContentName, contentNames, newContentNameLength);
			contentNames += newContentNameLength + 1;

			strcpy(newContentName + newContentNameLength, ":Id");
			newContent->id = iniparser_getint(newconf, newContentName, 0);
			if (newContent->id == 0)
				return returnError(newContentName, path);

			strcpy(newContentName + newContentNameLength, ":Dir");
			newContent->contentDir = iniparser_getstring(newconf, newContentName, NULL);
			if (newContent->contentDir == NULL)
				return returnError(newContentName, path);

			strcpy(newContentName + newContentNameLength, ":FilePattern");
			newContent->filePattern = iniparser_getstring(newconf, newContentName, NULL);
			if (newContent->filePattern == NULL)
				return returnError(newContentName, path);

			strcpy(newContentName + newContentNameLength, ":InstallScript");
			newContent->installScript = iniparser_getstring(newconf, newContentName, NULL);
			if (newContent->installScript == NULL)
				return returnError(newContentName, path);

			if (ght_insert(distoreConfig.contents, (void *)newContent, sizeof(int), (void *)&(newContent->id)) < 0) {
				newContentName[newContentNameLength]='\0';
				fprintf(stderr, "Failed to insert content %s to hash table!\n", newContentName);
				return -1;
			}
		} while (*nextContentName != '\0');
	}

	free(newContentName);

	dmesg(DBG_DEBUG, "loaded configuration:\n");
	dmesg(DBG_DEBUG, "  MulticastGoups=\"%s\"\n", distoreConfig.multicastGroup);
	dmesg(DBG_DEBUG, "  UnicastTargets=\"%s\"\n", distoreConfig.unicastTargets);
	dmesg(DBG_DEBUG, "  ListenPort=\"%d\"\n", distoreConfig.listenPort);
	dmesg(DBG_DEBUG, "  AnnouncePeriod=\"%lu\"\n", distoreConfig.announcePeriod.tv_sec);
	dmesg(DBG_DEBUG, "  CheckDoUpdatePeriod=\"%lu\"\n", distoreConfig.checkDoUpdatePeriod);

#ifdef DEBUG
	{
		struct Content * content = NULL;
		ght_iterator_t iterator;
		const void * key = NULL;

		dmesg(DBG_DEBUG, "\n");
		dmesg(DBG_DEBUG, "  Contents:\n");
		for (content = (struct Content *)ght_first(distoreConfig.contents, &iterator, &key); content;
									content = (struct Content *)ght_next(distoreConfig.contents, &iterator, &key)) {
			dmesg(DBG_DEBUG, "    Id=%u\n", content->id);
			dmesg(DBG_DEBUG, "    ContentDir=%s\n", content->contentDir);
			dmesg(DBG_DEBUG, "    FilePattern=%s\n", content->filePattern);
			dmesg(DBG_DEBUG, "    InstallScript=%s\n", content->installScript);
			dmesg(DBG_DEBUG, "\n");
		}
	}
#endif /* DEBUG */

	return 0;
}

struct Config * getConfig() {
	return &distoreConfig;
}

