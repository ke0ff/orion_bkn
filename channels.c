/****************************************************************************************
 ****************** COPYRIGHT (c) 2018 by Joseph Haas (DBA FF Systems)  *****************
 *
 *  File name: channels.c
 *
 *  Module:    Control
 *
 *  Summary:   This is the main code file for the ADF4351 PLL setup application
 *
 *  File scope revision history:
 *    04-29-17 jmh:  Rev 1.2:
 *					 Added #if build option to support NUM_CHAN such that only the required number of channels
 *						are supported.
 *    04-29-16 jmh:  Rev 0.0:
 *                   Initial file creation
 *					 This source file holds the channel data arrays in the top sector of FLASH
 *    06-12-16 jmh:  Rev 0.1:
 *                   Added notes for linker settings.
 *					 Updated PLL registers for Orion-I PLL
 *
 *	To get the pll_ch[] array to target a specific FLASH address, configure the linker (for Keil,
 *	this is in the options dialog, under BL51 Locate, enter "?CO?CHANNELS(0x1d00)" into the Code: field)
 *	to set the target address to 0x1d00 to allow space for 100 channels (plus some spares).
 *
 ***************************************************************************************/
#include "typedef.h"
#include "main.h"

	// declarations for PLL channel data
	//	A number, N, channels are allocated (N = 100, typ.), with the first channel in the list being 00,
	//	and the last being N-1.
	//	Each Channel is organized as 6, 32 bit registers (for the ADF-4351) starting with register 0 at
	//	the lowest address, and register 5 at the highest address.
	//	Unused channels should have R5 set to 0xFFFFFFFF (the SW should detect this and either disable
	//	the ADF4351 output, or default to another channel, such as CH#00).  0xFFFFFFFF is the implicit
	//	value of a register location assuming that the FLASH bytes are in the erased state.
	//
	// These register data are for an Orion-I with 10 MHz ref osc:
	U32 code pll_ch_array[] = { //0xffffffff };

		//      R0          R1          R2          R3          R4          R5		// ADF reg#s
		// Ch 00
		0x00A00720, 0x08009389, 0x00004E42, 0x000004B3, 0x00E5043C, 0x00580005,		// 10 MHz Reference Clock and 50.057 MHz, 1KHz channel, VCO PWRDN enab, +5dBm, MTLD = enab (beacon)
		// Ch 01
		0x00AC9038, 0x08009389, 0x00004E42, 0x000004B3, 0x00B5043C, 0x00580005,		// 10 MHz Reference Clock and 432.288 MHz, 1KHz channel, VCO PWRDN enab, +5dBm, MTLD = enab (beacon)
		// Ch 02 - 1152.0
		0x00730010, 0x00008029, 0x00004E42, 0x000004B3, 0x0095042C, 0x00580005,		// 10 MHz Reference Clock and 1152 MHz (RX) -1dBm, 100KHz channel, MTLD, PS 4/5
//		0x00730288, 0x00008641, 0x00004E42, 0x000004B3, 0x0085043C, 0x00580005,		// 10 MHz Reference Clock and 2304.05 MHz VCOpd=disab, MTLD=enab, +5dBm
		// Ch 03


		0x007310C0, 0x08009389, 0x00004E42, 0x000004B3, 0x00C5043C, 0x00580005,		// 10 MHz Reference Clock and 144.286 MHz, 1KHz channel, VCO PWRDN enab, +5dBm, MTLD = enab (beacon)
#if NUM_CHAN > 4
		// Ch 04
		0x00A00720, 0x08009389, 0x00004E42, 0x000004B3, 0x00E5043C, 0x00580005,		// 10 MHz Reference Clock and 50.057 MHz, 1KHz channel, VCO PWRDN enab, +5dBm, MTLD = enab (beacon)

//		0x007310C0, 0x08009389, 0x00004E42, 0x000004B3, 0x00C50424, 0x00580005,		// 10 MHz Reference Clock and 144.286 MHz, 1KHz channel, VCO PWRDN enab, -4dBm, MTLD = enab (beacon)
//		0x00B18128, 0x080083E9, 0x00004E42, 0x000004B3, 0x00C5043C, 0x00580005,		// 10 MHz Reference Clock and 222.060 MHz, 1KHz channel, VCO PWRDN enab, +5dBm, MTLD = enab (beacon)
#endif
#if NUM_CHAN > 5
		// Ch 05
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
//		0x00AC86D8, 0x080087D1, 0x00004E42, 0x000004B3, 0x00B5043C, 0x00580005,		// 10 MHz Reference Clock and 432.345 MHz, 1KHz channel, VCO PWRDN enab, +5dBm, MTLD = enab (beacon)
//		0x00818150, 0x00008641, 0x00004E42, 0x000004B3, 0x0095043C, 0x00580005,		// 10 MHz Reference Clock and 1296.05 MHz VCOpd=disab, MTLD=enab, +5dBm
#endif
#if NUM_CHAN > 6
		// Ch 06
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
//		0x00B40178, 0x08008191, 0x00004E42, 0x000004B3, 0x00A5043C, 0x00580005,		// 10 MHz Reference Clock and 902.350 MHz, 1KHz channel, VCO PWRDN enab, +5dBm, MTLD = enab (beacon)
//		0x00AC83C8, 0x00008641, 0x00004E42, 0x000004B3, 0x0085043C, 0x00580005,		// 10 MHz Reference Clock and 3456.05 MHz VCOpd=disab, MTLD=enab, +5dBm
#endif
#if NUM_CHAN > 7
		// Ch 07
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
//		0x00818038, 0x080080C9, 0x00004E42, 0x000004B3, 0x0095043C, 0x00580005,		// 10 MHz Reference Clock and 1296.400 MHz, 1KHz channel, VCO PWRDN enab, +5dBm, MTLD = enab (beacon)
//		0x00B40520, 0x00008641, 0x00004E42, 0x000004B3, 0x00A5043C, 0x00580005,		// 10 MHz Reference Clock and 902.05 MHz VCOpd=disab, MTLD=enab, +5dBm
#endif
#if NUM_CHAN > 8
		// Ch 08
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
//		0x00731948, 0x0800BE81, 0x00004E42, 0x000004B3, 0x0085043C, 0x00580005,		// 10 MHz Reference Clock and 2304.045 MHz, 1KHz channel, VCO PWRDN enab, +5dBm, MTLD = enab (beacon)
//		0x00AC8400, 0x00008641, 0x00004E42, 0x000004B3, 0x00B5043C, 0x00580005,		// 10 MHz Reference Clock and 902.05 MHz VCOpd=disab, MTLD=enab, +5dBm
#endif
#if NUM_CHAN > 9
		// Ch 09
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
//		0x007310C0, 0x08009389, 0x00004E42, 0x000004B3, 0x00C50424, 0x00580005,		// 10 MHz Reference Clock and 144.286 MHz, 1KHz channel, VCO PWRDN enab, -4dBm, MTLD = enab (beacon)
//		0x00D70008, 0x08008641, 0x00004E42, 0x000004B3, 0x0085043C, 0x00580005,		// 10 MHz Reference Clock and 4300.05 MHz VCOpd=disab, MTLD=enab, +5dBm
#endif
#if NUM_CHAN > 10
		// Ch 10
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
//		0x007310C0, 0x08009389, 0x00004E42, 0x000004B3, 0x00C5042C, 0x00580005,		// 10 MHz Reference Clock and 144.286 MHz, 1KHz channel, VCO PWRDN enab, -1dBm, MTLD = enab (beacon)
#endif
#if NUM_CHAN > 11
		// Ch 11
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
//		0x007310C0, 0x08009389, 0x00004E42, 0x000004B3, 0x00C50434, 0x00580005,		// 10 MHz Reference Clock and 144.286 MHz, 1KHz channel, VCO PWRDN enab, +2dBm, MTLD = enab (beacon)
#endif
#if NUM_CHAN > 12
		// Ch 12
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
//		0x007310C0, 0x08009389, 0x00004E42, 0x000004B3, 0x00C5043C, 0x00580005,		// 10 MHz Reference Clock and 144.286 MHz, 1KHz channel, VCO PWRDN enab, +5dBm, MTLD = enab (beacon)
#endif
#if NUM_CHAN > 13
		// Ch 13
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 14
		// Ch 14
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 15
		// Ch 15
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 16
		// Ch 16
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
 #endif
#if NUM_CHAN > 17
		// Ch 17
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 18
		// Ch 18
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 19
		// Ch 19
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 20
		// Ch 20
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 21
		// Ch 21
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 22
		// Ch 22
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 23
		// Ch 23
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 24
		// Ch 24
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 25
		// Ch 25
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 26
		// Ch 26
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 27
		// Ch 27
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 28
		// Ch 28
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 29
		// Ch 29
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 30
		// Ch 30
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 31
		// Ch 31
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 32
		// Ch 32
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 33
		// Ch 33
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 34
		// Ch 34
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 35
		// Ch 35
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 36
		// Ch 36
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 37
		// Ch 37
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 38
		// Ch 38
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 39
		// Ch 39
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 40
		// Ch 40
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 41
		// Ch 41
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 42
		// Ch 42
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 43
		// Ch 43
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 44
		// Ch 44
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 45
		// Ch 45
		0xFFFFFFFF, 0xFFFFFF45, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 46
		// Ch 46
		0xFFFFFFFF, 0xFFFFFF46, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 47
		// Ch 47
		0xFFFFFFFF, 0xFFFFFF47, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 48
		// Ch 48
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 49
		// Ch 49
		0xFFFFFFFF, 0xFFFFFF48, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 50
		// Ch 50
		0xFFFFFFFF, 0xFFFFFF50, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 51
		// Ch 51
		0xFFFFFFFF, 0xFFFFFF51, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 52
		// Ch 52
		0xFFFFFFFF, 0xFFFFFF52, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 53
		// Ch 53
		0xFFFFFFFF, 0xFFFFFF53, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 54
		// Ch 54
		0xFFFFFFFF, 0xFFFFFF54, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 55
		// Ch 55
		0xFFFFFFFF, 0xFFFFFF55, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 56
		// Ch 56
		0xFFFFFFFF, 0xFFFFFF56, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 57
		// Ch 57
		0xFFFFFFFF, 0xFFFFFF57, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 58
		// Ch 58
		0xFFFFFFFF, 0xFFFFFF58, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 59
		// Ch 59
		0xFFFFFFFF, 0xFFFFFF59, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 60
		// Ch 60
		0xFFFFFFFF, 0xFFFFFF60, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 61
		// Ch 61
		0xFFFFFFFF, 0xFFFFFF61, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 62
		// Ch 62
		0xFFFFFFFF, 0xFFFFFF62, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 63
		// Ch 63
		0xFFFFFFFF, 0xFFFFFF63, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 64
		// Ch 64
		0xFFFFFFFF, 0xFFFFFF64, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 65
		// Ch 65
		0xFFFFFFFF, 0xFFFFFF65, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 66
		// Ch 66
		0xFFFFFFFF, 0xFFFFFF66, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 67
		// Ch 67
		0xFFFFFFFF, 0xFFFFFF67, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 68
		// Ch 68
		0xFFFFFFFF, 0xFFFFFF68, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 69
		// Ch 69
		0xFFFFFFFF, 0xFFFFFF69, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 70
		// Ch 70
		0xFFFFFFFF, 0xFFFFFF70, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 71
		// Ch 71
		0xFFFFFFFF, 0xFFFFFF71, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 72
		// Ch 72
		0xFFFFFFFF, 0xFFFFFF72, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 73
		// Ch 73
		0xFFFFFFFF, 0xFFFFFF73, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 74
		// Ch 74
		0xFFFFFFFF, 0xFFFFFF74, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 75
		// Ch 75
		0xFFFFFFFF, 0xFFFFFF75, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 76
		// Ch 76
		0xFFFFFFFF, 0xFFFFFF76, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 77
		// Ch 77
		0xFFFFFFFF, 0xFFFFFF77, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 78
		// Ch 78
		0xFFFFFFFF, 0xFFFFFF78, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 79
		// Ch 79
		0xFFFFFFFF, 0xFFFFFF79, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 80
		// Ch 80
		0xFFFFFFFF, 0xFFFFFF80, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 81
		// Ch 81
		0xFFFFFFFF, 0xFFFFFF81, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 82
		// Ch 82
		0xFFFFFFFF, 0xFFFFFF82, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 83
		// Ch 83
		0xFFFFFFFF, 0xFFFFFF83, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 84
		// Ch 84
		0xFFFFFFFF, 0xFFFFFF84, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 85
		// Ch 85
		0xFFFFFFFF, 0xFFFFFF85, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 86
		// Ch 86
		0xFFFFFFFF, 0xFFFFFF86, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 87
		// Ch 87
		0xFFFFFFFF, 0xFFFFFF87, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 88
		// Ch 88
		0xFFFFFFFF, 0xFFFFFF88, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 89
		// Ch 89
		0xFFFFFFFF, 0xFFFFFF89, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 90
		// Ch 90
		0xFFFFFFFF, 0xFFFFFF90, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 91
		// Ch 91
		0xFFFFFFFF, 0xFFFFFF91, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 92
		// Ch 92
		0xFFFFFFFF, 0xFFFFFF92, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 93
		// Ch 93
		0xFFFFFFFF, 0xFFFFFF93, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 94
		// Ch 94
		0xFFFFFFFF, 0xFFFFFF94, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 95
		// Ch 95
		0xFFFFFFFF, 0xFFFFFF95, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 96
		// Ch 96
		0xFFFFFFFF, 0xFFFFFF96, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 97
		// Ch 97
		0xFFFFFFFF, 0xFFFFFF97, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 98
		// Ch 98
		0xFFFFFFFF, 0xFFFFFF98, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 99
		// Ch 99
		0xFFFFFFFF, 0xFFFFFF99, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
	};
