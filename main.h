/*************************************************************************
 *********** COPYRIGHT (c) 2018 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: init.h
 *
 *  Module:    Control
 *
 *  Summary:   This is the header file for main.
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    05-10-13 jmh:  creation date
 *    07-13-13 jmh:  removed typecast from timer defines & updates XTAL freq 
 *
 *******************************************************************/

#include "typedef.h"

#ifndef INIT_H
#define INIT_H
#endif

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

#define	NUM_CHAN	16		// define number of PLL channels for this build

// timer definitions.  Uses EXTXTAL #def to select between ext crystal and int osc
//  for normal mode.
// SYSCLK value in Hz
#define SYSCLKL 10000L
#ifdef EXTXTAL
#define SYSCLKF 20000000L
#else
#define SYSCLKF (12000000L) // / 8)
#endif
// timer2 register value
#define TMR2RLL_VAL (U8)((65536 -(SYSCLK/(12L * 1000L))) & 0xff)
#define TMR2RLH_VAL (U8)((65536 -(SYSCLK/(12L * 1000L))) >> 8)
#define	TMRSECHACK	2
#define TMRIPL		1
#define TMRRUN		0

#define MS_PER_TIC  1
// General timer constants
#define MS50        	(50/MS_PER_TIC)
#define MS100       	(100/MS_PER_TIC)
#define MS250       	(250/MS_PER_TIC)
#define MS400       	(400/MS_PER_TIC)
#define MS450       	(450/MS_PER_TIC)
#define MS500       	(500/MS_PER_TIC)
#define MS750       	(750/MS_PER_TIC)
#define MS1000      	(1000/MS_PER_TIC)
#define MS2000      	(2000/MS_PER_TIC)
#define MS5000      	(5000/MS_PER_TIC)
#define MS10000     	(10000/MS_PER_TIC)
#define MS20000     	(20000/MS_PER_TIC)
#define MINPERHOUR		60
#define SECPERMIN		60
#define MINPER6HOUR		(6 * MINPERHOUR)
#define MINPER12HOUR	(12 * MINPERHOUR)
#define MINPER18HOUR	(18 * MINPERHOUR)
#define MINPER24HOUR	(24 * MINPERHOUR)
#define MSG_DLY			9000					// default message pause time

// CW Timing parameters
// WPM = 2.4 * (Dots per second)
// DIT_TIME (SEC/DOT) = 1.2/WPM, DIT repetition rate = 2.4/WPM
#define	CW_WPM		20				// integer WPM value
#define	DIT_TIME	((((1000*12)/(10*CW_WPM))/MS_PER_TIC)) // { ((((1000 ms/s * (2.4/2) s/wpm)/CW_WPM wpm))/Xms/tic) } (tic)
#define	DAH_TIME	(DIT_TIME * 3)
#define	SPACE_TIME	(DIT_TIME * 7)
#define	COMMA_TIME	(DIT_TIME * 14)
#define	CARRET_TIME	200				// 2 seconds for the carret timer
#define	CW_STOP		0x18			// this is considered as an illegal CW construct as it specifies neither a legal
									// dit nor dah.  Thus, this is used as an embedded command semaphore
#define	CW_STOPW	0x18FF			// The EOM (stop) embedded command semaphore
#define	CW_EOM		0xff			// = end of message
#define	CW_IOP		0xe0			// = add 1 to I/O bits 4-6
#define	CW_IOM		0xd0			// = subtract 1 from I/O bits 4-6
#define	CW_IOSET	0xa0			// = set I/O bits from lower bits of param
#define	CW_CHSET	0xc0			// = set new channel
#define	CW_CHADD	0xb0			// = add low bits to current channel #
#define	CW_CHCLR	0x90			// = clear delta ch to zero if delta > lower nybble
#define	CW_IOMASK	0xf8			// mask for I/O set cmd. (data & CW_IOMASK) == 0 to trap valid I/O bits in [2:0]

//PBSW mode defines
#define PB_POR      0xFF
#define PB_NORM     0
#define UCPC        1

// CLI commands
#define	CLI_NULL	0x00
#define	CLI_LOGHDR	0x01
#define	CLI_SURV	0x02

// DAC commands
#define	DAC_IREF	0x60
#define	DAC_VREF	0x70
#define	DAC_SET		0x30

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

#ifndef IS_MAINC
extern U8 spi_tmr;
#endif

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

void Init_Device(void);
void wait(U16 waitms);

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
