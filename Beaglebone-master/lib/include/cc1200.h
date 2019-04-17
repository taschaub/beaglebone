/**
 * @file radio_control.h
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

#ifndef RADIO_CONTROL_H
#define SRADIO_CONTROL_H

#include <SPIv1.h>

/**
 * @brief Die Testfunktion für das radio control Modul einschalten.
 **/
#define DEBUG_RADIOCONTROL

/**
 * @brief Die Funktion Reset_CC1200 bringt den Transceiver in seinen Initial-Zustand
 *
 * @date 05.06.2015 - Erste Implementierung
 *
 * @version 0.1
 * @author Berthold Rathke
 **/
extern void Reset_CC1200 (void);
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
extern CC1200_STATES Get_Status (void);
/****************************************************************************************/

/**
 * @brief Loeschen und Zuruecksetzen des RX Fifo's
 *
 * @date 03.02.2016 - Erste Implementierung
 *
 * @version 0.1
 * @author Berthold Rathke
 */
extern void RxFifoFlush (void);
/****************************************************************************************/

/**
 * @brief Loeschen und Zuruecksetzen des TX Fifo's
 *
 * @date 03.02.2016 - Erste Implementierung
 *
 * @version 0.1
 * @author Berthold Rathke
 */
extern void TxFifoFlush (void);
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
extern int SwitchToRx (CC1200_STATES state);
/****************************************************************************************/
#endif
