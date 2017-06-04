#ifndef __k66_ethernet_h__
#define __k66_ethernet_h__

#ifndef __MK66FX1M0__
  #error must be Teensy 3.6
#endif

#include <Arduino.h>
#include "IPAddress.h"
#include "struct.h"

#define RXSIZE 12
#define TXSIZE 10

#define IPMACMAPSIZE 100


class k66ethernet {
private:
  uint8_t _macAddr[6];
  uint8_t _ipAddr[4];
  static uint8_t _currentTxBuffer = 0;
  static uint8_t _currentRxBuffer = 0;
  int _ipMacMapLocation = 0;

  int _watingForArp = 0;
  //uint8_t _packetLen = 0;
  /* static int interruptFlag;
  // set this to an unused IP number for your network
  // static IPAddress myaddress;
  static uint8_t myaddress_byte[4];
  IPAddress subNet(255, 255, 255, 0);
  IPAddress prefix(0, 0, 0, 0);
  static int rxnum;
  static int txnum;
  static const uint8_t MACADDR[6];
*/
  enetbufferdesc_t _rxRing[RXSIZE] __attribute__ ((aligned(16)));
  enetbufferdesc_t _txRing[TXSIZE] __attribute__ ((aligned(16)));
  uint32_t _rxBuffer[RXSIZE*128] __attribute__ ((aligned(16)));
  uint32_t _txBuffer[TXSIZE*128] __attribute__ ((aligned(16)));

  uint8_t _ipMap[IPMACMAPSIZE][4];
  uint8_t _macMap[IPMACMAPSIZE][6];

public:
  k66ethernet();
  k66ethernet(uint8_t *mac);
  void begin();
  // int begin(uint8_t *dhcpIP);
  int beginEthernetPacket(uint8_t *mac, EtherType type);
  int beginIPv4Packet(uint8_t *ip, uint8_t protocol, uint8_t DSCP = 0);
  int sendArp(uint8_t *trgIP, uint8_t *trgMAC, uint16_t operation, bool awaitRespons = true);
  void endPacket();

  void setIP(uint8_t *ip);
  //void rxIRQ();
  /*static void outgoing(void *packet, unsigned int len);
  static void outgoing_pre(void *packet, unsigned int len, const uint8_t* dstMAC, uint16_t EtherType);
  // static void ping_reply(const uint32_t *recv, unsigned int len);
  static void arp_reply(const uint8_t *mac, const uint8_t *ip);
  static uint16_t mdio_read(int phyaddr, int regaddr);
  static void mdio_write(int phyaddr, int regaddr, uint16_t data);
  // static void incoming(void *packet, unsigned int len, uint16_t flags);
  */
};


#endif
