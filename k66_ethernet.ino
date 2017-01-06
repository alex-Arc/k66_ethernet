#include "IPAddress.h"
#include "EtherType.h"
#include "IPprotocol.h"
#include "struct.h"
//static const int IPv4 = 0x0008;     //byte swap Internet Protocol version 4
//static const int ARP = 0x0608;   //Address Resolution Protocol

// set this to an unused IP number for your network
IPAddress myaddress(2, 2, 6, 10);
uint8_t myaddress_byte[4] = {2,2,6,10};

enum EtherType: uint16_t {
  IPv4  = 0x0008,
  ARP   = 0x0608
};

#define MACADDR1 0x04E9E5
#define MACADDR2 0x000001

const uint8_t MACADDR[6] = {0x04,
                            0xE9,
                            0xE5,
                            0x00,
                            0x00,
                            0x01};

const uint8_t BoardCastMACADDR[6] =  {0xFF,
                                      0xFF,
                                      0xFF,
                                      0xFF,
                                      0xFF,
                                      0xFF};

// This test program prints a *lot* of info to the Arduino Serial Monitor
// Ping response time is approx 1.3 ms with 180 MHz clock, due to all the
// time spent printing.  To get a realistic idea of ping time, you would
// need to delete or comment out all the Serial print stuff.



#define RXSIZE 12
#define TXSIZE 10
static enetbufferdesc_t rx_ring[RXSIZE] __attribute__ ((aligned(16)));
static enetbufferdesc_t tx_ring[TXSIZE] __attribute__ ((aligned(16)));
uint32_t rxbufs[RXSIZE*128] __attribute__ ((aligned(16)));
uint32_t txbufs[TXSIZE*128] __attribute__ ((aligned(16)));

void begin() {
  MPU_RGDAAC0 |= 0x007C0000;
	SIM_SCGC2 |= SIM_SCGC2_ENET;
	CORE_PIN3_CONFIG =  PORT_PCR_MUX(4); // RXD1
	CORE_PIN4_CONFIG =  PORT_PCR_MUX(4); // RXD0
	CORE_PIN24_CONFIG = PORT_PCR_MUX(2); // REFCLK
	CORE_PIN25_CONFIG = PORT_PCR_MUX(4); // RXER
	CORE_PIN26_CONFIG = PORT_PCR_MUX(4); // RXDV
	CORE_PIN27_CONFIG = PORT_PCR_MUX(4); // TXEN
	CORE_PIN28_CONFIG = PORT_PCR_MUX(4); // TXD0
	CORE_PIN39_CONFIG = PORT_PCR_MUX(4); // TXD1
	CORE_PIN16_CONFIG = PORT_PCR_MUX(4); // MDIO
	CORE_PIN17_CONFIG = PORT_PCR_MUX(4); // MDC
	SIM_SOPT2 |= SIM_SOPT2_RMIISRC | SIM_SOPT2_TIMESRC(3);
	// ENET_EIR	1356	Interrupt Event Register
	// ENET_EIMR	1359	Interrupt Mask Register
	// ENET_RDAR	1362	Receive Descriptor Active Register
	// ENET_TDAR	1363	Transmit Descriptor Active Register
	// ENET_ECR	1363	Ethernet Control Register
	// ENET_RCR	1369	Receive Control Register
	// ENET_TCR	1372	Transmit Control Register
	// ENET_PALR/UR	1374	Physical Address
	// ENET_RDSR	1378	Receive Descriptor Ring Start
	// ENET_TDSR	1379	Transmit Buffer Descriptor Ring
	// ENET_MRBR	1380	Maximum Receive Buffer Size
	//		1457	receive buffer descriptor
	//		1461	transmit buffer descriptor

	print("enetbufferdesc_t size = ", sizeof(enetbufferdesc_t));
	print("\n rx_ring size = ", sizeof(rx_ring));
	memset(rx_ring, 0, sizeof(rx_ring));
	memset(tx_ring, 0, sizeof(tx_ring));

  for (int i=0; i < RXSIZE; i++) {
		rx_ring[i].flags.all = 0x8000; // empty flag
		rx_ring[i].buffer = rxbufs + i * 128;
	}
	rx_ring[RXSIZE-1].flags.all = 0xA000; // empty & wrap flags
	for (int i=0; i < TXSIZE; i++) {
		tx_ring[i].buffer = txbufs + i * 128;
	}
	tx_ring[TXSIZE-1].flags.all = 0x2000; // wrap flag

	ENET_EIMR = 0;
	ENET_MSCR = ENET_MSCR_MII_SPEED(15);  // 12 is fastest which seems to work
	ENET_RCR = ENET_RCR_NLC | ENET_RCR_MAX_FL(1522) | ENET_RCR_CFEN |
		ENET_RCR_CRCFWD | ENET_RCR_PADEN | ENET_RCR_RMII_MODE |
		/* ENET_RCR_FCE | ENET_RCR_PROM | */ ENET_RCR_MII_MODE;
	ENET_TCR = ENET_TCR_ADDINS | /* ENET_TCR_RFC_PAUSE | ENET_TCR_TFC_PAUSE | */
		ENET_TCR_FDEN;
	ENET_PALR = (MACADDR1 << 8) | ((MACADDR2 >> 16) & 255);
	ENET_PAUR = ((MACADDR2 << 16) & 0xFFFF0000) | 0x8808;
	ENET_OPD = 0x10014;
	ENET_IAUR = 0;
	ENET_IALR = 0;
	ENET_GAUR = 0;
	ENET_GALR = 0;
	ENET_RDSR = (uint32_t)rx_ring;
	ENET_TDSR = (uint32_t)tx_ring;
	ENET_MRBR = 512;
	ENET_TACC = ENET_TACC_SHIFT16;
	//ENET_TACC = ENET_TACC_SHIFT16 | ENET_TACC_IPCHK | ENET_TACC_PROCHK;
	ENET_RACC = ENET_RACC_SHIFT16 | ENET_RACC_PADREM;

	ENET_ECR = 0xF0000000 | ENET_ECR_DBSWP | ENET_ECR_EN1588 | ENET_ECR_ETHEREN;
	ENET_RDAR = ENET_RDAR_RDAR;
	ENET_TDAR = ENET_TDAR_TDAR;
}

