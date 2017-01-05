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
  uint8_t           Verion_IHL;          //bit 0-3: Version  bit 4-7: Internet Header Length (IHL)
  uint8_t           DSCP_ECN;
  uint16_t          totalLength;
  uint16_t          ID;
  uint16_t          Flags_FragmentOffset;
  uint8_t           TTL;
  uint8_t           protocol;
  uint16_t          headerChecksum;
  uint8_t           srcIP[4];
  uint8_t           dstIP[4];
  uint32_t          options[4];
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
      uint16_t E     : 1;
      uint16_t RO1   : 1;  //Receive software ownership. This field is reserved for use by software. This read/write field is not modified by hardware, nor does its value affect hardware.
      uint16_t W     : 1;  //Warp
      uint16_t RO2   : 1;
      uint16_t L     : 1;  //last frame
      uint16_t res2  : 2;  //Reserved
      uint16_t M     : 1;  //This field is valid only if the L and PROM bits are set. 0 The frame was received because of an address recognition hit. 1 The frame was received because of promiscuous mode.
      uint16_t BC    : 1;  //Set if the DA is broadcast (FFFF_FFFF_FFFF).
      uint16_t MC    : 1;  //Set if the DA is multicast and not BC.
      uint16_t LG    : 1;  //This field is valid only if the L field is set. A frame length greater than RCR[MAX_FL] was recognized.
      uint16_t NO    : 1;  //This field is valid only if the L field is set. Receive non-octet aligned frame
      uint16_t res1  : 1;  //Reserved
      uint16_t CR    : 1;  //This field is valid only if the L field is set. Receive CRC or frame error.
      uint16_t OV    : 1;  //This field is valid only if the L field is set. A receive FIFO overrun occurred
      uint16_t TR    : 1;  //frame must be discarded. Set if the receive frame is truncated (frame length >TRUNC_FL).
    };
  }flags;
	void *buffer;
	uint32_t moreflags;
	uint16_t checksum;
	uint16_t header;
	uint32_t dmadone;
	uint32_t timestamp;
	uint32_t unused1;
	uint32_t unused2;
} enetbufferdesc_t;

typedef struct {
  uint8_t header[8];
} artPacket_t;

#endif
