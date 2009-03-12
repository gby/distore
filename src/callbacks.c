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

#include <sys/types.h>
#include <stdlib.h>
#include <event.h>

#include "callbacks.h"
#include "messages.h"
#include "config.h"
#include "update.h"
#include "debug.h"

void evReadAnnounce(int fd, short event, void *arg) {
	struct event * ev = (struct event *) arg;
	readAnnounce(fd);
	event_add(ev, NULL);
}

void evSendAnnounce(int fd, short event, void *arg) {
	struct Config * distoreConfig = getConfig();
	struct event * ev = (struct event *) arg;
	sendAnnounce(fd);
	event_add(ev, &(distoreConfig->announcePeriod));
}

void evRunUpdateIfNeeded(int fd, short event, void *arg) {
	struct Config *distoreConfig = getConfig();
	struct event *ev = (struct event *) arg;
	runUpdateIfNeeded();
	event_add(ev, &(distoreConfig->checkDoUpdatePeriod));
}