// initialize the ethernet hardware
void setup()
{
	while (!Serial) ; // wait
	print("Ethernet Testing");
	print("----------------\n");
	begin();

	printhex("\n MDIO PHY ID2 (LAN8720A should be 0007): ", mdio_read(0, 2));
	printhex("\n MDIO PHY ID3 (LAN8720A should be C0F?): ", mdio_read(0, 3));
}

// watch for data to arrive
void loop()
{
	static int rxnum=0;
	volatile enetbufferdesc_t *buf;

	buf = rx_ring + rxnum;


	if (buf->flags.E == 0) {
    if (buf->flags.TR == 0) {
      Serial.println("--------------------------------------------");
      Serial.println(buf->protocolType, DEC);
      Serial.println(buf->header>>2, DEC);
      Serial.println(buf->moreflags.all, BIN);
      if (buf->flags.L == 1) {
        if (buf->flags.MC == 1 || buf->flags.BC == 1 || buf->flags.M == 0) {
          if (buf->moreflags.ICE == 0) {
            incoming(buf->buffer, buf->length, buf->flags.all);
          }else{Serial.println("not IP frame or IP checksum error");}
        }else{Serial.println("mac miss");}
      }else{Serial.println("can't handel multiframe right now");}
    }else{Serial.println("ERROR");}
		if (rxnum < RXSIZE-1) {
			buf->flags.all = 0x8000;
			rxnum++;
		} else {
			buf->flags.all = 0xA000;
			rxnum = 0;
		}
	}
	// TODO: if too many packets arrive too quickly, which is
	// a distinct possibility when we spend so much time printing
	// to the serial monitor, ENET_RDAR_RDAR can be cleared if
	// the receive ring buffer fills up.  After we free up space,
	// ENET_RDAR_RDAR needs to be set again to restart reception
	// of incoming packets.
}

