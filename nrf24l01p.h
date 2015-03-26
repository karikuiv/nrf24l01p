#ifndef __NRF24L01P_H_
#define __NRF24L01P_H_

#include <m8c.h>
#include "PSoCAPI.h"

//The length of the address. 5, 4 or 3 bytes
#define ADDRESS_LENGTH 5
#define PACKET_SIZE 32

void nrf24PowerDown(void);
void nrf24StandBy(void);
void nrf24PowerUP(void);

void nrf24Configure(void);
void nrf24SetMode(BYTE mode);

BYTE nrf24GetStatus(void);
void nrf24ClearStatus(BYTE mask);

BYTE nrf24GetRegister(BYTE address);
void nrf24SetRegister(BYTE address, BYTE data);
void nrf24SetMultiRegister(BYTE address, BYTE *data, BYTE bytes);

BYTE nrf24SendCommand(BYTE command);
void nrf24SendMultiCommand(BYTE command, BYTE *data, BYTE num);

void nrf24ReadPayload(BYTE *packet);
void nrf24SendPayload(BYTE *packet);
void nrf24Transmit(void);

// SPI commands of the nRF24L01
#define R_REGISTER		0x00
#define W_REGISTER		0x20
#define R_RX_PAYLOAD	0x61
#define W_TX_PAYLOAD	0xA0
#define FLUSH_TX		0xE1
#define FLUSH_RX		0xE2
#define REUSE_TX_PL		0xE3
#define NOP				0xFF


// Register addresses
#define CONFIG			0x00
#define EN_AA			0x01
#define EN_RXADDR		0x02
#define SETUP_AW		0x03
#define SETUP_RETR		0x04
#define RF_CH			0x05
#define RF_SETUP		0x06
#define STATUS			0x07
#define OBSERVE_TX		0x08
#define CD				0x09
#define RX_ADDR_P0		0x0A
#define RX_ADDR_P1		0x0B
#define RX_ADDR_P2		0x0C
#define RX_ADDR_P3		0x0D
#define RX_ADDR_P4		0x0E
#define RX_ADDR_P5		0x0F
#define TX_ADDR			0x10
#define RX_PW_P0		0x11
#define RX_PW_P1		0x12
#define RX_PW_P2		0x13
#define RX_PW_P3		0x14
#define RX_PW_P4		0x15
#define RX_PW_P5		0x16
#define FIFO_STATUS		0x17

// CONFIG register
#define MASK_RX_DR		0x40 // 0 enable 1 disable interrupt
#define MASK_TX_DS		0x20 // 0 enable 1 disable interrupt
#define MASK_MAX_RT		0x10 // 0 enable 1 disable interrupt
#define EN_CRC			0x08 // 0 disable 1 enable automatic CRC
#define CRC0			0x04 // 0 one byte 1 two byte crc
#define PWR_UP			0x02 // 0 power down 1 power up radio
#define PRIM_RX			0x01 // 0 transmitter 1 receiver

// EN_AA register
// 0 disable 1 enable enhanced shockburst
#define ENAA_P5			0x20
#define ENAA_P4			0x10
#define ENAA_P3			0x08
#define ENAA_P2			0x04
#define ENAA_P1			0x02
#define ENAA_P0			0x01

// EN_RXADDR register
// 1 enable 0 disable rx pipe
#define ERX_P5			0x20
#define ERX_P4			0x10
#define ERX_P3			0x08
#define ERX_P2			0x04
#define ERX_P1			0x02
#define ERX_P0			0x01

// SETUP_AW register
// address width is: 01 - three bytes
// 	10 - four bytes, 11 - five bytes
#define AW				0x11

// SETUP_RETR register
// Autoretransmit delay (bits 7:4)
// 0000 - 250+86 us
// 0001 - 500+86 us
// 1111 - 4000+86 us
#define ARD				0x10
// Autoretransmit count (bits 3:0)
// 0000 - retransmit disabled
// 1111 - Up to 15 retransmits
#define ARC				0x05

// STATUS register
#define RX_DR			0x40
#define TX_DS			0x20
#define MAX_RT			0x10
#define RX_P_NO			0x02
#define TX_FULL			0x01

// OBSERVE_TX register
// lost packet count (bits 7:4)
// counts up to 15 lost packets. does not overflow.
// reset by writing to RF_CH
#define PLOS_CNT		0x10
// resent packet count (bits 3:0)
// counts the packets that were resent.
// reset by sending a new packet
#define ARC_CNT			0x01

// FIFO_STATUs register
#define TX_REUSE		0x40 // 0 dont keep 1 keep on resending tx payload
#define TX_FIFO_FULL	0x20 // 0 tx fifo not full 1 full
#define TX_FIFO_EMPTY	0x10 // 0 tx fifo not empty 1 empty
#define RX_FIFO_FULL	0x02 // 0 rx fifo not full 1 full
#define RX_FIFO_EMPTY	0x01 // 0 rx fifo not empty 1 empty

// general defines
#define DUMMYDATA		0x00
#define MODE_RX			0x01
#define MODE_TX			0x10

#endif