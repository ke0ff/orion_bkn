/*************************************************************************
 *********** COPYRIGHT (c) 2017 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: cwconst.h
 *
 *  Module:    Control
 *
 *  Summary:   This is the header file for cwconst.c
 *
 *******************************************************************/

/********************************************************************
 *  File scope declarations revision history:
 *    04-20-17 jmh:  creation date
 *
 *******************************************************************/

//------------------------------------------------------------------------------
// extern defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// public Function Prototypes
//------------------------------------------------------------------------------
extern U8 code diode_matrix[];

//------------------------------------------------------------------------------
// global defines
//------------------------------------------------------------------------------

#define	KEY_IDX 	0			// (8b)  offset in CW array for key polarity
#define KEY_MASK	0x01		//		 mask for KEY bit
#define	DAC_MASK	0x40		//		 mask for DAC-ramp enable bit
#define FSK_MASK	0x80		//		 mask for FSK mode bit
#define	RMP_IDX 	1			// (8b)  offset in CW array for ramp length
#define	DIT_IDX 	2			// (16b) offset in CW array for dit length
#define	DLY_IDX 	4			// (16b) offset in CW array for intra-msg delay
#define	RTBL_IDX	6			//	(8 bytes) ramp-DAC profile table. 8 bytes with the 8bit DAC levels alont the ramp intervals
#define	RTBLE_IDX	14			//	end of ramp table
#define	MSG_IDX 	14			// offset in CW array for start of message