// when we get data, try to parse it
void incoming(void *packet, unsigned int len, uint16_t flags)
{
	const uint8_t *p8;
  const uint8_t *b8;
	const uint16_t *p16;
  const uint16_t *b16;
	const uint32_t *p32;
	IPAddress src, dst;
	uint16_t type;



  T_EthernetHeader *ethernetHeader;
  ethernetHeader = (T_EthernetHeader*)packet;

  Serial.println();
  print("dstMAC: ");
  printmac(ethernetHeader->dstMAC);
  Serial.println();
  print("srcMAC: ");
  printmac(ethernetHeader->srcMAC);
  Serial.println();
  // Serial.print(ethernetHeader->type);
  if (1 == 1) {
    print("EtherType: ");
    p8 = (const uint8_t*)packet + sizeof(T_EthernetHeader);
    switch(ethernetHeader->type) {
      case EtherType::IPv4:
        print("IPv4 \n");
        T_IPv4Header *IPv4Header;
        IPv4Header = (T_IPv4Header*)p8;
        print(" version: ", IPv4Header->Version_IHL.version);
        print("\n header len: ", IPv4Header->Version_IHL.IHL);
        print("\n DiffServ level: ", IPv4Header->DSCP_ECN.DSCP);
        print("\n total length: ", __builtin_bswap16(IPv4Header->totalLength));
        print("\n srcIP = ", IPv4Header->srcIP[0]);
        print(".", IPv4Header->srcIP[1]);
        print(".", IPv4Header->srcIP[2]);
        print(".", IPv4Header->srcIP[3]);
        Serial.println();

        print(" dstIP = ", IPv4Header->dstIP[0]);
        print(".", IPv4Header->dstIP[1]);
        print(".", IPv4Header->dstIP[2]);
        print(".", IPv4Header->dstIP[3]);
        Serial.println();

        if(1 == 1) {
          p8 = (const uint8_t*)IPv4Header + (IPv4Header->Version_IHL.IHL)*4;
          switch(IPv4Header->protocol) {
            case UDP:
              print("UDP \n");
              T_UDPHeader *UDPHeader;
              // p8 = (const uint8_t*)IPv4Header + (IPv4Header->Verion_IHL & 0x0F)*4;
              UDPHeader = (T_UDPHeader *)p8;
              print(" srcPort: ", __builtin_bswap16(UDPHeader->srcPort));
              print("\n dstPort: ", __builtin_bswap16(UDPHeader->dstPort));
              print("\n len: ", __builtin_bswap16(UDPHeader->length));
            break;

            default:
              print("unknown");
            break;
          }
        }else {
          print("to someone else");
        }
      break;

      case EtherType::ARP:
        print("ARP \n");
        const T_arp *ARPpacket;
        ARPpacket = (const T_arp *)p8;

        if(ARPpacket->HTYPE == 0x0100 && ARPpacket->PTYPE == EtherType::IPv4) {
          if(ARPpacket->OPER == 0x0100) {
            print("ARP request");
            Serial.print("  Who is ");
            Serial.print(IPAddress(ARPpacket->TPA));
            Serial.print(" Tell ");
            Serial.print(IPAddress(ARPpacket->SPA));
            if(IPAddress(ARPpacket->TPA) == myaddress) {
              Serial.println("\n i am, sending reply.. ");
              IPAddress from(ARPpacket->SPA);
              arp_reply(ARPpacket->SHA, ARPpacket->SPA);
            }
          }else if(ARPpacket->OPER == 0x0200) {
            print("ARP reply");
          }
        }else{print("error");}
  		  /*  if (p32[4] == 0x00080100 && p32[5] == 0x01000406) {
  			// request is for IPv4 address of ethernet mac
  			IPAddress from((p16[15] << 16) | p16[14]);
  			IPAddress to(p32[10]);
  			Serial.print("  Who is ");
  			Serial.print(to);
  			Serial.print(" from ");
  			Serial.print(from);
  			Serial.print(" (");
  			printmac(p8 + 22);
  			Serial.println(")");
  			if (to == myaddress) {
  				arp_reply(p8+22, from);
        }
        }*/
      break;

      default:
        print("unknown");
      break;
      }
    }else {
      print("to some one else");
    }




  typedef struct {
    uint8_t   filler0[2];
    uint8_t   dstMAC[6];
    uint8_t   srcMAC[6];
    uint16_t  type;
    uint8_t   filler1[9];
    uint8_t   protocol;
    uint16_t  IPV4checksum;
    IPAddress srcIP;
    IPAddress dstIP;

  } artnetPacket_t;

  artnetPacket_t *a;

  a = (artnetPacket_t *)packet;
  b8 = (const uint8_t *)packet;
  p8 = (const uint8_t *)packet + 2;
  b8 = (const uint8_t *)packet;
  p16 = (const uint16_t *)p8;
  b16 = (const uint16_t *)packet;
  p32 = (const uint32_t *)packet;
  /*
  Serial.println();
  print("dstMAC: ");
  printmac(a->dstMAC);
  Serial.println();
  print("dstMAC: ");
  printmac(a->srcMAC);
  printhex(" type ", a->type);
  printhex(" protocol = ", a->protocol);
  print("srcIP = ", a->srcIP[0]);
  print(".", a->srcIP[1]);
  print(".", a->srcIP[2]);
  print(".", a->srcIP[3]);
  Serial.println();
  print("dstIP = ", a->dstIP[0]);
  print(".", a->dstIP[1]);
  print(".", a->dstIP[2]);
  print(".", a->dstIP[3]);
  Serial.println();
  */


	Serial.println();
	print("data, len=", len);

	type = p16[6];
	if (type == 0x0008) {   // IPv4
		src = p32[7];
		dst = p32[8];
		Serial.print("IPv4 Packet, src=");
		Serial.print(src);
		Serial.print(", dst=");
		Serial.print(dst);
    if (p8[23] == 0x11) { //Udp Protocoll
      Serial.print(", Udp=");
      Serial.print( __builtin_bswap16(b16[19]));
      if (__builtin_bswap16(b16[19]) == 6454) {
        printhex("  is artnet? ", b8[44]);
/*        if(memcmp(b8[44], "Art-Net", 8)) {
          Serial.print("yes");
        }*/
      }
    }
    printhex(",  protocol", p8[23]);
		printpacket(p8, len - 2);
		if (p8[23] == 1 && dst == myaddress) {
			Serial.println("  Protocol is ICMP:");
			if (p8[34] == 8) {
				print("  echo request:");
				uint16_t id = __builtin_bswap16(p16[19]);
				uint16_t seqnum = __builtin_bswap16(p16[20]);
				printhex("   id = ", id);
				print("   sequence number = ", seqnum);
				ping_reply((uint32_t *)packet, len);
			}
		}
	}
}

