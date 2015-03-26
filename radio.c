#include <string.h>
#include "nrf24l01p.h"
#include "radio.h"

extern volatile BYTE IRQstatus;

//write the packet full of 0
PACKET_STATE clearPacket(Packet *packet) {
	BYTE i;
	packet->id = 0;
	for(i = 0; i < 5; i++) {
		packet->address[i] = 0;
	}
	packet->timestamp = 0;
	packet->command = 0;
	for(i = 0; i < 23; i++) {
		packet->payload[i] = 0;
	}
	
	return PACKET_CLEAR;
}

//figure the appropriate action for the command found in the packet
RADIO_STATE parsePacket(BYTE command) {

	switch(command) {
		case COMMAND_DATA :
		{
			return PRINT_DATA;
			break;
		}
		
		case COMMAND_SLEEP :
		{
			return RADIO_SLEEP;
			break;
		}
		
		case COMMAND_STANDBY :
		{
			return RADIO_SLEEP;
			break;
		}
		
		default:
			break;
	}

	return RADIO_LISTEN;
}

//prints up to 16 bytes as HEX or TXT from a packet on the LCD
void printPacket(BYTE *packet, BYTE start, BYTE len, BYTE type) {
	BYTE i;
	
	LCD_Position(0, 0);
	if( (start < 0) || (start > PACKET_SIZE) ) {
		start = 0;
	}
	
	if( (len < 0) || (len > PACKET_SIZE) ) {
		len = PACKET_SIZE;
	} else if(len > 16) {
		len = 16;
	}
	
	if(type == 0) {
		for(i = 0; i < len; i++) {
		
			LCD_PrHexByte(packet[start+i]);
		
			if(i == 7) {
				LCD_Position(1,0);
			}
		}
	} else if (type == 1) {
		char buf[16];
		memcpy(buf, &(packet[start]), len);
		buf[len] = '\0';

		LCD_Position(1,0);
		LCD_PrString(buf);
	}
}

//copy a buffer of data to packet.payload.
//returns 1 if payload couldn't hold all the data
//returns 0 if all data fit in payload
//UNTESTED AS OF YET!
BYTE enqueueData(Packet *packet, void *data, BYTE len) {
	BYTE index;
	BYTE *ptr;
	ptr = &packet->payload[0];
	index = packet->payloadSize;
	ptr += index;
	if((index + len) > 23) {
		return 0;
	}
	
	memcpy(ptr+index, data, len);
	packet->payloadSize += len;
	return 1;
}

//waits a packet forever. this will hang if nothing is received
void waitForPacket(Packet *packet) {
	while( (IRQstatus & RX_DR) == 0 )
		;
	nrf24ReadPayload((BYTE*)packet);
	IRQstatus &= ~RX_DR;
	nrf24ClearStatus(RX_DR);
}

//sends a packet and if EN_AA is set then waits for an ACK or MAX_RT
//returns PACKET_SENT if ACK is received or if EN_AA is not set
//returns PACKET_LOST if MAX_RT (max retrasmits) is asserted
PACKET_STATE sendPacket(Packet *packet) {
	nrf24SendPayload((BYTE*)packet);
	
	nrf24Transmit();
	
	do {
		if(IRQstatus & MAX_RT) {
			IRQstatus &= ~MAX_RT;
			nrf24ClearStatus(MAX_RT);
			nrf24SendCommand(FLUSH_TX);
			return PACKET_LOST;
		}
	} while( !(IRQstatus & TX_DS) );
	IRQstatus &= ~TX_DS;
	nrf24ClearStatus(TX_DS);
	return PACKET_SENT;
}