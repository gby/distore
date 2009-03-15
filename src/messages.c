
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ght_hash_table.h>

#include "messages.h"
#include "update.h"
#include "hash.h"
#include "config.h"
#include "debug.h"

extern int dbg_msglevel;

struct Version {
	unsigned int  id;
	unsigned long version;
};

/* Header of an announcement message that each node broadcasts */
struct AnnounceHeader {
	unsigned int magic;                     /* The fixed magic number 0xC60C1DC1 */
	unsigned int protocolVersion;           /* Protocol version number, always 0 */
	unsigned char hash[SHA1_LENGTH];        /* SHA1 sum of the Version[] fields together with the pre-shared secret */
	unsigned int versionsCount;             /* How many items follow  */
};

/* Ready-to-send announce. Data consists of AnnounceHeader followed by Version[versionsCount] array.
 * Oncly data field will be sent over network.
 */
struct Announce {
	struct AnnounceHeader * announceHeader; /* actually points to data */
	unsigned int announceHeaderLength;

	struct Version * versions; /* pointer to version arrady inside of the data */
	unsigned int versionsLength; /* length of the verions array inside of the data */

	char * data; /* actual data to be sent over network */
	unsigned int length; /* total data length */
};

static struct Announce currentAnnounce = { NULL, sizeof(struct AnnounceHeader), NULL, 0, NULL, 0 };

#ifdef DEBUG
void printAnnounce(struct AnnounceHeader *announceHeader, struct Version * versions) {

	int i=0;

	dmesg(DBG_DEBUG, "  magic=%x\n", announceHeader->magic);
	dmesg(DBG_DEBUG, "  protocolVersion=%u\n", announceHeader->protocolVersion);
	
	if (dbg_msglevel >= DBG_DEBUG) {
		dmesg(DBG_DEBUG, "  hash=");
			for(i = 0; i < SHA1_LENGTH; i++) printf("%02x", announceHeader->hash[i]);
		printf("\n");
	}

	dmesg(DBG_DEBUG, "  versionsCount=%u\n", announceHeader->versionsCount);
	dmesg(DBG_DEBUG, "  Versions:\n");
	for (i=0; i < announceHeader->versionsCount; i++)
	    dmesg(DBG_DEBUG, "    id=%u; version=%lu\n", versions[i].id, versions[i].version);

}
#endif /* DEBUG */

void signAnnounce() {
	struct Config * distoreConfig = getConfig();

	sha1sumInit();
	sha1sumUpdate((const char *)currentAnnounce.versions, currentAnnounce.versionsLength);
	sha1sumUpdate((const char *)distoreConfig->secret, strlen(distoreConfig->secret));
	sha1sumFinal(currentAnnounce.announceHeader->hash);
}

int rebuildCurrentAnnounce(ght_hash_table_t * currentVersions) {

	int i = 0;
	unsigned long * version = NULL;
	const void * key = NULL;
	ght_iterator_t iterator;
	unsigned int versionsCount = ght_size(currentVersions);

	if (currentAnnounce.data != NULL)
		free(currentAnnounce.data);

	currentAnnounce.versionsLength = versionsCount*sizeof(struct Version);
	currentAnnounce.length = currentAnnounce.announceHeaderLength + currentAnnounce.versionsLength;

	currentAnnounce.data = (char *) malloc(currentAnnounce.length);
	if (currentAnnounce.data == NULL) {
		fprintf(stderr, "Out of memory! Buy!\n");
		return -1;
	}

	currentAnnounce.announceHeader = (struct AnnounceHeader *) currentAnnounce.data;
	currentAnnounce.announceHeaderLength = sizeof(struct AnnounceHeader);
	currentAnnounce.announceHeader->magic = PROTO_MAGIC;
	currentAnnounce.announceHeader->protocolVersion = PROTO_VERSION;
	currentAnnounce.announceHeader->versionsCount = versionsCount;

	currentAnnounce.versions = (struct Version *)(currentAnnounce.data+sizeof(struct AnnounceHeader));

	for (version = (unsigned long *)ght_first(currentVersions, &iterator, &key); version;
								version = (unsigned long *)ght_next(currentVersions, &iterator, &key)) {
		currentAnnounce.versions[i].id = *(int *)key;
		currentAnnounce.versions[i].version = *version;
		i++;
	}

	signAnnounce();

#ifdef DEBUG
	dmesg(DBG_DEBUG, "Current announce:\n");
	printAnnounce(currentAnnounce.announceHeader, currentAnnounce.versions);
#endif
	return 0;
}

