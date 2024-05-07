/**
 * NOTE: This program is to show case the functionality of the Netlink sockets 
 * to interact with the Link status of the interfaces.
 * Author: G. Naveen Kumar
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>

#define BUFFER_SIZE 8192

#define print_mac(x) printf("%02x:%02x:%02x:%02x:%02x:%02x", x[0], x[1], x[2], x[3], x[4], x[5])

int main(int argc, char **argv){
	int sockfd = 0 ;
	char buffer[BUFFER_SIZE];
	struct sockaddr_nl addr;
	struct nlmsghdr *nlh;
	struct ifinfomsg *ifm;
	struct rtattr *rta;
	int len = 0, rta_len = 0;
	unsigned char mac[6] = {0};
	struct rtnl_link_stats link_stats;
	int index = 1;

	sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if(sockfd < 0){
		printf("Failed to create socket fd\n");
		return -1;
	}

	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();

	if(bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_nl)) < 0){
		printf("Failed to bind the netlink sockaddr\n");
		close(sockfd);
		return -1;
	}

	nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(BUFFER_SIZE));
	if(!nlh){
		printf("Failed to allocate memory for struct nlmsghdr\n");
		close(sockfd);
		return -1;
	}

	nlh->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	nlh->nlmsg_type = RTM_GETLINK;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	nlh->nlmsg_seq = time(NULL);

	if(send(sockfd, nlh, nlh->nlmsg_len, 0) < 0){
		printf("Sending via netlink failed\n");
		close(sockfd);
		return -1;
	}
	printf("Send success\n");

	while((len = recv(sockfd, buffer, BUFFER_SIZE, MSG_DONTWAIT)) > 0){
		for(nlh = (struct nlmsghdr*)buffer; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh,len)){
			if(nlh->nlmsg_type == NLMSG_DONE){
				break;
			}
			ifm = (struct ifinfomsg*)NLMSG_DATA(nlh);
			char ifname[32] = {0};
			if_indextoname(ifm->ifi_index, ifname);
			printf("%d: Interface: %s\n", index++, ifname);
			rta = (struct rtattr*)IFLA_RTA(ifm);
			rta_len = RTM_PAYLOAD(nlh);
			for( ; RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)){
				switch(rta->rta_type){
					case IFLA_ADDRESS:
						memset(mac, 0, sizeof(mac));
						memcpy(mac, RTA_DATA(rta), 6);
						printf(" mac: ");
						print_mac(mac);
						break;

					case IFLA_BROADCAST:
						memset(mac, 0, sizeof(mac));
						memcpy(mac, RTA_DATA(rta), 6);
						printf(" Broadcast: ");
						print_mac(mac);
						break;
					case IFLA_MTU: 
						printf(" MTU: %d ", *(int*)(RTA_DATA(rta)));
						break;
					case IFLA_LINKMODE:
						if(*(int*)RTA_DATA(rta) == 1)
							printf("\tState: UP ");
						else
							printf("\tState: DOWN ");
						break;
					case IFLA_QDISC:
						printf("QDisc: %s", (char*)RTA_DATA(rta));
						break;
					case IFLA_STATS:
						memset(&link_stats, 0, sizeof(struct rtnl_link_stats));
						memcpy(&link_stats, (struct rtnl_link_stats*)RTA_DATA(rta), sizeof(struct rtnl_link_stats));
						printf("\n\tRx: Packets %u, bytes %u", link_stats.rx_packets, link_stats.rx_bytes);
						printf("\n\tTx: Packets %u, bytes %u", link_stats.tx_packets, link_stats.tx_bytes);
						printf("\n\tErrors: Rx %u, Tx %u", link_stats.rx_errors, link_stats.tx_errors);

						break;
					default:
						break;
				}
			}
			printf("\n\n");
		}
	} 

	printf("Socket close success\n");
	close(sockfd);
	return 0;
}
