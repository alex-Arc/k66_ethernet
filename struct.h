#ifndef struct_h
#define struct_h

typedef struct S_EthernetHeader {
  uint16_t   pad;
  uint8_t   dstMAC[6];
  uint8_t   srcMAC[6];
  uint16_t  type;
} T_EthernetHeader;

typedef struct S_IPv4Header {
  //T_EthernetHeader  ethernetHeader;
  struct {
    uint8_t     IHL       : 4;
    uint8_t     version   : 4;
  }Version_IHL;
  struct {
    uint8_t     ECN       :2;
    uint8_t     DSCP      :6;
  }DSCP_ECN;
  uint16_t    totalLength;
  uint16_t    ID;
  uint16_t    Flags_FragmentOffset;
  uint8_t     TTL;
  uint8_t     protocol;
  uint16_t    headerChecksum;
  uint8_t     srcIP[4];
  uint8_t     dstIP[4];
  uint32_t    options[4];
} T_IPv4Header;

typedef struct S_arp{
  uint16_t  HTYPE;    //Hardware type
  uint16_t  PTYPE;    //Protocol type
  uint8_t   HLEN;     //Hardware length. lenght hardware address is 6
  uint8_t   PLEN;     //Protocol length. length of IPv4 addr is 4
  uint16_t  OPER;     //Operation 1 for request, 2 for reply
  uint8_t   SHA[6];   //sender hardware address (MAC)
  uint8_t   SPA[4];   //Sender protocol address (IP)
  uint8_t   THA[6];    //Target hardware address (MAC)
  uint8_t   TPA[4];    //Target protocol address (IP)
}T_arp;

typedef struct S_UDPHeader {
  uint16_t          srcPort;
  uint16_t          dstPort;
  uint16_t          length;
  uint16_t          checksum;
} T_UDPHeader;

typedef struct {
	uint16_t length;
  union {
    uint16_t all;
    struct {
      uint16_t TR   : 1;  //frame must be discarded. Set if the receive frame is truncated (frame length >TRUNC_FL).
      uint16_t OV   : 1;  //This field is valid only if the L field is set. A receive FIFO overrun occurred
      uint16_t CR   : 1;  //This field is valid only if the L field is set. Receive CRC or frame error.
      uint16_t Reserved0 : 1;  //Reserved
      uint16_t NO   : 1;  //This field is valid only if the L field is set. Receive non-octet aligned frame
      uint16_t LG   : 1;  //This field is valid only if the L field is set. A frame length greater than RCR[MAX_FL] was recognized.
      uint16_t MC   : 1;  //Set if is multicast and not BC.
      uint16_t BC   : 1;  //Set if is broadcast (FFFF_FFFF_FFFF).
      uint16_t M    : 1;  //This field is valid only if the L and PROM bits are set. 0 The frame was received because of an address recognition hit. 1 The frame was received because of promiscuous mode.
      uint16_t Reserved1 : 2;  //Reserved
      uint16_t L    : 1;  //last frame
      uint16_t RO2  : 1;
      uint16_t W    : 1;  //Warp
      uint16_t RO1  : 1;  //Receive software ownership. This field is reserved for use by software. This read/write field is not modified by hardware, nor does its value affect hardware.
      uint16_t E    : 1;  //0 data in buffer. 1 buffer empty.
    };
  }flags;
	void *buffer;
  union {
    uint32_t all;
    struct {
      uint8_t Reserved0  : 7;
      uint8_t INT   : 1;
      uint8_t UC    : 1;        //unicast frame
      uint8_t CE    : 1;        //Collision error The frame is invalid. Only valid if the L field is set.
      uint8_t PE    : 1;        //PHY error The frame is invalid. Only valid if the L field is set.
      uint8_t Reserved1  : 4;
      uint8_t ME    : 1;        //Erroe in memory or receive FIFO overflow. Only valid if the L field is set.
      uint8_t FRAG  : 1;        //This is a IPv4 fragment frame. Only valid if the L field is set.
      uint8_t IPV6  : 1;        //IPV6 tag. Only valid if the L field is set.
      uint8_t VLAN  : 1;        //VLAN tag. Only valid if the L field is set.
      uint8_t Reserved2  : 1;
      uint8_t PCR  : 1;         //Protocol checksum error. Only valid if the L field is set.
      uint8_t ICE  : 1;         //IP header checksum error. Only valid if the L field is set.
      uint8_t Reserved3  : 2;
      uint8_t Reserved4  : 5;
      uint8_t VPCP : 3;         //VLAN priority code 0-7
    };
  }moreflags;
	uint16_t checksum;
	uint8_t header;
  uint8_t protocolType :4;
	uint32_t dmadone;
	uint32_t timestamp;
	uint32_t unused1;
	uint32_t unused2;
} enetbufferdesc_t;

typedef struct {
  uint8_t header[8];
} artPacket_t;

#endif
