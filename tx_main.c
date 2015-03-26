#include <m8c.h>
#include "PSoCAPI.h"

#include "nrf24l01p.h"
#include "radio.h"

#include <string.h>

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
	WORD timeToWait = 2000; // 2 seconds
	WORD timeSnap = 0;
	BYTE radio_stop = 0;
	BYTE packetCount = 0;
	BYTE temp[2];
	RADIO_STATE radio_state;
	PACKET_STATE packet_state;
	Packet packet;
	packet_state = clearPacket(&packet);
	IRQstatus = 0;
		
	CE_Off();
	CS_On();	
	
	M8C_EnableGInt;
	M8C_EnableIntMask(INT_MSK0, INT_MSK0_GPIO);
	
	Timer8_WritePeriod(99);
    Timer8_WriteCompareValue(0);
	Timer8_EnableInt();
	
	SPIM_Start(SPIM_SPIM_MODE_0 | SPIM_SPIM_MSB_FIRST);
	
	LCD_Start();
	
	radio_state = RADIO_INIT;

	do {
		switch(radio_state) {
			case RADIO_INIT :
			{
				nrf24Configure();
				nrf24ClearStatus(MAX_RT | TX_DS | RX_DR);
				nrf24SetMode(MODE_TX);
				LCD_Position(1,0);
				LCD_PrCString("TX START");
				radio_state = RADIO_BUILD_PACKET;
				break;
			}
			
			case RADIO_BUILD_PACKET :
			{
				packet.id = 0x55;
				
				if(packetCount == 0) {
					packet.command = COMMAND_SLEEP;
					*(WORD*)&(packet.payload[0]) = timeToWait;
					
					LCD_Position(1,0);
					LCD_PrHexByte(packet.id);
					LCD_PrCString(" ");
					LCD_PrHexByte(packet.command);
					printPacket(&(packet.payload[0]), 0, 2, 0);
				} else if(packetCount == 1) {
					char buf[] = "qWeRtYuIoP";
					packet.command = COMMAND_DATA;
					packet.payloadSize = strlen(buf)+1;
					memcpy(&(packet.payload[0]), &(buf[0]), packet.payloadSize);
					
					LCD_Position(1,0);
					LCD_PrHexByte(packet.id);
					LCD_PrCString(" ");
					LCD_PrHexByte(packet.command);					
					LCD_Position(0,0);
					LCD_PrString(buf);
				}
				packetCount++;
				radio_state = RADIO_SEND;
				break;
			}
			
			case RADIO_SEND :
			{
				while(sendPacket(&packet) != PACKET_SENT)
					;
				if(packet.command == COMMAND_SLEEP) {
					packet_state = clearPacket(&packet);
					radio_state = RADIO_STANDBY;
				} else if(packet.command == COMMAND_DATA) {
					packet_state = clearPacket(&packet);
					radio_state = RADIO_STOP;
				}
				
				break;
			}
			
			case RADIO_STANDBY :
			{
				Timer8_Start();	
				nrf24PowerDown();
				while(timeMS < timeToWait)
					;
				Timer8_Stop();
				nrf24PowerUP();	
				radio_state = RADIO_BUILD_PACKET;
				break;
			}
			
			case RADIO_STOP :
			{
				nrf24PowerDown();
				radio_stop = 1;
				break;
			}
			
			default:
				break;
		}
		
	} while(!radio_stop);
	
	while(1)
		;
}