void updateCurrentAnnounce(unsigned int id, unsigned long version) {
	int i = 0;
	for (i=0; i < currentAnnounce.announceHeader->versionsCount; i++) {
		if (currentAnnounce.versions[i].id == id) {
			currentAnnounce.versions[i].version = version;
			signAnnounce();
			break;
		}
	}
	signAnnounce();
#ifdef DEBUG
	dmesg(DBG_DEBUG, "Updated Id=%u in current announce:\n", id);
	printAnnounce(currentAnnounce.announceHeader, currentAnnounce.versions);
#endif
}

static ght_hash_table_t * availableVersions = NULL;

ght_hash_table_t * getAvailableVersions() {
	return availableVersions;
}

struct NetVersion * allocateNetVersion(unsigned long version, struct sockaddr_in * addr) {
	struct NetVersion * netVersion = NULL;

	netVersion = (struct NetVersion *) malloc(sizeof(struct NetVersion));
	if (netVersion != NULL) {
		netVersion->version = version;
		memcpy(&(netVersion->ip), &addr, sizeof(addr));
	}
	return netVersion;
}

void readAnnounce(int fd) {
	struct sockaddr_in addr;
	unsigned int addrlen = sizeof(addr);
	int nbytes = 0;
	int ebytes = 0;
	unsigned char dhash[SHA1_LENGTH] = {0};

	static struct Announce netAnnounce = { NULL, sizeof(struct AnnounceHeader), NULL, 0, NULL, 0 };

	if (netAnnounce.data == NULL) {
		netAnnounce.data = calloc(1, sizeof(struct AnnounceHeader));
		if ( netAnnounce.data == NULL) {
			fprintf(stderr, "Out of memory while trying to initially allocate netAnnounce.data!\n");
			return;
		}
		netAnnounce.announceHeader = (struct AnnounceHeader *)netAnnounce.data;
	}

	ebytes = sizeof(struct AnnounceHeader); /* first, we only peek the header */
	if ((nbytes = recvfrom(fd, netAnnounce.announceHeader, sizeof(struct AnnounceHeader), MSG_PEEK, (struct sockaddr *) &addr, &addrlen)) < 0) {
		perror("recvfrom");
		return;
	}
	if (nbytes != ebytes) {
		fprintf(stderr, "incomplete packet recieved from %s:\n", inet_ntoa(addr.sin_addr));
		return;
	}

	{
		int netVersionsCount = netAnnounce.announceHeader->versionsCount;
		if (netAnnounce.versionsLength/sizeof(struct Version) < netVersionsCount) {
			int    newLength = sizeof(struct AnnounceHeader) + sizeof(struct Version)*netVersionsCount; 
			void * newBuffer = realloc(netAnnounce.data, newLength);
			if ( newBuffer == NULL) {
				fprintf(stderr, "Out of memory while trying to enlarge recieve buffer from %u to %u bytes!\n",
						netAnnounce.versionsLength, newLength);
				return;
			}
			netAnnounce.data = (char *)newBuffer;
			netAnnounce.length = newLength;
			netAnnounce.versions = (struct Version *)(netAnnounce.data + sizeof(struct AnnounceHeader));
			netAnnounce.versionsLength = sizeof(struct Version) * netVersionsCount;
			netAnnounce.announceHeader = (struct AnnounceHeader *)netAnnounce.data;
		}
	}

	/* now read the whole message again including header */
	ebytes = sizeof(struct AnnounceHeader) + sizeof(struct Version)*netAnnounce.announceHeader->versionsCount;
	/* We assume that the whole announce message fits in one UDP packet.
	 * This should be enough to recieve at least hundred versions.
	 */
	if ((nbytes = recvfrom(fd, netAnnounce.data, ebytes, 0, (struct sockaddr *) &addr, &addrlen)) < 0) {
		perror("recvfrom");
		return;
	}
	if (nbytes != ebytes) {
		fprintf(stderr, "incomplete packet recieved from %s:\n", inet_ntoa(addr.sin_addr));
		return;
	}

	/* Check signature */
	{
		struct Config * distoreConfig = getConfig();
		sha1sumInit();
		sha1sumUpdate((const char *)netAnnounce.versions, sizeof(struct Version)*netAnnounce.announceHeader->versionsCount);
		sha1sumUpdate((const char *)distoreConfig->secret, strlen(distoreConfig->secret));
		sha1sumFinal(dhash);
		if ( memcmp(netAnnounce.announceHeader->hash, dhash, SHA1_LENGTH) != 0) {
			fprintf(stderr, "!!! Achtung !!! <- Got announce with bad signature!\n");
			return;
		}
	}

	dmesg(DBG_DEBUG, "Got announce from %s:\n", inet_ntoa(addr.sin_addr));
#ifdef DEBUG
	printAnnounce(netAnnounce.announceHeader, netAnnounce.versions);
#endif

	/* update availableVersions */

	if (availableVersions == NULL) {
		availableVersions = ght_create(netAnnounce.announceHeader->versionsCount);
        if (availableVersions == NULL) {
            fprintf(stderr, "Failed to create availableVersions hash table!\n");
            return;
        }
        ght_set_rehash(availableVersions, 1);
	}

	{
		int i = 0;
		struct NetVersion * oldVersion = NULL;
		struct NetVersion * netVersion = NULL;
		for (i=0; i < netAnnounce.announceHeader->versionsCount; i++) {

			oldVersion = (struct NetVersion *) ght_get(availableVersions, sizeof(int), (void *)&(netAnnounce.versions[i].id));
			if (oldVersion == NULL) {
				netVersion = allocateNetVersion(netAnnounce.versions[i].version, &addr);
				if (netVersion == NULL) {
					fprintf(stderr, "Out of memory! Buy!\n");
					return;
				}
				if (ght_insert(availableVersions, (void *)netVersion, sizeof(int), (void *)&(netAnnounce.versions[i].id)) < 0) {
					fprintf(stderr, "Failed to insert NetVersion to availableVersions hash table!\n");
					free(netVersion);
					return;
				}
			} else {
				if (oldVersion->version < netAnnounce.versions[i].version) {
					netVersion = allocateNetVersion(netAnnounce.versions[i].version, &addr);
					if (netVersion == NULL) {
						fprintf(stderr, "Out of memory! Buy!\n");
						return;
					}
					if (ght_replace(availableVersions, (void *)netVersion, sizeof(int), (void *)&(netAnnounce.versions[i].id)) < 0) {
						fprintf(stderr, "Failed to replace NetVersion in availableVersions hash table!\n");
						free(netVersion);
						return;
					}
					free(oldVersion);
				}
			}
		}
	}
}

