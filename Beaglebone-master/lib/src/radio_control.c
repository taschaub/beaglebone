/**
 * @file radio_control.c
 * @brief Steuerung des drahtlosen Kanals
 *
 * Der CC1200 befindet sich in mehrere Zuständen zwischen denen hin und her geschaltet
 * wird. Dieses Modul veranlasst das Wechseln der Zustände.
  *
 * @date 05.06.2015 - Erste Implementierung
 * @date 14.10.2015 - Aktualisieren der Dokumentation
 * @date 03.02.2016 - Funktion RxFifoFlush und TxFifoFlush eingefuehrt
 *
 * @version 0.1
 **/

#include <stdio.h>
#include "cc1200.h"

/**
 * @brief Die maximale Anzahl der Versuche den Zustand des CC1200 zu wechseln.
 *
 * Nachdem der CC1200 angewiesen wurde seinen Zustand zu ändern, wird überprüft, ob der
 * Zustand wirklich gewechselt wurde. Die maximale Anzahl an Überprüfungen wird durch die
 * Konstante MAX_CNT festgelegt.
 **/
#define MAX_CNT 50

/**
 * @brief Die Funktion Reset_CC1200 bringt den Transceiver in seinen Initial-Zustand
 *
 * @date 05.06.2015 - Erste Implementierung
 *
 * @version 0.1
 * @author Berthold Rathke
 **/
void Reset_CC1200 (void) {cc1200_cmd(SRES);}
/****************************************************************************************/

/**
 * @brief Den Zustand des CC1200 ermitteln
 *
 * Der Zustand des Chips wird über den SPI Bus bereitgestellt.
 *
 * @return  CC1200_CHIP_STATES Zustand des CC1200
 *
 * @date 05.06.2015 - Erste Implementierung
 *
 * @version 0.1
 * @author Berthold Rathke
 **/
CC1200_STATES Get_Status (void){cc1200_cmd(SNOP); return get_status_cc1200();}
/****************************************************************************************/

/**
 * @brief Loeschen und Zuruecksetzen des RX Fifo's
 *
 * @date 03.02.2016 - Erste Implementierung
 *
 * @version 0.1
 * @author Berthold Rathke
 */
void RxFifoFlush (void) {

	int state=Get_Status();

	if (state==IDLE || state==RX_FIFO_ERROR)
		cc1200_cmd(SFRX);
}
/****************************************************************************************/

/**
 * @brief Loeschen und Zuruecksetzen des TX Fifo's
 *
 * @date 03.02.2016 - Erste Implementierung
 *
 * @version 0.1
 * @author Berthold Rathke
 */
void TxFifoFlush (void) {

	int state=Get_Status();

	if (state==IDLE || state==TX_FIFO_ERROR)
		cc1200_cmd(SFTX);
}
/****************************************************************************************/

/**
 * @brief In den Empfangszustand schalten
 *
 * Die Funktion versucht von einem beliebigen Zustand aus in den Empfangsmodus zu 
 * schalten. Da dies abhängig vom augenblicklichen Zustand ist, wird dieser als
 * Parameter an die Funktion übergeben.
 *
 * @param  state Zustand in dem die der CC1200 momentan befinndet.
 * @return int Wenn in den Empfangszustand gewechselt wurde 1, sonst 0.
 *
 * @todo Um den Parameter state zu sparen, kann der Zustand des Chips in der Funktion
 *       ermittelt werden.
 *
 * @date 05.06.2015 - Erste Implementierung
 *
 * @version 0.1
 * @author Berthold Rathke
 **/ 
int SwitchToRx (CC1200_STATES state) {

	int cnt=0;
  
	switch (state) {
		case IDLE:
			cc1200_cmd(SRX);
			for (cnt=0; (state!=RX) && (cnt<MAX_CNT); cnt++) state=Get_Status();
			break;
		case RX:
			printf ("WARNING: already in RX state\n");
            break;
		default:
			printf ("dont know how zu switch from state %s to RX state\n", 
			        print_status(state));
	}
	if (cnt >= MAX_CNT) printf("ERROR: Unable to go to RX state (max retry error\n");
	#ifdef DEBUG_RADIOCONTROL
		if (state==RX) printf ("SwitchToRX ist sucessfull\n");
        else{
			printf ("SwitchToRX: ERROR state is ");
			printf (print_status(state));
			printf ("\n");
		}
	#endif
	return Get_Status()==RX;
}
/****************************************************************************************/
