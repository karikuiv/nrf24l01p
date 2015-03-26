#ifndef __RADIO_H_
#define __RADIO_H_

typedef struct {
	BYTE id;
	//add more configs like pipe addresses and stuff
} Radio;

typedef enum _radio_command {
	COMMAND_DATA =	0x00,
	COMMAND_SLEEP = 0x11,
	COMMAND_STANDBY = 0x22,
	COMMAND_CONFIG = 0x33,
} RADIO_COMMAND;

typedef enum  _radio_state {
	RADIO_INIT,
	RADIO_LISTEN,
	RADIO_SEND,
	RADIO_SLEEP,
	RADIO_STANDBY,
	RADIO_STOP,
	RADIO_EXEC_COMMAND,
	RADIO_BUILD_PACKET,
	RADIO_PARSE_PACKET,
	PRINT_DATA,
} RADIO_STATE;

typedef enum _packet_state {
	PACKET_LOST,
	PACKET_SENT,
	PACKET_INVALID,
	PACKET_DATA,
	PACKET_COMMAND,
	PACKET_CLEAR,
	PACKET_WRONG_ID,
} PACKET_STATE;

typedef struct {
	BYTE id;
	BYTE address[5];
	BYTE timestamp;
	BYTE command;
	BYTE payloadSize;
	BYTE payload[23];
} Packet;

PACKET_STATE clearPacket(Packet *packet);
RADIO_STATE parsePacket(BYTE command);
void printPacket(BYTE *packet, BYTE start, BYTE len, BYTE type);
BYTE enqueueData(Packet *packet, void *data, BYTE len);

void waitForPacket(Packet *packet);
PACKET_STATE sendPacket(Packet *packet);

#endif