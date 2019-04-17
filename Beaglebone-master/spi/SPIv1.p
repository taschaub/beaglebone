.setcallreg r25.w0
.origin 0
.entrypoint START
#include "SPI.hp"
//#define CONST_PRUDRAM C24
#define CONST_INTC C0
#define CONST_IEP C26
#define SICR 24
//#define PRU0_ARM_INTERRUPT 19
#define CMP_CFG 0x40
#define GLOBAL_CFG 0x00
#define GLOBAL_STATUS 0x04
#define CMP_STATUS 0x44
#define CMP0 0x48
#define CMP1 0x4C	
#define COUNT 0xC 	
#define COMPEN 0x8


START:

	    // Enable OCP master port
	    LBCO r0, CONST_PRUCFG, 4, 4
	    CLR  r0, r0, 4
	    SBCO r0, CONST_PRUCFG, 4, 4

	    //C28 will point to 0x00012000 (PRU shared RAM)
	    MOV  r0, 0x00000120
	    MOV  r1, CTPPR_0
	    ST32 r0, r1

	    // Enable CLKSPIREF and CLK
	    // Checked, is indeed correct in Clock Register p1179 TRM
	    // TODO You got to do this too with SPI0 if you
	    // intend to use it
	    MOV  r1, 0x44E0004C
	    MOV  r2,  0x00000002
	    SBBO r2,  r1, 0, 4

            // Reset CC1200 HIGH
            SET  r30, 3
INTC_INIT:
	//Set Parity (active low/high) on all events to high
	// SIPR0 offset = 0xD00
	MOV   r0, 0xD00
	MOV   r1, 0xFFFFFFFF
	SBCO  r1, CONST_INTC, r0, 4
	// SIPR1 offset = 0xD04
	MOV   r1, 0xFFFFFFFF
	SBCO  r1, CONST_INTC, r0, 4
	// All interrupts shall be of type pulse (SITR0/1) to zero
	MOV   r0, 0xD80
	MOV   r1, 0x0
	SBCO  r1, CONST_INTC, r0, 4
	MOV   r0, 0xD84
	SBCO  r1, CONST_INTC, r0, 4
	//map all system events to channels
	//All unused system events to channel 9
	MOV    r0, 0x400
	MOV    r1, 0x09090909
	SBCO   r1, CONST_INTC, r0, 4
	MOV    r0, 0x408
	SBCO   r1, CONST_INTC, r0, 4 
	MOV    r0, 0x40C
	SBCO   r1, CONST_INTC, r0, 4
	//All except system event 7 (which is channel 0)
	//Actually there are more which were set by linux
	//But we don't touch those	
	MOV    r0, 0x404
	MOV    r1, 0x00090909
	SBCO   r1, CONST_INTC, r0, 4
	//Now the host interrupts
	//Don't touch the ones used by linux (1-3) HMR0
	//All other channels are now host interrupt 9
	MOV    r0, 0x800
	//TODO Channel 0 Host Interrupt 0
	LBCO   r1, CONST_INTC, r0, 4
	MOV    r2, 0xFFFFFF00
	AND    r1, r1, r2
	SBCO   r1, CONST_INTC, r0, 4
	MOV    r1, 0x09090909
	MOV    r0, 0x804
	SBCO   r1, CONST_INTC, r0, 4
	MOV    r0, 0x808
	MOV    r1, 0x00000909
	SBCO   r1, CONST_INTC, r0, 4
	CALL   RESET
	//Turn on Global Interrupts
	MOV    r0, 0x10
	MOV    r1, 0x1
	SBCO   r1, CONST_INTC, r0, 4
	//Turn on Host Interrupts 0-3 
	MOV    r0, 0x1500
	MOV    r1, 0xF
	SBCO   r1, CONST_INTC, r0, 4
	CALL   ALL_EVENTS

	    // Reset SPI
	    MOV  r1, 0x48030110
	    LBBO r2,  r1, 0, 4
	    SET  r2.t1
	    SBBO r2,  r1, 0, 4

	    //Wait for RESET
SPI_RESET:
	    MOV  r1,  0x48030114
	    LBBO r2,   r1, 0, 4
	    QBBC SPI_RESET, r2.t0

	    //Config MODULCTRL
	    MOV  r1, 0x48030128
            // Change 1 << 0 Singlechannel > Multichannel (0 << 0)
	    //  single channel mode 
	    MOV  r2,  0x00000001
	    SBBO r2,  r1 , 0, 4

	    //Config SYSCONFIG
	    MOV  r1, 0x48030110
	    // Smart Idle, OCP functional clock maintained
	    MOV  r2,  0x00000311
	    SBBO r2,  r1, 0, 4
	    LBBO r5,  r1, 0, 4

	    //EDIT: XFER_LEVEL
	    MOV  r1, 0x4803017C
	    MOV  r2, 0x00000000
	    SBBO r2, r1, 0, 4



