#include <m8c.h>
#include <string.h>
#include "PSoCAPI.h"

#include "nrf24l01p.h"
#include "radio.h"

volatile WORD timeMS = 0;
extern volatile BYTE IRQstatus;

void TimerISR(void) {
	if(timeMS >= 30000) {
		timeMS = 0;
	}
	timeMS += 10;
}

#pragma interrupt_handler IRQHandler
void IRQHandler(void) {
	IRQstatus = nrf24GetStatus();
}

void main(void)
{		
	BYTE packet_state = PACKET_CLEAR;
	BYTE radio_state = RADIO_INIT;
	BYTE radio_stop = 0;
	BYTE receivedPackets = 0;
		
	Radio radio;
	Packet packet;
	packet_state = clearPacket(&packet);
	
	CE_Off();
	CS_On();
	
	M8C_EnableGInt;
	M8C_EnableIntMask(INT_MSK0, INT_MSK0_GPIO);

	Timer8_WritePeriod(99);
    Timer8_WriteCompareValue(0);
	Timer8_EnableInt();
	
	SPIM_Start(SPIM_SPIM_MODE_0 | SPIM_SPIM_MSB_FIRST);
	
	LCD_Start(); 
	
	do {
		switch(radio_state) {
			case RADIO_INIT :
			{
				//sets the accepted packet id and default configurations
				radio.id = 0x55;
				IRQstatus = 0;
				
				nrf24Configure();
				nrf24ClearStatus(MAX_RT | TX_DS | RX_DR);
				nrf24SetMode(MODE_RX);
				
				LCD_Position(0,0);
				LCD_PrCString("RX START");
				
				radio_state = RADIO_LISTEN;
				break;
			}
			
			case RADIO_LISTEN : 
			{
				//waits forever for a packet
				waitForPacket(&packet);
				receivedPackets++;
				LCD_Position(1,14);
				LCD_PrHexByte(receivedPackets);
				radio_state = RADIO_PARSE_PACKET;
				break;
			}
				
			case RADIO_PARSE_PACKET :
			{
				if(packet.id == radio.id) {
					//packet contains valid id. let's parse it
					radio_state = parsePacket(packet.command);
				} else {
					//packet is invalid. clear it and move on.
					packet_state = clearPacket(&packet);
					radio_state = RADIO_LISTEN;					
				}
				break;
			}
				
			case RADIO_EXEC_COMMAND :
			{
				//exec command
				radio_state = RADIO_LISTEN;
				break;
			}
			
			//the first 2 bytes of payload contain
			//the time the radio has to sleep		
			case RADIO_SLEEP :
			{	
				WORD timeToSleep;
				Timer8_Start();
				timeToSleep = *(WORD*)&packet.payload[0];
				LCD_Position(0,0);
				if(packet.command == COMMAND_SLEEP) {
					nrf24PowerDown();
					LCD_PrCString("Sleeping for ");
				} else if(packet.command == COMMAND_STANDBY) {
					nrf24StandBy();
					LCD_PrCString("Standby for ");
				}
				
				LCD_PrHexInt(timeToSleep);
				packet_state = clearPacket(&packet);
				while(timeMS < timeToSleep)
					;
				Timer8_Stop();
				nrf24PowerUP();	
				radio_state = RADIO_LISTEN;
				break;
			
			}
			
			case PRINT_DATA :
			{	
				//print the first 8 bytes on the LCD as hex
				printPacket(&packet.payload[0], 0, 8, 0);
				//print the first 8 bytes on the LCD as text
				printPacket(&packet.payload[0], 0, packet.payloadSize, 1);
				
				packet_state = clearPacket(&packet);
				radio_state = RADIO_LISTEN;
				break;
			}
			
			case RADIO_STOP :
			{
				packet_state = clearPacket(&packet);
				nrf24PowerDown();
				radio_stop = 1;
				break;
			}
			
			default:
				break;
		}
	} while(!radio_stop);
	
	nrf24PowerDown();
	
	while(1)
		;
			
}