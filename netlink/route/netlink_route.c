/**
 * NOTE: This program is to show case the functionality of the Netlink sockets 
 * to interact with the routing table.
 * Author: G. Naveen Kumar
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#define BUFFER_SIZE 8192

int main(int argc, char **argv){
	int sockfd = 0;
	struct sockaddr_nl saddr;
	struct nlmsghdr *nlh;
	struct rtmsg *rtm;
	struct rtattr *rta;
	int len = 0, rta_len = 0;
	char buffer[BUFFER_SIZE];
	char ifname[32] = {0};


	// Open the socket with AF_NETLINK and NETLINK_ROUTE
	sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if(sockfd < 0){
		printf("Failed to create Netlink socket\n");
		return -1;
	}
	printf("Netlink socket FD: %d\n", sockfd);
	
	memset(&saddr, 0, sizeof(struct sockaddr_nl));
	saddr.nl_family = AF_NETLINK;
	saddr.nl_pid = getpid();

	// Bind the netlink socket
	if(bind(sockfd, (struct sockaddr*)&saddr, sizeof(struct sockaddr_nl)) < 0){
		printf("Failed to bind the socket\n");
		close(sockfd);
		return -1;
	}
	printf("Bind success\n");

	// Prepare the Netlink Message Header - struct nlmsghdr
	nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(BUFFER_SIZE));
	nlh->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	nlh->nlmsg_type = RTM_GETROUTE;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	nlh->nlmsg_seq = time(NULL);

	// Send the Netlink Communication
	if(send(sockfd, nlh, nlh->nlmsg_len, 0) < 0){
		printf("Failed to send data via netlink socket\n");
		close(sockfd);
		return -1;
	}


	// Retreive the response from the Netlink Socket
	do {
		len = recv(sockfd, buffer, BUFFER_SIZE, MSG_DONTWAIT);
		if(len <= 0){
			printf("End Messages\n");
			break;
		}

		// Loop throught the Netlink Message Header and get the data
		for(nlh = (struct nlmsghdr*)buffer; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)){
			// Retrieve the Data by pointing to the appropriate type
			// In this case its struct rtmsg
			rtm = (struct rtmsg*)NLMSG_DATA(nlh);
			if(rtm->rtm_table != RT_TABLE_MAIN){
				continue;
			}

			// Retreive the routing data
			rta = (struct rtattr*)RTM_RTA(rtm);
			rta_len = RTM_PAYLOAD(nlh);

			for(; RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)){
				switch(rta->rta_type){
					case RTA_DST:
						printf("Route - %s ", inet_ntoa(*(struct in_addr*)(RTA_DATA(rta))));
						break;
					case RTA_GATEWAY:
						printf("Gateway(Default) via %s ", inet_ntoa(*(struct in_addr*)(RTA_DATA(rta))));
						break;
					case RTA_OIF:
						memset(ifname, 0, sizeof(ifname));
						if_indextoname(*(int*)RTA_DATA(rta), ifname);
						printf("dev %s ", ifname);
						break;
					default:
						break;
				}
			}
			printf("\n");
		}
	} while (nlh->nlmsg_type != NLMSG_DONE);

	// Close the socket 
	close(sockfd);
	printf("Closed socket\n");
	return 0;
}