MAIN_LOOP:
	// Wait for ARM to signal transfer
        // SHARED RAM OFFSET Where we write received data
        MOV  r8, 20
	WBS r31, 30
	CALL ALL_EVENTS
        CALL ARM_EVENT
REPEAT_TEST_TEST_1:
	    //Reset interrupt status bits
	    MOV  r1, 0x48030118
	    MOV  r2,  0xFFFFFFFF
	    SBBO r2,  r1, 0, 4

	    //Disable interupts
	    //EDIT: Enable TX1_EMPTY
	    // EDIT EDIT: No need, we can poll the interrupt anyway
	    MOV  r1, 0x4803011C
	    MOV  r2,  0x00000000
	    SBBO r2,  r1, 0, 4

	    // Disable channel 0
	    MOV  r1, 0x48030134
	    MOV  r2,  0x00000000
	    SBBO r2,  r1, 0, 4
	    // Configure channel 0 of MCSPI0
	    MOV  r1, 0x4803012C
	    // Change -> 8 FIFO Write
	    // Change -> 1 << 28 FIFO read
	// Change -> 0 << 13-12 T&R (instead of 10 Transmit only)
	// Change -> 7 <- WL (8 Bit word length)
	    // In Transmit only this works
	    // In T&R mode it does not :/
	    // .. Buffer size seems to be the problem
	    // Like, writing only 8*4 Bytes into a 64 Byte buffer at a time no problemo
	    // Writing 7 into a 32 Byte buffer is more problematic
	    //MOV  r2, 0x181903C0
            MOV r2, 0x001903CC
	    SBBO r2, r1, 0, 4


	    // Bug fix Channel 1 two words swallowed up
	    MOV  r1, 0x48030138
	    MOV  r6, 0xAAAAAAAA
	    SBBO r6, r1, 0, 4
	    SBBO r6, r1, 0, 4
	 
REPEAT_TEST:
	    LBCO r10, CONST_PRUSHAREDRAM, 0, 4 //word #1
            LBCO r11, CONST_PRUSHAREDRAM, 4, 4 //word #2
            LBCO r12, CONST_PRUSHAREDRAM, 8, 4 //word #3
            LBCO r13, CONST_PRUSHAREDRAM, 12, 4 // Num words
            // Number of times to transmit backup

	// Write DAC command to SPI_TX1
	  // Enable CHannel 1
	  MOV  r1, 0x48030134
	  MOV  r2, 0x00000001
	  SBBO r2, r1, 0, 4
          // CALL CS_ON
          JMP POLLING_TX_RX
	// ******************** BEGIN ACQUISITION ********************
RUN_AQ:
	  //Write data to FIFO
	  MOV  r1, 0x48030138
	  // Get maximum out of 16 and num times to transmit
	  // Thus ensuring that we write at most 16 Words into the buffer at one time
	  // 8 Byte instead of 16 due to using FIFO R&W
	  //MOV  r10, 0xB4
	  MIN  r6, r3, 8
BUF_FILL_START:
	  QBEQ BUF_FILL_END, r6, 0
	  SBBO r10, r1, 0, 4
	  SUB  r3, r3, 1
	  SUB  r6, r6, 1
	  JMP BUF_FILL_START
BUF_FILL_END:

CHECKTX1:
	  MOV  r1, 0x48030130
	  //EDIT: Poll IRQ_STATUS REGISTER AEL for channel 1 instead
	  //MOV  r1, 0x481A0118
CHECKTX1_1:
	  LBBO r2, r1, 0, 4
	  // Check FIFO empty is this:
	  QBBC CHECKTX1_1, r2.t3
          // Change We wait for EOT before checking RX register
	LBBO r2, r1, 0, 4
	QBBC CHECKTX1_1, r2.t2

	  // Drain RX FIFO
	  // RX BUFFER register
	  MOV  r4, 0x4803013C
CHECKRX1_1:
	  LBBO r2, r1, 0, 4
	QBBC CHECKRX1_1, r2.t2
        SUB r7, r7, 1
	LBBO r2, r4, 0, 4
	// Save Read Word
        //QBNE RX_SIGNAL_ZERO, r2, 0
        //MOV  r2, 0xAAAAAAAA