// compose an answer to ARP requests
void arp_reply(const uint8_t *mac, const uint8_t *ip) {
  void *packet = calloc(1, sizeof(T_arp));


    T_arp *ptr = (T_arp *)packet;
    ptr->HTYPE = 0x0100;             //ethernet
    ptr->PTYPE = IPv4;            //IPv4
    ptr->HLEN = 6;
    ptr->PLEN = 4;
    ptr->OPER = 0x0200;  //1 for request, 2 for reply.
    memcpy(ptr->SHA, MACADDR, 6);
    memcpy(ptr->SPA, myaddress_byte, 4);
    memcpy(ptr->THA, mac, 6);
    memcpy(ptr->TPA, ip, 4);

	Serial.println("ARP Reply:");

	outgoing_pre(packet, sizeof(T_arp), mac, EtherType::ARP);
}
void outgoing_pre(void *packet, unsigned int len, const uint8_t* dstMAC, uint16_t EtherType) {

	static int txnum=0;
	volatile enetbufferdesc_t *buf;
	uint16_t flags;

	buf = tx_ring + txnum;
	flags = buf->flags.all;
	if ((flags & 0x8000) == 0) {
		print("tx, num=", txnum);
    T_EthernetHeader *ptr = (T_EthernetHeader *)buf->buffer;
    ptr->pad = 0;       // first 2 bytes are padding
    ptr->type = EtherType;
		buf->length = len+sizeof(T_EthernetHeader);
    memcpy(ptr->dstMAC, dstMAC, 6);
		memcpy(buf->buffer+sizeof(T_EthernetHeader), packet, len);
		buf->flags.all = flags | 0x8C00;
		ENET_TDAR = ENET_TDAR_TDAR;
		if (txnum < TXSIZE-1) {
			txnum++;
		} else {
			txnum = 0;
		}
	}
}

