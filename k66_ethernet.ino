#include "k66ethernet.h"

//#include "print.h"

k66ethernet enet = k66ethernet();

// initialize the ethernet hardware
void setup()
{
  Serial.begin(9600);
	while (!Serial) ; // wait
	Serial.print("Ethernet Testing");
	Serial.print("----------------\n");
  uint8_t myIP[4] = {2,0,0,2};
	enet.begin();
  enet.setIP(myIP);
  delay(100);
  Serial.println();
  uint8_t dstIP[4] = {255,255,255,255};

  enet.beginIPv4Packet(dstIP, 0x11);
  enet.endPacket();
uint8_t dstIP1[4] = {2,0,0,1};
uint8_t dstMAC[6] = {0,0,0,0,0,0};
  enet.sendArp(dstIP1, dstMAC, 1);
	//printhex("\n MDIO PHY ID2 (LAN8720A should be 0007): ", enet.mdio_read(0, 2));
	//printhex("\n MDIO PHY ID3 (LAN8720A should be C0F?): ", enet.mdio_read(0, 3));
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




// watch for data to arrive
void loop() {
/*

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
  */
	// TODO: if too many packets arrive too quickly, which is
	// a distinct possibility when we spend so much time printing
	// to the serial monitor, ENET_RDAR_RDAR can be cleared if
	// the receive ring buffer fills up.  After we free up space,
	// ENET_RDAR_RDAR needs to be set again to restart reception
	// of incoming packets.
}
