
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <event.h>

#include <ght_hash_table.h>

#include "config.h"
#include "debug.h"
#include "sock.h"
#include "callbacks.h"
#include "update.h"
#include "messages.h"

#ifdef DEBUG
int dbg_msglevel = DBG_TRACE;
#endif

int main(int argc, char * argv[]) {
	
	int opt = 0;
	char * configPath = NULL;
	struct Config * distoreConfig = NULL;
	ght_hash_table_t * currentVersions = NULL;
	struct event * ev = NULL;
	int fg = 0;
	int fd = 0;

	while ((opt = getopt(argc, argv, "fc:")) != -1) {
		switch (opt) {
			case 'f':
				fg = 1;
				break;
			case 'c':
				configPath = optarg;
				break;
			default: /* ’?’ */
				fprintf(stderr, "Usage: %s -c config_file\n [-f]", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (configPath == NULL) {
		fprintf(stderr, "Usage: %s -c config_file [-f]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (loadConfig(configPath) != 0) {
		fprintf(stderr, "Failed to parse config file: %s\n", configPath);
		exit(EXIT_FAILURE);
	}

	distoreConfig = getConfig();

	currentVersions = getCurrentVersions();
	if (currentVersions == NULL) {
		fprintf(stderr, "Failed to parse contents' files!\n");
		exit(EXIT_FAILURE);
	}

	if (rebuildCurrentAnnounce(currentVersions) < 0) {
		fprintf(stderr, "Failed to build our annouce message!\n");
		exit(EXIT_FAILURE);
	}


	/* We are good to go - switch to daemon */
	if (fg != 1) {
		int result = daemon(0,1);
		if (result != 0)
			perror("Failed to daemonize");
	}

	event_init();

	/* Schedule read event */
	{
		fd = CreateDgramServerSock(distoreConfig->listenPort, distoreConfig->multicastGroup);
		if (fd < 0) {
			fprintf(stderr, "Failed to create socket!\n");
			exit(EXIT_FAILURE);
		}
		dmesg(DBG_DEBUG, "Bound to socket\n");

		if ( (ev = malloc(sizeof(struct event))) == NULL ) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		event_set(ev, fd, EV_READ, evReadAnnounce, ev);
		event_add(ev, NULL);
	}

	/* Schedule send event */
	{
		fd = CreateDgramClientSock();
		if (fd < 0) {
			fprintf(stderr, "Failed to create socket\n");
			exit(EXIT_FAILURE);
		}
		dmesg(DBG_DEBUG, "Created client socket\n");

		if ( (ev = malloc(sizeof(struct event))) == NULL ) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		/* not using evtimer_set, since we want to pass fd as well */
		event_set(ev, fd, 0, evSendAnnounce, ev);
		event_add(ev, &(distoreConfig->announcePeriod));
	}

	/* Schedule install event */
	{
		if ( (ev = malloc(sizeof(struct event))) == NULL ) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		event_set(ev, -1, 0, evRunUpdateIfNeeded, ev);
		event_add(ev, &(distoreConfig->checkDoUpdatePeriod));
	}

	dmesg(DBG_DEBUG, "Dispacting events...\n");
	event_dispatch();

	return EXIT_SUCCESS;	
}
