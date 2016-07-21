/*
 * etharp.c
 *
 *  Created on: 2016-7-19
 *      Author: Administrator
 *      ʵ��arpЭ�飬�������漰ipЭ�鲿�֡�
 */

#include "etharp.h"
#include "stats.h"
#include "debug.h"
#include "def.h"
#include <string.h>
#include "net.h"
struct stats_ lwip_stats;

const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};
const struct eth_addr ethzero = {{0,0,0,0,0,0}};

/** the time an ARP entry stays valid after its last update,
 *  for ARP_TMR_INTERVAL = 5000, this is
 *  (240 * 5) seconds = 20 minutes.
 */
#define ARP_MAXAGE 240
/** the time an ARP entry stays pending after first request,
 *  for ARP_TMR_INTERVAL = 5000, this is
 *  (2 * 5) seconds = 10 seconds.
 *
 *  @internal Keep this number at least 2, otherwise it might
 *  run out instantly if the timeout occurs directly after a request.
 */
#define ARP_MAXPENDING 2

#define HWTYPE_ETHERNET 1

/**
 * Responds to ARP requests to us. Upon ARP replies to us, add entry to cache
 * send out queued IP packets. Updates cache with snooped address pairs.
 *
 * Should be called for incoming ARP packets. The pbuf in the argument
 * is freed by this function.
 *
 * @param netif The lwIP network interface on which the ARP packet pbuf arrived.
 * @param ethaddr Ethernet address of netif.
 * @param p The ARP packet that arrived on netif. Is freed by this function.
 *
 * @return NULL
 *
 * @see pbuf_free()
 */
static void
etharp_arp_input(struct netif *netif, struct eth_addr *ethaddr, struct pbuf *p)
{
	struct etharp_hdr *hdr;
	struct chitic_etharp_hdr *chitic_hdr;
	struct eth_hdr *ethhdr;

	LWIP_ERROR("netif != NULL", (netif != NULL), return;);


	/* drop short ARP packets: we have to check for p->len instead of p->tot_len here
	     since a struct etharp_hdr is pointed to p->payload, so it musn't be chained! */
	if (p->len < SIZEOF_ETHARP_PACKET) {
		LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING,
		  ("etharp_arp_input: packet dropped, too short (%"S16_F"/%"S16_F")\n", p->tot_len,
		  (s16_t)SIZEOF_ETHARP_PACKET));
		ETHARP_STATS_INC(etharp.lenerr);
		ETHARP_STATS_INC(etharp.drop);
		return;
	}

	ethhdr = (struct eth_hdr *)p->payload;
	hdr = (struct etharp_hdr *)((u8_t*)ethhdr + SIZEOF_ETH_HDR);

	 /* RFC 826 "Packet Reception": */
	if ((hdr->hwtype != PP_HTONS(HWTYPE_ETHERNET)) ||
	  (hdr->hwlen != ETHARP_HWADDR_LEN)
	  )  {
		LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING,
		  ("etharp_arp_input: packet dropped, wrong hw type, hwlen, proto, protolen or ethernet type (%"U16_F"/%"U16_F"/%"U16_F"/%"U16_F")\n",
		  hdr->hwtype, hdr->hwlen, hdr->proto, hdr->protolen));
		ETHARP_STATS_INC(etharp.proterr);
		ETHARP_STATS_INC(etharp.drop);
		return;
	}
	ETHARP_STATS_INC(etharp.recv);

	//�Զ����Э�顣���������ģ��ԱȽ��շ�MAC�뱾��MAC��һ�½����ͷ��ͽ��շ��Ի�λ�ã�Ȼ��Ӧ��
	//����Ӧ���ģ��ԱȽ��շ�MAC�뱾��MAC�������ͷ�MAC����
	//���ģ���ַ�ǹ㲥��ַ��Ҳ��Ϊ�뱾��ƥ��
	if( hdr->proto == PP_HTONS(ETHTYPE_CHITIC))
	{
		chitic_hdr = (struct chitic_etharp_hdr *)((u8_t*)ethhdr + SIZEOF_ETH_HDR);
		printf( "chitic_hdr->opcode:%x \n", htons( chitic_hdr->opcode));
		switch (chitic_hdr->opcode)
		{
			case PP_HTONS(ARP_REQUEST):
				if( ( macCompAddr( &chitic_hdr->dhwaddr, (void *)netif->hwaddr) == 0) || \
					( macCompAddr( &chitic_hdr->dhwaddr, &ethbroadcast) == 0))
				{
					ETHADDR16_COPY(&chitic_hdr->dhwaddr, &chitic_hdr->shwaddr);
					ETHADDR16_COPY(&chitic_hdr->shwaddr, netif->hwaddr);
					chitic_hdr->opcode = htons(ARP_REPLY);
					/* return ARP reply */
					//TODO	����������������
					netif->linkoutput(netif, p);
				}
				break;
			case PP_HTONS(ARP_REPLY):
				if( macCompAddr( &chitic_hdr->dhwaddr, netif->hwaddr) == 0)
				{
					int i = 0;
					for( i = 0 ; i < CONNECT_INFO_NUM; i++)
					{
						if( Eth_Cnnect_info[i].status == CON_STATUS_PENDING)
						{
//							if( macCompAddr( Eth_Cnnect_info[i].target_hwaddr, &chitic_hdr->shwaddr) == 0)
							if( ( macCompAddr( Eth_Cnnect_info[i].target_hwaddr, &chitic_hdr->shwaddr) == 0) || \
								( macCompAddr( Eth_Cnnect_info[i].target_hwaddr, &ethbroadcast)== 0) )
							{
								ETHADDR16_COPY( Eth_Cnnect_info[i].target_hwaddr, &chitic_hdr->shwaddr);
								Eth_Cnnect_info[i].status = CON_STATUS_ESTABLISH;
								break;
							}
						}
					}
				}

				break;
			 default:
			    LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("etharp_arp_input: ARP unknown opcode type %"S16_F"\n", htons(hdr->opcode)));
			    ETHARP_STATS_INC(etharp.err);
			    break;

		}
	}
}

