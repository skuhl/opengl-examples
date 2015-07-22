// This program is used by DGR applications. This program simply reads
// the UDP packets that DGR generates and then sends them on to a new
// IP address.

// Authors:
// James Walker   jwwalker at mtu dot edu
// Scott A. Kuhl  kuhl at mtu dot edu

#ifndef __MINGW32__
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <string>
#include <vector>


/* On most networks, the MTU is set to 1500 bytes. With header
 * overhead, this means that we could expect to have around 1472 bytes
 * of data in a UDP packet. Local loopback addresses will often have a
 * higher MTU. One way to send packets larger than the MTU is with
 * IPv4 fragmentation---which happens automatically. */
#define BUFLEN 65536

char *RELAY_IN_PORT = NULL; // the port we listen for UDP packets on
char *RELAY_OUT_IP = NULL; // the address we send UDP packets to

pthread_t receiverThread;

int s_R; // socket we will read packets from
socklen_t slen_R;


struct sockaddr_in si_me_R;

std::vector<int> s_S; // list of sockets to send data to
std::vector<struct sockaddr_in> si_other_S; // list of sockaddr objects for our sockets.
socklen_t slen_S;

bool receivedPacket = false;
int framesPassed = 0;

// This function receives incoming packets, repackages them, and then forwards them
// on the network for consumption by the slaves. It does this in an infinite loop.
void * receiver(void *) {
	char buf[BUFLEN];
	while (true) {
		// Receive any frames
		int bytesReceived;
		bytesReceived = recvfrom(s_R, buf, BUFLEN, 0, NULL, 0);
		if (bytesReceived == -1) {
			perror("DGR Relay: ERROR recvfrom");
			exit(EXIT_FAILURE);
		}

		receivedPacket = true;
		framesPassed = 0;

		// When we have received a frame, send it out!
		for(unsigned int i = 0; i < s_S.size(); i++){
			if (sendto(s_S[i], buf, bytesReceived, 0, (struct sockaddr*)&si_other_S[i],
					   slen_S) == -1) {
				perror("DGR Relay: ERROR sendto");
				exit(EXIT_FAILURE);
			}
		}

		/* Check if the frame that we just forwarded was informing
		 * processes to exit. */
		if(bytesReceived > (int) strlen("!!!dgr_died!!!") &&
		   strcmp(buf, "!!!dgr_died!!!") == 0)
		{
			printf("DGR Relay: Received message from master indicating that DGR communication is complete.\n");
			exit(EXIT_SUCCESS);
		}
	}
}
#endif // __MINGW32__


int main(int argc, char **argv) {
#ifndef __MINGW32__
	if (argc < 4) {
		printf("USAGE: %s port-in ipaddr-out port-out [ port2-out .. ]\n", argv[0]);
		printf("This program will listen on a specific port for UDP packets. When one is received, it will be sent to the specified IP address. If more than one port is specified, it will send the packet to multiple ports at that IP address.\n");
		exit(EXIT_FAILURE);
	}
	RELAY_IN_PORT=argv[1];
	RELAY_OUT_IP=argv[2];

	// for each of the output ports, create a socket:
	for(int i = 3; i < argc; i++){
		printf("DGR Relay: Preparing to send data to %s on port %s\n", RELAY_OUT_IP, argv[i]);
		
		struct sockaddr_in _si_other_S;
		int _s_S;
		slen_S=sizeof(_si_other_S);

		if ((_s_S=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
			perror("DGR Relay: ERROR socket");
			exit(EXIT_FAILURE);
		}
		int so_broadcast = 1;
		setsockopt(_s_S, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof(so_broadcast));

		memset((char *) &_si_other_S, 0, sizeof(_si_other_S));
		_si_other_S.sin_family = AF_INET;
		_si_other_S.sin_port = htons(atoi(argv[i]));
		if (inet_aton(RELAY_OUT_IP, &_si_other_S.sin_addr) == 0) {
			fprintf(stderr, "DGR Relay: inet_aton() failed\n");
			exit(1);
		}

		// add this socket to a list
		s_S.push_back(_s_S);
		si_other_S.push_back(_si_other_S);
    }


	printf("DGR Relay: Preparing to receive data on port %s\n", RELAY_IN_PORT);
	// Create and bind the socket that we will use to receive data from.
	if ((s_R=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("DGR Relay: ERROR socket");
		exit(EXIT_FAILURE);
	}
	memset((char *) &si_me_R, 0, sizeof(si_me_R));
	si_me_R.sin_family = AF_INET;
	si_me_R.sin_port = htons(atoi(RELAY_IN_PORT));
	si_me_R.sin_addr.s_addr = htonl(INADDR_ANY);
	int _bind_result = bind(s_R, (struct sockaddr*)&si_me_R, sizeof(si_me_R));
	if ( _bind_result == -1) {
		perror("DGR Relay: ERROR bind");
		exit(EXIT_FAILURE);
	}

	// listen for updates
	if (pthread_create(&receiverThread, NULL, &receiver, NULL) != 0) {
		perror("DGR Relay: Exiting because pthread_create() failed.");
		exit(EXIT_FAILURE);
	}

	printf("DGR Relay: Initialization complete, running...\n");

	while (true) {
	        usleep(100000); // 1/10th a second

		// The relay automatically shuts itself off if it hasn't received any packets
		// within a certain time period if it has already received a packet,
		// >15 seconds if it hasn't received any packets yet).
		framesPassed++;

		int timeoutReceivedPacket = 50; // tenths of a second to timeout (if we HAVE received previous packet)
		int timeoutFirstPacket = 150; // tenths of a second to timeout (if we have NOT received previous packet)
		
		if(receivedPacket && framesPassed > timeoutReceivedPacket) {
		printf("DGR Relay: Exiting because we haven't received a packet within %f seconds (and we have received packets previously).\n", timeoutReceivedPacket/10.0);
				exit(EXIT_SUCCESS);
		}
		if(receivedPacket == 0 && framesPassed > timeoutFirstPacket) {
		printf("DGR Relay: Exiting because we never received any packets within %f seconds.\n", timeoutFirstPacket/10.0);
			exit(EXIT_SUCCESS);
		}
	}
#endif  // __MINGW32__
}
