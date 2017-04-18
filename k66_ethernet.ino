#include "IPAddress.h"
#include "EtherType.h"
#include "IPprotocol.h"
#include "struct.h"
#include "print.h"

//static const int IPv4 = 0x0008;     //byte swap Internet Protocol version 4
//static const int ARP = 0x0608;   //Address Resolution Protocol
int interruptFlag = 0;
// set this to an unused IP number for your network
IPAddress myaddress(2, 2, 6, 10);
uint8_t myaddress_byte[4] = {2,2,6,10};
/*IPAddress subNet(255, 255, 255, 0);
IPAddress prefix(0, 0, 0, 0);*/

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
    rx_ring[i].moreflags.all = 0x800000; // set Interrupt true
		rx_ring[i].buffer = rxbufs + i * 128;
	}
	rx_ring[RXSIZE-1].flags.all = 0xA000; // empty & wrap flags
	for (int i=0; i < TXSIZE; i++) {
		tx_ring[i].buffer = txbufs + i * 128;
    // tx_ring[i].moreflags.all = 0x80; // set Interrupt true
	}
	tx_ring[TXSIZE-1].flags.all = 0x2000; // wrap flag

  ENET_EIR = ENET_EIR_RXF; //clear Interrupt
  ENET_EIR = ENET_EIR_RXB; //clear Interrupt
  // ENET_EIR = ENET_EIR_MII;

	ENET_EIMR = ENET_EIRM_RXF | ENET_EIRM_RXB;// | ENET_EIRM_MII;
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

void rxIRQ(void){
  Serial.println("........Interrupt........");
  interruptFlag++;
  ENET_EIR = ENET_EIR_RXF;
  ENET_EIR = ENET_EIR_RXB;
  // ENET_EIR = ENET_EIR_MII;
}

/*void attachInterrupt(void (*isr)(void)) {
  _VectorsRam[IRQ_ENET_RX + 16] = isr;
  NVIC_ENABLE_IRQ(IRQ_ENET_RX);
}*/

// initialize the ethernet hardware
void setup()
{
  Serial.begin(9600);
	while (!Serial) ; // wait
	print("Ethernet Testing");
	print("----------------\n");
  __disable_irq();
	begin();

  attachInterruptVector(IRQ_ENET_RX, rxIRQ);
  // NVIC_IS_ACTIVE(IRQ_ENET_RX);
  NVIC_ENABLE_IRQ(IRQ_ENET_RX);
  //attachInterrupt(rxIRQ);
  __enable_irq();
  Serial.println();
  // printhex("NVIC_GET_PRIORITY: ", NVIC_GET_PRIORITY(IRQ_ENET_RX));
	printhex("\n MDIO PHY ID2 (LAN8720A should be 0007): ", mdio_read(0, 2));
	printhex("\n MDIO PHY ID3 (LAN8720A should be C0F?): ", mdio_read(0, 3));
/*
  Serial.print("my IP: ");
  Serial.println(myaddress);
  Serial.print("my subnet: ");
  Serial.println(subNet);
  Serial.print("Network prefix: ");
  prefix = myaddress & subNet;
  Serial.println(prefix);*/
  //NVIC_TRIGGER_IRQ(IRQ_ENET_ERROR);

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


// watch for data to arrive
void loop()
{
	static int rxnum=0;
	volatile enetbufferdesc_t *buf;

	buf = rx_ring + rxnum;


	if (buf->flags.E == 0) {
    printhex("interruptFlag: ", interruptFlag);
    if (buf->flags.TR == 0) {
      Serial.println("--------------------------------------------");
      printhex("protocolType: ", buf->protocolType);
      printhex("header: ", buf->header);
      Serial.print("flags: ");
      Serial.println(buf->moreflags.all, HEX);
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
    // buf->flags.moreflags.all |= 0x800000; // set Interrupt true;
    Serial.println();
	}
	// TODO: if too many packets arrive too quickly, which is
	// a distinct possibility when we spend so much time printing
	// to the serial monitor, ENET_RDAR_RDAR can be cleared if
	// the receive ring buffer fills up.  After we free up space,
	// ENET_RDAR_RDAR needs to be set again to restart reception
	// of incoming packets.
}

// when we get data, try to parse it
void incoming(void *packet, unsigned int len, uint16_t flags) {
	const uint8_t *p8;
	IPAddress src, dst;

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
    break;

    default:
      print("unknown");
    break;
  }
}