err_t
chitic_arp_output(struct netif *netif, struct pbuf *q, struct eth_addr *target_hwaddr)
{
  struct chitic_etharp_hdr *chitic_hdr;
  struct eth_hdr *ethhdr ;



  ethhdr = (struct eth_hdr *)q->payload;
  chitic_hdr = (struct chitic_etharp_hdr *)((u8_t*)ethhdr + SIZEOF_ETH_HDR);

  ETHADDR16_COPY(&ethhdr->dest, &ethbroadcast);
  ETHADDR16_COPY(&ethhdr->src, netif->hwaddr);
  ethhdr->type = htons(ETHTYPE_ARP);

  chitic_hdr->hwtype = htons(HWTYPE_ETHERNET);
  chitic_hdr->proto = htons(ETHTYPE_CHITIC);
  chitic_hdr->hwlen = ETHARP_HWADDR_LEN;
  chitic_hdr->protolen = 0;
  chitic_hdr->opcode = htons(ARP_REQUEST);
  ETHADDR16_COPY(&chitic_hdr->shwaddr, netif->hwaddr);
  ETHADDR16_COPY(&chitic_hdr->dhwaddr, target_hwaddr);

  q->len = 34;
  /* continuation for multicast/broadcast destinations */
  /* obtain source Ethernet address of the given interface */
  /* send packet directly on the link */
  return netif->linkoutput( netif, q);
}


/*
 *
 * �������ܵ�����
 *
 */
err_t ethernet_input(struct pbuf *p, struct netif *netif)
{
	struct eth_hdr* ethhdr;
	u16_t type;

	ethhdr = (struct eth_hdr *)p->payload;
	 if (p->len <= SIZEOF_ETH_HDR) {
	    /* a packet with only an ethernet header (or less) is not valid for us */
	    ETHARP_STATS_INC(etharp.proterr);
	    ETHARP_STATS_INC(etharp.drop);
	    goto free_and_return;
	  }



	 printf("ethernet_input: dest:%02x:%02x:%02x:%02x:%02x:%02x, src:%02x:%02x:%02x:%02x:%02x:%02x, type:%x\n",
	     (unsigned)ethhdr->dest.addr[0], (unsigned)ethhdr->dest.addr[1], (unsigned)ethhdr->dest.addr[2],
	     (unsigned)ethhdr->dest.addr[3], (unsigned)ethhdr->dest.addr[4], (unsigned)ethhdr->dest.addr[5],
	     (unsigned)ethhdr->src.addr[0], (unsigned)ethhdr->src.addr[1], (unsigned)ethhdr->src.addr[2],
	     (unsigned)ethhdr->src.addr[3], (unsigned)ethhdr->src.addr[4], (unsigned)ethhdr->src.addr[5],
	     (unsigned)htons(ethhdr->type));

	  type = ethhdr->type;


	  switch( type)
	  {
		case PP_HTONS(ETHTYPE_ARP):
			if (!(netif->flags & NETIF_FLAG_ETHARP)) {
			  goto free_and_return;
			}
			/* pass p to ARP module */
			etharp_arp_input(netif, (struct eth_addr*)(netif->hwaddr), p);
			break;
		case PP_HTONS(ETHTYPE_CHITIC):
			if( netif->input)
				return netif->input( p, netif);
			break;
	  }

free_and_return:
//	  pbuf_free(p);
	  return ERR_OK;
}