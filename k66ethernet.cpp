#include "k66ethernet.h"

// #include "IPAddress.h"
#include "EtherType.h"
#include "IPprotocol.h"
#include "struct.h"
#include "print.h"

//static uint8_t _macAddr[6];

k66ethernet::k66ethernet() {
  uint8_t mac[6] = {0x04, 0xE9, 0xE5, 0x03, 0x02, 0x01};
  memcpy(_macAddr, mac, 6);

  print("enetbufferdesc_t size = ", sizeof(enetbufferdesc_t));
	print("\n _rxRing size = ", sizeof(_rxRing));
	memset(_rxRing, 0, sizeof(_rxRing));
	memset(_txRing, 0, sizeof(_txRing));

  _currentTxBuffer = 0;
  _currentRxBuffer = 0;
  _ipMacMapLocation = 0;
  _watingForArp = 0;
}

k66ethernet::k66ethernet(uint8_t *mac) {
  memcpy(_macAddr, mac, 6);

  print("enetbufferdesc_t size = ", sizeof(enetbufferdesc_t));
	print("\n _rxRing size = ", sizeof(_rxRing));
	memset(_rxRing, 0, sizeof(_rxRing));
	memset(_txRing, 0, sizeof(_txRing));

  _currentTxBuffer = 0;
  _currentRxBuffer = 0;
  _ipMacMapLocation = 0;
  _watingForArp = 0;
}

void rxIRQ(){
  static int interruptFlag = 0;
  interruptFlag++;
 
  printhex("........Interrupt........ ", interruptFlag);
  if ((_rxRing[_currentRxBuffer].flags.all & 0xC000) == 0) {
    if ((_rxRing[_currentRxBuffer].moreflags.MSB & 0x20) == 0x20) {
      ethernetHeader_t *ethernetHeader = (ethernetHeader_t*)_rxRing[_currentRxBuffer].buffer;
      if(ethernetHeader->type == EtherType::ARP) {
        _watingForArp = _currentRxBuffer;
      }
    }
  }
  if (_currentRxBuffer < RXSIZE-1) {
    _currentRxBuffer++;
  } else {
    _currentRxBuffer = 0;
  }
  ENET_EIR = ENET_EIR_RXF;
  // ENET_EIR = ENET_EIR_RXB;
}

void k66ethernet::begin() {

  __disable_irq();

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


  for (int i=0; i < RXSIZE; i++) {
		_rxRing[i].flags.all = 0x8000; // empty flag
    _rxRing[i].moreflags.LSB = 0x80; // set Interrupt true
		_rxRing[i].buffer = _rxBuffer + i * 128;
	}
	_rxRing[RXSIZE-1].flags.all = 0xA000; // empty & wrap flags
	for (int i=0; i < TXSIZE; i++) {
		_txRing[i].buffer = _txBuffer + i * 128;
    // _txRing[i].moreflags.all = 0x80; // set Interrupt true
	}
	_txRing[TXSIZE-1].flags.all = 0x2000; // wrap flag

  ENET_EIR = ENET_EIR_RXF; //clear Interrupt
  // ENET_EIR = ENET_EIR_RXB; //clear Interrupt
  // ENET_EIR = ENET_EIR_MII;

	ENET_EIMR = ENET_EIRM_RXF;// | ENET_EIRM_RXB;// | ENET_EIRM_MII;
	ENET_MSCR = ENET_MSCR_MII_SPEED(15);  // 12 is fastest which seems to work
	ENET_RCR = ENET_RCR_NLC | ENET_RCR_MAX_FL(1522) | ENET_RCR_CFEN |
		ENET_RCR_CRCFWD | ENET_RCR_PADEN | ENET_RCR_RMII_MODE |
		/* ENET_RCR_FCE | ENET_RCR_PROM | */ ENET_RCR_MII_MODE;
	ENET_TCR = ENET_TCR_ADDINS | /* ENET_TCR_RFC_PAUSE | ENET_TCR_TFC_PAUSE | */
		ENET_TCR_FDEN;

  uint32_t macUp = (_macAddr[0]<<16) | (_macAddr[1]<<8) | (_macAddr[2]);
  uint32_t macDn = (_macAddr[3]<<16) | (_macAddr[4]<<8) | (_macAddr[5]);


  ENET_PALR = (macUp << 8) | ((macDn >> 16) & 255);
	ENET_PAUR = ((macDn << 16) & 0xFFFF0000) | 0x8808;
	ENET_OPD = 0x10014;
	ENET_IAUR = 0;
	ENET_IALR = 0;
	ENET_GAUR = 0;
	ENET_GALR = 0;
	ENET_RDSR = (uint32_t)_rxRing;
	ENET_TDSR = (uint32_t)_txRing;
	ENET_MRBR = 512;
	//ENET_TACC = ENET_TACC_SHIFT16;
	ENET_TACC = ENET_TACC_SHIFT16 | ENET_TACC_IPCHK | ENET_TACC_PROCHK;
	ENET_RACC = ENET_RACC_SHIFT16 | ENET_RACC_PADREM;

	ENET_ECR = 0xF0000000 | ENET_ECR_DBSWP | ENET_ECR_EN1588 | ENET_ECR_ETHEREN;
	ENET_RDAR = ENET_RDAR_RDAR;
	ENET_TDAR = ENET_TDAR_TDAR;

  // set interrupt vector and enable interrupts
  attachInterruptVector(IRQ_ENET_RX, rxIRQ);
  NVIC_ENABLE_IRQ(IRQ_ENET_RX);
  __enable_irq();

}

