#include "nrf24l01p.h"
#include "radio.h"
#include "delay.h"

extern BYTE RxAddrP0[5] = 	{0x01, 0x01, 0x01, 0x01, 0x01};
extern BYTE TxAddr[5] = 	{0x01, 0x01, 0x01, 0x01, 0x01};
extern volatile BYTE IRQstatus = 0;


//power down the radio. reading and writing registers still works.
//if in rx mode, goes to standby first and then powers down
void nrf24PowerDown(void) {
	BYTE currentConfig;
	currentConfig = nrf24GetRegister(CONFIG);
	
	if(currentConfig & PRIM_RX) {
		CE_Off();
		Delay50uTimes(3);
	}
	
	currentConfig &= ~PWR_UP;
	nrf24SetRegister(CONFIG, currentConfig);
}

//from RX mode to standby
//tx mode is this by default except when sending
void nrf24StandBy(void) {
	CE_Off();
	Delay50uTimes(3);
}

//powers up the radio. in tx mode will remain in standby.
void nrf24PowerUP(void) {
	BYTE currentConfig;
	currentConfig = nrf24GetRegister(CONFIG);
	
	if(!(currentConfig & PWR_UP)) {
		currentConfig |= PWR_UP;
		nrf24SetRegister(CONFIG, currentConfig);
		Delay50uTimes(35);
	}
	
	if(currentConfig & PRIM_RX) {
		CE_On();
		Delay50uTimes(3);
	}
}

//configure all essential registers
void nrf24Configure(void) {
	//set address length to 5 bytes
	nrf24SetRegister(SETUP_AW, (ADDRESS_LENGTH - 2));

	//set autoretransmit delay as specified in the header
	//set autoretransmit count as specified in the header
	nrf24SetRegister(SETUP_RETR, (ARD | ARC));
	
	// enable only data pipe 0
	nrf24SetRegister(EN_RXADDR, ERX_P0);
	
	// enable autoack on pipe 0
	nrf24SetRegister(EN_AA, ENAA_P0);
	
	// set CRC to 2 bytes and enable interrupts on IRQpin
	nrf24SetRegister(CONFIG, (EN_CRC | CRC0));
	
	// set data rate to 1Mbps and rf output power to 00 dbm
	nrf24SetRegister(RF_SETUP, 0x06);
	
	// set the RX_ADDR_P0, 5 bytes
	nrf24SetMultiRegister(RX_ADDR_P0, RxAddrP0, ADDRESS_LENGTH);
	
	// set the TX_ADDR, 5 bytes
	nrf24SetMultiRegister(TX_ADDR, TxAddr, ADDRESS_LENGTH);

	// set RX_PW_P0 to 32 bytes 
	nrf24SetRegister(RX_PW_P0, PACKET_SIZE);	
}

void nrf24SetMode(BYTE mode) {
	BYTE currentConfig;
	
	CE_Off();
	
	if (mode == MODE_TX) {
		currentConfig = nrf24GetRegister(CONFIG);
		currentConfig &= ~PRIM_RX; //PRIM_RX = 0
		currentConfig |= PWR_UP; //PWR_UP = 1
		nrf24SetRegister(CONFIG, currentConfig);
		Delay50uTimes(30);
	} else if (mode == MODE_RX) {
		currentConfig = nrf24GetRegister(CONFIG);
		currentConfig |= (PRIM_RX | PWR_UP); //PRIM_RX = 1, PWR_UP = 1
		nrf24SetRegister(CONFIG, currentConfig);
		Delay50uTimes(30);
		CE_On();
		Delay50uTimes(3);
	}
}

BYTE nrf24GetStatus(void) {
	BYTE status = 0;
	
	CS_Off();
	
	while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
		SPIM_SendTxData(NOP); 
	
	while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
		status = SPIM_bReadRxData();
	
	CS_On();
	
	return status;
}

void nrf24ClearStatus(BYTE mask) {
	BYTE status;
	
	CS_Off();
	
	while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
		SPIM_SendTxData(NOP); 
	
	while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
		status = SPIM_bReadRxData();
	
	CS_On();
	
	status |= mask;
	
	nrf24SetRegister(STATUS, status);
}

BYTE nrf24GetRegister(BYTE address) {
	BYTE status = 0;
	
	CS_Off();
	
	while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
		SPIM_SendTxData(R_REGISTER | address);
	
	while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
		SPIM_bReadRxData();
		
	while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
		SPIM_SendTxData(DUMMYDATA);
		
	while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
		status = SPIM_bReadRxData();
		
	CS_On();
	
	return status;
}

void nrf24SetRegister(BYTE address, BYTE data) {
	BYTE status = 0;
	
	CS_Off();
	
	while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
		SPIM_SendTxData(W_REGISTER | address);
	
	while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
		SPIM_bReadRxData();
		
	while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
		SPIM_SendTxData(data);
		
	while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
		status = SPIM_bReadRxData();
		
	CS_On();
}

void nrf24SetMultiRegister(BYTE address, BYTE *data, BYTE bytes) {
	BYTE i;
	
	CS_Off(); 
	
	while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
		SPIM_SendTxData(W_REGISTER | address);
	
	while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
		SPIM_bReadRxData();
		
	for(i = 0; i < bytes; i++) {

		while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
			SPIM_SendTxData(data[i]);	
		
		while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
			SPIM_bReadRxData();			
	}
	
	CS_On();
}

BYTE nrf24SendCommand(BYTE command) {
	BYTE status = 0;
	
	CS_Off();
	
	while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
		SPIM_SendTxData(command);
	
	while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
		status = SPIM_bReadRxData();
		
	CS_On();
	
	return(status);
}

void nrf24SendMultiCommand(BYTE command, BYTE *data, BYTE bytes) {
	BYTE i;
	
	CS_Off(); 
	
	while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
		SPIM_SendTxData(command);
	
	while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
		SPIM_bReadRxData();
		
	for(i = 0; i < bytes; i++) {

		while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
			SPIM_SendTxData(data[i]);	
		
		while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
			SPIM_bReadRxData();			
	}
	
	CS_On();
}

void nrf24ReadPayload(BYTE *buffer) {
	BYTE i;
	
	CE_Off();
	Delay50uTimes(3);
	CS_Off();
	
	while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
		SPIM_SendTxData(R_RX_PAYLOAD);
		
	while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL); 
		buffer[0] = SPIM_bReadRxData(); 
		
	for(i = 0; i < PACKET_SIZE; i++) {

		while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
		SPIM_SendTxData( DUMMYDATA );
		
		while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL); 
		buffer[i] = SPIM_bReadRxData();  
			
	}
	
	CS_On();	
	CE_On();
	Delay50uTimes(3);
}

void nrf24SendPayload(BYTE *buffer) {
	BYTE i;
	
	CS_Off(); 
	
	while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
		SPIM_SendTxData(W_TX_PAYLOAD);
	
	while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
		SPIM_bReadRxData();
		
	for(i = 0; i < PACKET_SIZE; i++) {

		while(!(SPIM_bReadStatus() & SPIM_SPIM_TX_BUFFER_EMPTY));  
			SPIM_SendTxData(buffer[i]);	

		while(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL);
			SPIM_bReadRxData();			
	}
	
	CS_On();
}

void nrf24Transmit(void) {
	CE_On();
	Delay50u();
	CE_Off();
}