void sendAnnounce(int fd) {
	struct sockaddr_in addr;
	unsigned int addrlen = sizeof(struct sockaddr_in);
	struct Config * distoreConfig = getConfig();
	char * uniTarget = NULL;

	/* set up destination address */
	memset(&addr, 0, addrlen);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(distoreConfig->listenPort);
	if ( inet_aton(distoreConfig->multicastGroup, &(addr.sin_addr)) == 0) {
		fprintf(stderr, "Bad IP address: %s\n", distoreConfig->multicastGroup);
	} else {
		if (sendto(fd, currentAnnounce.data, currentAnnounce.length, 0 ,(struct sockaddr *) &addr, addrlen) < 0) {
		   perror("sendto");
		}
		dmesg(DBG_DEBUG, "Sent announce to %s:\n", inet_ntoa(addr.sin_addr));
	}

	/* And now - unicast targets */
	uniTarget=distoreConfig->unicastTargets;
	while ( (uniTarget != NULL) && (*uniTarget != '\0') ) {
		/* "+1" - for ' ' separator between addresses */
		if ( inet_aton(uniTarget, &(addr.sin_addr)) == 0) {
			fprintf(stderr, "Bad IP address: %s\n", uniTarget);
		} else {
			if (sendto(fd, currentAnnounce.data, currentAnnounce.length, 0 ,(struct sockaddr *) &addr, addrlen) < 0) {
			   perror("sendto");
			}
			dmesg(DBG_DEBUG, "Additionally sent unicast announce to %s:\n", inet_ntoa(addr.sin_addr));
		}
		uniTarget = strchr(uniTarget, ' ');
		if (uniTarget == NULL) {
			break;
		} else {
			uniTarget++;
		}
	}
}

