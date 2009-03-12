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

#ifndef DISTORE_CONFIG_H
#define DISTORE_CONFIG_H

#include <sys/time.h>
#include <ght_hash_table.h>

/* Load config from specified ini file.
 * Returns 0 on success and negative value otherwise
 * 
 * After successfull call to this function, config
 * structure may be obtained using getConfig()
 */
int loadConfig(char *path);

/* Structure to hold our configuration. Initialized from ini file */
struct Config {
	char * secret;
	char * multicastGroup;
	char * unicastTargets;
	unsigned int listenPort;
	struct timeval announcePeriod;
	struct timeval checkDoUpdatePeriod;
	ght_hash_table_t * contents;
};

struct Config* getConfig();

/* Represents one content secion of ini file.
 * "Content"s are stored in "contents" hash table of Config struct.
 */
struct Content {
	unsigned int id;
	char * contentDir;
	char * filePattern;
	char * installScript;
};

#endif
