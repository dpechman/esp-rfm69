#include "c_types.h"
#include "c_stdio.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"

//Listening connection data
static struct espconn dnsConn;
static esp_udp dnsUdp;

static const char *localDomains[]={
    INTERFACE_DOMAIN, //because I can be anybody

    //captive portal domains
    "google.com", //android , yes I can be Google too
    "gsp1.apple.com", //iphone
    "akamaitechnologies.com",
    "apple.com",
    "appleiphonecell.com",
    "itools.info",
    "ibook.info",
    "airport.us",
    "thinkdifferent.us",
    "apple.com.edgekey.net",
    "akamaiedge.net",
    "msftncsi.com", //for windows and windows phone,
    "microsoft.com",
    NULL
};

static int ICACHE_FLASH_ATTR isKnownDNS(char *dns)
{
   int i=0;
   while(true)
   {
      const char *cmp = localDomains[i];

      if(cmp==NULL) { break; }

      if(strstr(dns,cmp)) { return 1; }

      i++;
   }

   return 0;
}

static void ICACHE_FLASH_ATTR dnsQueryReceived(void *arg, char *data, unsigned short length)
{
   // parse incoming query domain
   char domain[30];
   char *writePos=domain;
   memset(domain,0,30);

   int offSet=12;
   int len=data[offSet];
   while(len!=0 && offSet<length)
   {
      offSet++;
      memcpy(writePos,data+offSet,len);
      writePos+=len; //advance
      offSet+=len;
      len=data[offSet];

      if(len!=0) { *writePos='.'; writePos++; }
   }

   DNS_DBG("DNS Query Received: %s",domain);
   if(!isKnownDNS(domain))
   return;

   struct espconn *conn=arg;

   remot_info *premot = NULL;

   if (espconn_get_connection_info(conn,&premot,0) == ESPCONN_OK)
   {
      os_memcpy(conn->proto.udp->remote_ip, premot->remote_ip, 4);
      conn->proto.udp->remote_port = premot->remote_port;

      uint8_t *ip1 = conn->proto.udp->remote_ip;
      NODE_DBG("UDP:\nremote_ip: %d.%d.%d.%d remote_port: %d\n",ip4_addr1_16(ip1),ip4_addr2_16(ip1), ip4_addr3_16(ip1), ip4_addr4_16(ip1), premot->remote_port);
   }
   else
      { NODE_ERR("UDP: connection_info == NULL?\n"); }

   //build response
   char response[100] = {data[0], data[1],
                        0b10000100 | (0b00000001 & data[2]), //response, authorative answer, not truncated, copy the recursion bit
                        0b00000000, //no recursion available, no errors
                        data[4], data[5], //Question count
                        data[4], data[5], //answer count
                        0x00, 0x00,       //NS record count
                        0x00, 0x00};      //Resource record count

   int idx = 12;
   memcpy(response+12, data+12, length-12); //Copy the rest of the query section
   idx += length-12;

   //Set a pointer to the domain name in the question section
   response[idx] = 0xC0;
   response[idx+1] = 0x0C;

   //Set the type to "Host Address"
   response[idx+2] = 0x00;
   response[idx+3] = 0x01;

   //Set the response class to IN
   response[idx+4] = 0x00;
   response[idx+5] = 0x01;

   //A 32 bit integer specifying TTL in seconds, 0 means no caching
   response[idx+6] = 0x00;
   response[idx+7] = 0x00;
   response[idx+8] = 0x00;
   response[idx+9] = 0x00;

   //RDATA length
   response[idx+10] = 0x00;
   response[idx+11] = 0x04; //4 byte IP address

   //The IP address
   response[idx + 12] = 192;
   response[idx + 13] = 168;
   response[idx + 14] = 4;
   response[idx + 15] = 1;

   int ret = espconn_send(conn, (uint8_t*)response, idx+16);

   uint8_t *ip = conn->proto.udp->local_ip;
   NODE_DBG("local_ip: %d.%d.%d.%d local_port: %d\n",ip4_addr1_16(ip),ip4_addr2_16(ip), ip4_addr3_16(ip), ip4_addr4_16(ip),conn->proto.udp->local_port);
   NODE_DBG("DNS reply sent\n");
}

void ICACHE_FLASH_ATTR init_dns()
{
   //set softAP DHCP server router info
   uint8_t mode = 1;
   wifi_softap_set_dhcps_offer_option(OFFER_ROUTER, &mode);

   //1:station; 2:soft-AP, 3:station+soft-AP
   wifi_set_broadcast_if(3);
   //  espconn_disconnect(&dnsConn);
   espconn_delete(&dnsConn);

   dnsConn.type=ESPCONN_UDP;
   dnsConn.state=ESPCONN_NONE;
   dnsUdp.local_port= 53;
   dnsConn.proto.udp=&dnsUdp;
   espconn_regist_recvcb(&dnsConn, dnsQueryReceived);

   int res = espconn_create(&dnsConn);

   NODE_DBG("DNS server init, conn=%p , status=%d", &dnsConn,res);
}
