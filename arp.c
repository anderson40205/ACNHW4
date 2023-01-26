#include "arp.h"
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <linux/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
struct arp_packet
{
	struct ether_header eth_hdr;//size = 14byte, 在linux drivers/staging/rtl8192u/ieee80211/ieee80211.h
	struct ether_arp arp;//size = 28
};
*/
/*
	ether_header結構
	uint8_t ether_dhost[6];
	uint8_t ether_shost[6];
	uint16_t ether_type;
*/
/*
	ether_arp結構
	struct arphdr ea_hdr;
	uint8_t arp_sha [6];
	uint8_t arp_spa [4];
	uint8_t arp_tha [6];
	uint8_t arp_tpa [4];
*/
/*
	arphdr結構
	uint16_t ar_hrd;
	uint16_t ar_pro;
	uint16_t ar_hin;
	uint16_t ar_pin;
	uint16_t ar_op;
*/
//You can fill the following functions or add other functions if needed. If not, you needn't write anything in them.  

void set_all_attribute_type(struct arp_packet *packet)
{
	packet->eth_hdr.ether_type = htons(0x0806);//Type:ARP
	packet->arp.ea_hdr.ar_hrd = htons(1);//Hardware type: Ethernet (1)
	packet->arp.ea_hdr.ar_pro = htons(0x0800);// Protocol type: IPv4 (0x0800)
	packet->arp.ea_hdr.ar_hln = 6;// Hardware size: 6
	packet->arp.ea_hdr.ar_pln = 4;// Protocol size: 4
	packet->arp.ea_hdr.ar_op = htons(ARPOP_REQUEST);//Request: Who has xxx, Tell me 
	//packet->arp.ea_hdr.ar_op = htons(ARPOP_REPLY);//Request: Who has xxx, Tell me 
}
void set_fake_hardware_addr(struct arp_packet *packet, char *srcMAC, char *dstMAC)
{
	//printf("dstMAC = %s\n",dstMAC);
	char *p = strtok(srcMAC,":");
	int i=0;
	for(i=0;i<6;i++)
	{
		int num = (int)strtol(p, NULL, 16);   
		//printf("%d\n",num);
		packet->arp.arp_sha[i] = (uint8_t)num;
		packet->eth_hdr.ether_shost[i] = (uint8_t)num;
		p = strtok (NULL, ":");
	}
	
	p = strtok(dstMAC,":");
	for(i=0;i<6;i++)
	{
		int num = (int)strtol(p, NULL, 16);   
		packet->arp.arp_tha[i] = (uint8_t)num;//廣播，0不能廣播
		packet->eth_hdr.ether_dhost[i] = (uint8_t)num;//廣播，0不能廣播
		p = strtok (NULL, ":");
	}
}
void set_sender_hardware_addr(struct arp_packet *packet, char *address)
{
	//printf("srcMAC = %s\n",address);
	char *p = strtok(address,":");
	int i=0;
	for(i=0;i<6;i++)
	{
		int num = (int)strtol(p, NULL, 16);   
		//printf("%d\n",num);
		packet->arp.arp_sha[i] = (uint8_t)num;
		packet->arp.arp_tha[i] = 0xFF;//廣播，0不能廣播
		packet->eth_hdr.ether_shost[i] = (uint8_t)num;
		packet->eth_hdr.ether_dhost[i] = 0xFF;//廣播，0不能廣播
		p = strtok (NULL, ":");
	}
	//packet->arp.arp_sha[0] = 0xF8;packet->arp.arp_sha[1] = 0x1D;packet->arp.arp_sha[2] = 0x0F;packet->arp.arp_sha[3] = 0x18;packet->arp.arp_sha[4] = 0x2B;packet->arp.arp_sha[5] = 0xF2;
	//packet->eth_hdr.ether_shost[0] = 0xF8;packet->eth_hdr.ether_shost[1] = 0x1D;packet->eth_hdr.ether_shost[2] = 0x0F;packet->eth_hdr.ether_shost[3] = 0x18;packet->eth_hdr.ether_shost[4] = 0x2B;packet->eth_hdr.ether_shost[5] = 0xF2;
}
void set_sender_protocol_addr(struct ether_arp *packet, char *address)
{
	int i,arr[4];
	char *ipPointer = strtok(address,".");
	for(i=0;i<4;i++)
	{
		if(ipPointer == NULL)
		{
			arr[i]=0;
			break;
		}
		else
		{
			arr[i] = atoi(ipPointer);
			packet->arp_spa[i] = arr[i];
		}
		ipPointer = strtok(NULL,".");
	}
}

void set_target_protocol_addr(struct ether_arp *packet, char *address)
{
	int i,arr[4];
	char *ipPointer = strtok(address,".");
	for(i=0;i<4;i++)
	{
		if(ipPointer == NULL)
		{
			arr[i]=0;
			break;
		}
		else
		{
			arr[i] = atoi(ipPointer);
			packet->arp_tpa[i] = arr[i];
		}
		ipPointer = strtok(NULL,".");
	}
}