int k66ethernet::beginEthernetPacket(uint8_t *mac, EtherType type) {
  if ((_txRing[_currentTxBuffer].flags.all & 0x8000) == 0) {
    if ((_txRing[_currentTxBuffer].flags.all & 0x4000) == 0) {
      ethernetHeader_t *ethernetHeader = (ethernetHeader_t*)_txRing[_currentTxBuffer].buffer;
      ethernetHeader->pad = 0;
      memcpy(ethernetHeader->dstMAC, mac, 6);
      //memcpy(ethernetHeader->srcMAC, _macAddr, 6); // hardware will do this
      ethernetHeader->type = type;
      _txRing[_currentTxBuffer].length = sizeof(ethernetHeader_t);
      _txRing[_currentTxBuffer].flags.all |= 0x4000;  // set user flag to show that this buffer is in use
      return 0;
    }else {
      print("not markt as ready to sent yet! mark as ready and com back ");
      return -2;
    }
  }else{
    print("not sent yet! wait..");
    return -1;
  }
}

int k66ethernet::sendArp(uint8_t *trgIP, uint8_t *trgMAC, uint16_t operation, bool awaitRespons) {
  uint8_t mac[6] = {255,255,255,255,255,255};
  beginEthernetPacket(mac, EtherType::ARP);
  uint8_t* prt = (uint8_t*)_txRing[_currentTxBuffer].buffer + _txRing[_currentTxBuffer].length;
  arpHeader_t *arpHeader = (arpHeader_t*)prt;
  arpHeader->HTYPE = 0x0100;
  arpHeader->PTYPE = EtherType::IPv4;
  arpHeader->HLEN = 6;
  arpHeader->PLEN = 4;
  arpHeader->OPER = __builtin_bswap16(operation);
  memcpy(arpHeader->SHA, _macAddr, 6);
  memcpy(arpHeader->SPA, _ipAddr, 4);
  memcpy(arpHeader->THA, trgMAC, 6);
  memcpy(arpHeader->TPA, trgIP, 4);
  _txRing[_currentTxBuffer].length += sizeof(arpHeader_t);

  _watingForArp = -1;
  print("who has ");
  printip(trgIP);
  print(" ? \n");
  endPacket();

  for(int i = 0; i < 100; i++) {
    delay(1);
    if(_watingForArp != -1) {
      *prt = (uint8_t*)_rxRing[_currentRxBuffer].buffer;
      *arpHeader = (arpHeader_t*)prt+sizeof(ethernetHeader_t);
      if (arpHeader->OPER = 0x0200) {
        if (memcmp(arpHeader->THA, _macAddr, 6) == 0) {
          if (memcmp(arpHeader->SPA, trgIP, 4) == 0) {
            memcpy(_ipMap[_ipMacMapLocation], trgIP, 4);
            memcpy(_macMap[_ipMacMapLocation], arpHeader->SHA, 6);
            printmac(arpHeader->SHA);
            print(" has! \n");
            return _ipMacMapLocation;
          }
        }
      }
    }
  }
  print("arp timeout \n");
  return -1;
}

int k66ethernet::beginIPv4Packet(uint8_t *ip, uint8_t protocol, uint8_t DSCP) {
  uint8_t mac[6];
  if (ip[0] <= 192) {
    // unicast
    for (int i = 0; i < IPMACMAPSIZE; i++) {
      if (memcmp(ip, _ipMap[i], 4) == 0) {
        memcpy(mac, _macMap[i], 6);
        break;
      }else if (_ipMap[i][0] == 0) {
        // empty map make a arp request
      }
    }
  }else if (ip[0] == 255) {
    //broardcast
    for(int i = 0; i < 6; i++) {
      mac[i] = 255;
    }
  }else if (ip[0] == 224) {
    //Multicast
  }

  if (beginEthernetPacket(mac, EtherType::IPv4) == 0){
    uint8_t* prt = (uint8_t*)_txRing[_currentTxBuffer].buffer + _txRing[_currentTxBuffer].length;
    IPv4Header_t *IPv4Header = (IPv4Header_t*)prt;
    IPv4Header->Version_IHL = 0x45;
    IPv4Header->DSCP_ECN = DSCP<<2;  //0x2e for high priorety 0x00 for best efort
    IPv4Header->totalLength = __builtin_bswap16(sizeof(IPv4Header_t));
    IPv4Header->ID = 0;
    IPv4Header->Flags_FragmentOffset = 0;
    IPv4Header->TTL = 20; //TODO??????
    IPv4Header->protocol = protocol;
    IPv4Header->headerChecksum = 0; //auto checksum generation is enabled
    memcpy(IPv4Header->srcIP, _ipAddr, 4);
    memcpy(IPv4Header->dstIP, ip, 4);
    _txRing[_currentTxBuffer].length += sizeof(IPv4Header_t);
    return 0;
  }else{
    return -1;
  }
}

void k66ethernet::endPacket() {
  _txRing[_currentTxBuffer].flags.all &= 0xBFFF; //remove user flag to show that this buffer i now free
  _txRing[_currentTxBuffer].flags.all |= 0x8C00;
  ENET_TDAR = ENET_TDAR_TDAR;
  if (_currentTxBuffer < TXSIZE-1) {
    _currentTxBuffer++;
  } else {
    _currentTxBuffer = 0;
  }
}

void k66ethernet::setIP(uint8_t *ip) {
  memcpy(_ipAddr, ip, 4);
}
