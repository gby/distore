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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

#include "sock.h"

int CreateDgramServerSock(int port, char *multicastGroup) {

	struct sockaddr_in addr;
	unsigned int addrlen = sizeof(struct sockaddr_in);
	int fd;
	struct ip_mreq mreq;
	unsigned int yes=1;

	/* create what looks like an ordinary UDP socket */
	if ((fd=socket(PF_INET,SOCK_DGRAM,0)) < 0) {
		perror("socket");
		return -1;
	}

	/* allow multiple sockets to use the same PORT number */
	if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
		perror("Reusing ADDR failed");
		return -1;
	}

	/* set up address */
	memset(&addr, 0, addrlen);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	/* bind to receive address */
	if (bind(fd, (struct sockaddr *) &addr, addrlen) < 0) {
		perror("bind");
		return -1;
	}

	if (multicastGroup != NULL) {
		/* use setsockopt() to request that the kernel join a multicast group */
		mreq.imr_multiaddr.s_addr = inet_addr(multicastGroup);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			perror("failed to join multicast group");
			return -1;
		}
	}

	return fd;
}

int CreateDgramClientSock() {

	int fd;

	/* create what looks like an ordinary UDP socket */
	if ((fd=socket(PF_INET,SOCK_DGRAM,0)) < 0) {
		perror("socket");
		return -1;
	}

	return fd;
}