// compose an reply to pings
void ping_reply(const uint32_t *recv, unsigned int len)
{
	uint32_t packet[32];
	uint8_t *p8 = (uint8_t *)packet + 2;

	if (len > sizeof(packet)) return;
	memcpy(packet, recv, len);
	memcpy(p8, p8 + 6, 6); // send to the mac address we received
	// hardware automatically adds our mac addr
	packet[8] = packet[7]; // send to the IP number we received
	packet[7] = (uint32_t)myaddress;
	p8[34] = 0;            // type = echo reply
	// TODO: checksums in IP and ICMP headers - is the hardware
	// really inserting correct checksums automatically?
	printpacket((uint8_t *)packet + 2, len - 2);
	outgoing(packet, len);
}

// transmit a packet
void outgoing(void *packet, unsigned int len)
{
	static int txnum=0;
	volatile enetbufferdesc_t *buf;
	uint16_t flags;

	buf = tx_ring + txnum;
	flags = buf->flags.all;
	if ((flags & 0x8000) == 0) {
		print("tx, num=", txnum);
		buf->length = len;
		memcpy(buf->buffer, packet, len);
		buf->flags.all = flags | 0x8C00;
		ENET_TDAR = ENET_TDAR_TDAR;
		if (txnum < TXSIZE-1) {
			txnum++;
		} else {
			txnum = 0;
		}
	}
}

// read a PHY register (using MDIO & MDC signals)
uint16_t mdio_read(int phyaddr, int regaddr)
{
	ENET_MMFR = ENET_MMFR_ST(1) | ENET_MMFR_OP(2) | ENET_MMFR_TA(0)
		| ENET_MMFR_PA(phyaddr) | ENET_MMFR_RA(regaddr);
	// TODO: what is the proper value for ENET_MMFR_TA ???
	//int count=0;
	while ((ENET_EIR & ENET_EIRM_MII) == 0) {
		//count++; // wait
	}
	//print("mdio read waited ", count);
	uint16_t data = ENET_MMFR;
	ENET_EIR = ENET_EIRM_MII;
	//printhex("mdio read:", data);
	return data;
}

// write a PHY register (using MDIO & MDC signals)
void mdio_write(int phyaddr, int regaddr, uint16_t data)
{
	ENET_MMFR = ENET_MMFR_ST(1) | ENET_MMFR_OP(1) | ENET_MMFR_TA(0)
		| ENET_MMFR_PA(phyaddr) | ENET_MMFR_RA(regaddr) | ENET_MMFR_DATA(data);
	// TODO: what is the proper value for ENET_MMFR_TA ???
	//int count=0;
	while ((ENET_EIR & ENET_EIRM_MII) == 0) {
		//count++; // wait
	}
	ENET_EIR = ENET_EIRM_MII;
	//print("mdio write waited ", count);
	//printhex("mdio write :", data);
}


// misc print functions, for lots of info in the serial monitor.
// this stuff probably slows things down and would need to go
// for any hope of keeping up with full ethernet data rate!

void print(const char *s)
{
	Serial.print(s);
}

void print(const char *s, int num)
{
	Serial.print(s);
	Serial.print(num);
}

void printhex(const char *s, int num)
{
	Serial.print(s);
	Serial.println(num, HEX);
}

void printmac(const uint8_t *data)
{
	Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X",
		data[0], data[1], data[2], data[3], data[4], data[5]);
}

void printpacket(const uint8_t *data, unsigned int len)
{
#if 1
	unsigned int i;

	for (i=0; i < len; i++) {
		Serial.printf(" %02X", *data++);
		if ((i & 15) == 15) Serial.println();
	}
	Serial.println();
#endif
}
