#ifndef CC1200_D
#define CC1200_D

#define REGISTER_WRITE 1
#define REGISTER_READ  2
#define SPI_INIT       3 
#define CC1200_CMD     4
#define CC1200_STATE   5
#define RSSI		   6

typedef struct {
	unsigned short cmd;
	unsigned short adr;
	unsigned short val;
} REG_CMD;

typedef struct {
	unsigned short cmd;
	unsigned short command;
} CC1200_COMMAND;

typedef struct {
	unsigned short cmd;
	unsigned short state;
} CC1200_STATUS;

typedef struct {
	unsigned short cmd;
	unsigned short on;
} CC1200_RSSI;
#endif