// RX_SIGNAL_ZERO:
REPEAT_UNTIL_FF_EMPTY:
	SBCO r2, CONST_PRUSHAREDRAM, r8, 4
	ADD  r8, r8, 4
        LBBO r2, r1, 0, 4
        QBBC REPEAT_UNTIL_FF_EMPTY, r2.t5
        //JMP CHECKRX1_1
        QBNE CHECKRX1_1, r7, 0
CHECKRX1_1_END:
POLLING_TX_RX:
          MOV r1, 0x48030138
          SBBO r10, r1, 0, 4
          MOV r2, 0x48030130
          MOV r4, 0x4803013C
POLL_RX_LOOP_1:
          LBBO r3, r2, 0, 4
//          JMP EXIT
          QBBC POLL_RX_LOOP_1, r3.t0
          LBBO r3, r4, 0, 4
          //QBBC POLL_RX_LOOP_1, r3.t1
          SBCO r3, CONST_PRUSHAREDRAM, r8, 4
          ADD r8, r8, 4

          QBGT PRE_EXIT, r13, 2
          SBBO r11, r1, 0, 4
POLL_RX_LOOP_2:
          LBBO r3, r2, 0, 4
          QBBC POLL_RX_LOOP_2, r3.t0
          LBBO r3, r4, 0, 4
          SBCO r3, CONST_PRUSHAREDRAM, r8, 4
          ADD r8, r8, 4

          QBGT PRE_EXIT, r13, 3
          SBBO r12, r1, 0, 4
POLL_RX_LOOP_3:
          LBBO r3, r2, 0, 4
          QBBC POLL_RX_LOOP_3, r3.t0
          LBBO r3, r4, 0, 4
          SBCO r3, CONST_PRUSHAREDRAM, r8, 4
          ADD r8, r8, 4

PRE_EXIT:
	//EDIT Clear interrupt flag in IRQ_STATUS register, TX1_empty -> AEL
	  MOV  r1, 0x48030118
	//  MOV  r2, 0x00000008
	  MOV  r2, 0xFFFFFFFF
	  SBBO r2, r1, 0, 4
	  //QBEQ EXIT, r3, 0
	  //JMP RUN_AQ
          JMP EXIT
	//////////////////////////////////////

EXIT:
          // Test without sending anything
          // Wait a few cycles
	  //Disable channel 1
CHECKTX1_2:
	MOV  r1, 0x48030130
	LBBO r2, r1, 0, 4
        // Disable channel 0
	MOV  r1, 0x48030134
	MOV  r2,  0x00000000
	SBBO r2,  r1, 0 ,4


        CALL CS_OFF

	// Write Number of written words to Shared RAM to SHARED_RAM+16
        // [Number of written words] = ([Address_now] - [Address_start]) / 4
	//SUB  r8, r8, 20
	//LSR  r8, r8, 1
	SBCO r8, CONST_PRUSHAREDRAM, 16, 4

	    // Configure channel 1 of MCSPI1
	    // Change -> 8 FIFO Write
	    // Change -> 1 << 28 FIFO read
	// Change -> 0 << 13-12 T&R (instead of 10 Transmit only)
	// Change -> 7 <- WL (8 Bit word length)
	    // In Transmit only this works
	    // In T&R mode it does not :/
	    // .. Buffer size seems to be the problem
	    // Like, writing only 8*4 Bytes into a 64 Byte buffer at a time no problemo
	    // Writing 7 into a 32 Byte buffer is more problematic
	MOV R31.b0, PRU0_ARM_INTERRUPT+16
	JMP MAIN_LOOP
	HALT

        // Toggles SPIEN (Chip select) of SPI0
CS_ON:
        MOV  r0, 0x00100000
        MOV  r1, 0x4803012C
        LBBO r2, r1, 0, 4
        OR  r2, r2, r0
        SBBO r2, r1, 0, 4
        RET
CS_OFF:
        MOV  r0, 0xFFEFFFFF
        MOV  r1, 0x4803012C
        LBBO r2, r1, 0, 4
        AND  r2, r2, r0
        SBBO r2, r1, 0, 4
        RET

	//Clears and resets all system events (currently only event 7)
ALL_EVENTS:
	MOV    r0, 0x7
	SBCO   r0, CONST_INTC, 0x24, 4
	SBCO   r0, CONST_INTC, 0x28, 4
	RET
//Resets everything regarding intc and iep
RESET:
	MOV    r1, r25
	CALL   ALL_EVENTS
	MOV    r25, r1
	RET
	//Clears and reenables system event 3 (hopefully the arm interrupt)
ARM_EVENT:
	MOV    r0, 21
	SBCO   r0, CONST_INTC, 0x24, 4
	SBCO   r0, CONST_INTC, 0x28, 4
	RET
	
