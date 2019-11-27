/****************************************************************************************
 ****************** COPYRIGHT (c) 2016 by Joseph Haas (DBA FF Systems)  *****************
 *
 *  File name: flash.c
 *
 *  Module:    Control
 *
 *  Summary:   This file contains the flash scratchpad r/w routines.
 *
 *  File scope revision history:
 *    08-11-16 jmh:  Rev 0.0:
 *                   adapted from F120 version
 *
 ***************************************************************************************/

#include "typedef.h"
#include "c8051F520.h"
//#include "stdio.h"
#define FLASH_INCL
#include "flash.h"

//------------------------------------------------------------------------------
// Define Statements
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Variable Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// flash initialization routine
//-----------------------------------------------------------------------------
void init_flash(void)
{
	VDDMON = 0xB0;
	PSCTL = 0x00;
	RSTSRC |= PORSF; 
}


//-----------------------------------------------------------------------------
// flash erase routine
//	erases scratchpad sector pointed to by addr
//-----------------------------------------------------------------------------
U8 erase_flash(U8 xdata * addr)
{
	U8	EA_save;
	U8	rtn;

	EA_save = EA;
	EA = 0;							// interrupts = off
	FLKEY = 0xA5;					// unlock FLASH
	FLKEY = 0xF1;
	rtn = FLKEY;
	if(rtn != 0x02){
		rtn |= 0x80;
	}
	PSCTL = PSEE|PSWE;				// enable erase/movx
	*addr = 0xff;					// erase sector
	PSCTL = 0x00;					// disbale erase
	EA = EA_save;					// restore intr
	return rtn;
}


//-----------------------------------------------------------------------------
// flash write routine
//	writes byte to scratchpad sector pointed to by addr
//-----------------------------------------------------------------------------
void wr_flash(char byte, U8 xdata * addr)
{
U8	EA_save;

	EA_save = EA;
	EA = 0;							// interrupts = off
	FLKEY = 0xA5;					// unlock FLASH
	FLKEY = 0xF1;
	PSCTL = PSWE;					// enable movx
	*addr = byte;					// write data
	PSCTL = 0x00;					// disable flash wr
	EA = EA_save;					// restore intr
}


