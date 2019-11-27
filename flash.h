/****************************************************************************************
 ****************** COPYRIGHT (c) 2017 by Joseph Haas (DBA FF Systems)  *****************
 *
 *  File name: flash.h
 *
 *  Module:    Control
 *
 *  Summary:   This file contains function prototypes for flash functions
 *
 *  File scope revision history:
 *    08-11-16 jmh:  Rev 0.0:
 *                   adapted from F120 version
 *
 ***************************************************************************************/

#include "typedef.h"

// extern defines
#ifndef FLASH_INCL

#endif

//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------

void init_flash(void);
U8 erase_flash(U8 xdata * addr);
void wr_flash(char byte, U8 xdata * addr);

//------------------------------------------------------------------------------
// global defines
//------------------------------------------------------------------------------

#define FLRT 0x30	// for 75MHz < sysclk < =100MHz
//FLSCL:
#define FLWE 0x01
//PSCTL:
#define SFLE 0x04
#define PSEE 0x02
#define PSWE 0x01
//RSTSRC
#define PORSF 0x02

#define	SECTOR_SIZE	512
#define	FLASH_END	0x1dfe								// 0x1dff is lock byte, no touchy!!!!
#define	SECT00_ADDR	0x0000								// start of FLASH
#define	SECTCH_ADDR	(SECT00_ADDR + (11 * SECTOR_SIZE))	// start of channel memory
#define	CHAN_ADDR	(SECTCH_ADDR + 0x80)				// channel data offset
#define	SECTCW_ADDR	(SECT00_ADDR + (12 * SECTOR_SIZE))	// start of message memory
