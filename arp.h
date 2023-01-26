#ifndef __ARP_UTIL_H__
#define __ARP_UTIL_H__

#include <netinet/if_ether.h>

struct arp_packet
{
	struct ether_header eth_hdr;//size = 14byte, 在linux drivers/staging/rtl8192u/ieee80211/ieee80211.h
	struct ether_arp arp;//size = 28
};


void set_all_attribute_type(struct arp_packet *packet);
void set_fake_hardware_addr(struct arp_packet *packet, char *srcMAC, char *dstMAC);
//void set_sender_hardware_addr(struct ether_arp *packet, char *address);
//MAC address有兩個struct都有，所以我改成arp_packet來修改那兩個struct
void set_sender_hardware_addr(struct arp_packet *packet, char *address);
void set_sender_protocol_addr(struct ether_arp *packet, char *address);
void set_target_protocol_addr(struct ether_arp *packet, char *address);

#endif
