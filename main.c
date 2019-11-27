/****************************************************************************************
 ****************** COPYRIGHT (c) 2018 by Joseph Haas (DBA FF Systems)  *****************
 *
 *  File name: main.c
 *
 *  Module:    Control
 *
 *  Summary:   This is the main code file for the ADF4351 PLL beacon application
 *
 *  Project scope revision history:
 *
 * !!!! Need to test key ramp timing logic. !!!!
 *
 *    11-19-18 jmh:  Rev 0.26, HWrevA/B/C (release candidate)
 *						Tweaked msg Read cmd to improve readability.
 *    10-07-18 jmh:  Rev 0.25, HWrevA/B/C (release candidate)
 *						Added 2ms delay before VCO shutdown on channel init. to allow PLL to settle before controlling VCO/RFO
 *						CHdelta is now cleared by all chset commands.
 *						Corrected EOM semaphore.
 *						GPIO and CH cmds tested for ch0/1 and out 0/1.
 *						Fixed ramp in DAC mode (was ramping on every element).
 *						Re-coded PTT logic to incorporate PTTenab register.  An if() statement intercepts nPTT detect just prior
 *							to the PLL ipl2 branch.  This intercept enables the ipl2 branch to reset the PLL based on the current
 *							CH settings and sets PLLenab = 1.  The if() statement that previously intercepted nPTT now intercepts
 *							PLLenab as a wrapper to the original if().  PLLenab = 0 at the end of the wrapper.
 *						Updated beacon spreadsheet to implement new features (GPOUT, DAC, and FSK modes)
 *    09-09-18 jmh:  Rev 0.25, HWrevA/B/C (release candidate)
 *						Debug of DAC ramp features.  Re-configured delay_halfbit to be a generic delay routine with a 16b
 *							parameter {now called delay_us()}.
 *						SPI flags were not indicating end of xfr as was previously thought.  Implemented several #defines
 *							to pass proper delays through delay_us().
 *						Re-mapped ramp profile to include offset for emitter follower deadband.  E-follower ramp shape is
 *							very close to the expected waveform.
 *						NEED TO REGRESSION TEST ...
 *    09-02-18 jmh:  Rev 0.25, HWrevA/B/C (release candidate)
 *						Made provisions to support FSK CW.  Set channel is "key down" frequency, the next channel up
 *							is the "key up" frequency.  Sends reg1 & 0 to change frequencies.  PLL config needs to be
 *							such that no other registers change between the two FSK frequencies.  The key polarity byte
 *							in the message array holds the enable for FSK (bit 0x80).  KEY still activates as before, but
 *							there are no ramp timings applied in FSK mode.
 *						Started process to allow a DAC to be used to set the ramp.  Uses KEYOUT output as SPI strobe, and uses
 *							same SPI bits as ADF4351 to drive an SPI DAC.
 *							8 step ramp values, with 0.6V offset:
 *							Index	RAW DAC (8b)	Chart of ramp (1ms per step)
 *													0    ---> V(dac) --->    max
 *							000		25				   *
 *							001		37				    *
 *							010		61				      *
 *							011		110				           *
 *							100		170				                 *
 *							101		219				                      *
 *							110		243				                        *
 *							111		255				                         *
 *													0    ---> V(dac) --->    max
 *						DAC now integrated <TESTING UNDERWAY 9/9/18>.  Set bit 0x40 in the key polarity byte to enable the DAC.
 *							When enabled, the KEYOUT GPIO becomes the DAC strobe (/CS).  Data and clock for the DAC come
 *							from the PLL SPI.  The DAC is intended to drive a non-inverting op-amp to supply ramped 5V to
 *							the GVC buffer, or to a later amplifier stage.  Ramp time is 8ms, with the bulk of the voltage
 *							change happening in about 5ms.
 *							Will look at placing the ramp array (8 DAC values that hold the ramp shape) in the diode matrix
 *							space to allow the array to be tweaked or customized to meet the needs of a particular application.
 *    08-11-18 jmh:  Rev 0.24, HWrevA/B/C (release candidate)
 *						Changed delay_halfbit to use HW timer0 instead of for-loop
 *						converged delay halfbit into a single Fn for BB/HWSPI.  Now, base timer value changes based on
 *							compile directive.
 *    08-10-18 jmh:  Rev 0.23, HWrevA/B/C (release candidate)
 *						Added halfbit delay after PLL init to counteract the RC delay in the REVC PLL CS hardware
 *						Reduced the halfbit delay.  Was about 392 us (measured), reduced to 196 us.
 *    02-17-18 jmh:  Rev 0.22, HWrevA/B/C (release candidate)
 *						Fixed "i" command to re-sample CH inputs
 *						Added "ch add" command to add (or subtract using 2's complement numbers) to the existing
 *							channel#.  Allows relative channel addressing inside CW message.
 *						"ch clr" command clears dela ch if it is greater than the operand.
 *    02-04-18 jmh:  Rev 0.21, HWrevC (release candidate)
 *						Modified to support bit I/O using FSEL4-6
 *						0x18 EOM semaphore modified to support multiple features:
 *							msg format    Description						Spreadsheet syntax
 *							0x18 0xff	= end of message					]%
 *							0x18 0xe0	= add 1 to I/O bits [6:4]			]>
 *							0x18 0xd0   = subtract 1 from I/O bits [6:4]	]<
 *							0x18 0xcx   = set pll ch to "x"					]a-p (a = ch00, p = ch15, not case sensitive)
 *							0x18 0xbx   = add "x" to PLL ch, mask to 4bits	]a-p (a = ch00, p = ch15, not case sensitive)
 *							0x18 0x9x   = if deltach > "x", clr delch		]a-p (a = ch00, p = ch15, not case sensitive)
 *							0x18 0xax   = set I/O bits [6:4] to "x"			]0-7
 *							0x18 0x08   = NOP (bits [2:0] are don't care)	]8-f
 *
 *							The command semaphore must be byte aligned within the message space (which will thus byte align
 *							the operand).  This must be addressed by the external message pre-processor (e.g., the spreadsheet).
 *                      Since only 4 bits are now used to input ch#, the BCD restriction has been bypassed.
 *							This allows 16 channels to be selected with 4 bits.
 *						Changed cwptr increment protocol.  This allows CW_STOP commands to start at the 1st msg byte
 *							and also allows sequential commands in the message stream.
 *						Modified "R" to properly trap EOM (which is now a 2-byte semaphore)
 *						Added haradware SPI and #if construct to select bit-bang or hdwr
 *						Modified message CRC to calculate from the start of the message space, up to and including
 *							the 0xFF that follows the command semaphore.
 *    04-29-17 jmh:  Rev 0.1, HWrevC (release candidate)
 *						Modified to support revC hardware:
 *							inverted LE and LOCK
 *							moved CH01 to 144.1 MHz
 *						CRC check modified to checck only the # channels specified in NUM_CHAN
 *    04-18-17 jmh:  Beacon Rev 0.1 (prototype)
 *						Re-arranged code to set BCD channel on power-up only then turn output on/off
 *							based on PTT status.
 *	  04-20-17 jmh:		Completed CW keying structure.  Keying drives VCO enable for true on/off
 *							keying.  PB7 = digital key out (GND = key) to drive wave-shape control via
 *							buffer power soft on-off.  Digital output goes active essentially when the
 *							PLL output turns on, but leads PLL turn-off by 5ms.
 *						CW string is a "diode matrix" array.  The array is shifted MSb first at the
 *							dit rate.  A "1" bit turns on the output, "0" turns it off.  The length of
 *							the array must be stored in FLASH.  Also, the dit-period is stored in FLASH
 *							to allow it to be uploaded with the CW matrix and channel data.
 *						PTT will force the output on.  This disables serial commands as long as PTT = GND.
 *						Channel load only happens on power up.  Changing the BCD channel # requires a power
 *							cycle or reset.
 *						Temp register feature is not available with this SW version.
 *						Mem Map:
 *						Channel data:	0x1280		U32, ADF4351 register channels (50)
 *						key_polarity:	0x1800		U8,  1 for high key, 0 for low key
 *						dit_time:		0x1801		U16, ms/dit
 *						message_delay:	0x1803		U16, delay from end of CW msg to re-start (ms)
 *						CW String:		0x1805		U8[], bit string of CW elements, sent MSb first
 *    08-12-16 jmh:  Rev 1.0 (release candidate)
 *						PTT and serial functions added to basic ADF4351 upload framework
 *						Updated comments to capture command line features
 *						Updated C and BL configs to get mem allocation to work within the 256 byte limit.
 *							mem model: small
 *							code model: large
 *							specify idata for large RAM buffers and arrays
 *						Finalized FLASH workarounds to address loss of upper 2.5K of FLASH
 *    04-09-16 jmh:  Rev 0000:
 *                   Initial project code copied from GPSDO and cleaned
 *
 ***************************************************************************************/

/****************************************************************************************
 *  File scope revision history:
 *  04-09-16 jmh:	creation date
 *
 ***************************************************************************************/

#define	REVC_HW 	0		// 1 = build for rev C hardware, else set to 0 for rev A or B
//#define	BB_SPI		1		// If defined, use bit-bang SPI code

#ifdef BB_SPI
	// BitBangSPI version uses HW timer0 to establish the bit-delay (200us, nominal)
	// T0 has about 0.5us of delay per timer tic when configured for clock source = SYSCLK/12
	// define 200us timer delay @24.5MHz/12 timer clock = (65536 - (400*0.5us))
#define	HAFBIT	65136
#define SH_DLY	0xfff0

#else
	// HWSPI version uses HW timer0 to establish quick delay (8us, nominal)
	// T0 has about 0.5us of delay per timer tic when configured for clock source = SYSCLK/12
	// define 8us timer delay @24.5MHz/12 timer clock = (65536 - (32*0.5us))
#define	BYTDLY	0xFFDC
#define SH_DLY	0xfff0
#endif

	// delay reg for 1ms = 65536 - (1ms/.5us) = 65536 - 2000 = 63536
#define	MS_DLY	63536

//--------------------------------------------------------------------------------------
// main.c
//      Uses a C8051F531-C-IT processor to program the ADF4351 PLL chip registers.
//			20 pin TSSOP package.
// *****
//		Some housekeeping:
//		Linker directive "?CO?CHANNELS(0x1680)" must be placed in the CODE SEGMENT command field
//			of the Keil compiler (configuring for other compiler suites are left as an excercise).
//			This places the channel data at the fixed FLASH location of 0x1680 which allows for
//			the containing FLASH sectors to be erased and re-written.  Sectors are 256 bytes, so
//			there is also 128 bytes of uncommitted FLASH between 0x1600 and 0x167F that are available
//			for cal or other NV memory features (not currently supported).
//
//			When using Keil, "pll_ch = pll_ch_array;" must be placed in main().  Configuring
//			this arrangement for other compiler suites is left as an excercise.
//			This allows the SW to access the array at the fixed CODE location of 0x1680.
//			This issues a warning in V5 of Keil, but not in V4.  Accepting the warning is thus far the only
//			successful mitigation.
//
//			If a clean data build is needed (i.e., no channel data in the object file), "channels.c"
//			can be removed from the project ("channels.h" header file must still be available to main.c)
//			The "pll_ch_array" statement in main() illustrated above must be replaced by "pll_ch = (U32 *) CHAN_ADDR;"
//			This will produce a SW obect that should load over into a part with an existing channel array
//			without disturbing the array.
//
//			SW Updates should be distributed as a SW object and a default channel object to allow
//			users the option to perform a clean load (load both objects) or just update the SW
//			(load the SW object only).
// *****
//      Up to 16 PLL channels may be stored and recalled using a 4-bit binary input,
//			GND true.  Channels are stored in top pages of FLASH, starting at 0x1680.  Unprogrammed,
//			or blank, channels should contain 0xFFFFFFFF for all register entries.
//
//			The hardware is configured to use a 16-pin dual-row header
//			  to input the channel select and PTT lines, C2D lines, and power.
//			  PTT is used to select between CH00 (PTT = hi) or the channel at the FSEL inputs
//			  (PTT = low).  If PTT funtion is not desired, it may be tied low.  This allows CH00
//			  to be used as any other channel.  If PTT is to be used to key the output, CH00 must
//			  contain a register set that disables the VCO (this is in the default compile load).
//
//		UART access supports external programming of channels.  Format is described below.
//			The programming erases all channels, then loads them one channel at a time.  A PC app
//			to manage the register input from the user and output to HEX format file
//			would be helpful (TBD).  An Excel spreadsheet has been written that outputs Intel HEX
//			or UART channel commands (see below).
//
//      The following resources of the F531 are used:
//      24.000 MHz internal osc
//
//      UART: 9600 baud, Simple I/O protocol
//
//      Timer0: n/u
//      Timer1: UART baud rate (9600 baud)
//      Timer2: Application timer (1ms/tic)
//
//      ADC: reads Port1-4 (LMT85) for temperaure
//
//      PCA: n/u
//
//      SYSTEM NOTES:
//		16 pin header for I/O (rev A, C):
//		pin  1: +Vin (5-12V)	pin  2: GND
//		pin  3: RS-232 TXD		pin  4: +3.3V
//		pin  5: RS-232 RXD		pin  6: GND
//		pin  7: /PTT			pin  8: FSEL 0
//		pin  9: FSEL1			pin 10: FSEL 2
//		pin 11: FSEL3			pin 12: GPIO_0
//		pin 13: GPIO_1			pin 14: GPIO_2
//		pin 15: KEY				pin 16: GND
//
//		Channels consist of 16, 32 bit x 6 register values that are sent to the PLL when selected.
//		Channel data is stored in top FLASH sectors.  System only supports erase
//			of the entire channel sector mem.  Upload of individual channels is allowed.
//
//			Note: this limits available code space to 5.5K (5632 bytes)
//
//		UART Command Line Protocol:
//		EC
//			Erase all PLL channels
//			prompts "Erase all, press Y to accept" and waits 5 sec for input.  Any character
//			other than upper-case "Y", or a delay of more than 5 sec will cause this command to abort.
//
//		EM
//			Erase all message data
//			prompts "Erase all, press Y to accept" and waits 5 sec for input.  Any character
//			other than upper-case "Y", or a delay of more than 5 sec will cause this command to abort.
//
//		Mxxaaaaaaaabbbbbbbbccccccccddddddddeeeeeeeeffffffff
//			Programs channel "xx" (xx is BCD ASCII '00' thru '99') with R0 (A), thru R5 (F) values
//			Data is represented as ASCII hex
//			spaces, commas, or tab characters (ASCII 0x08) may be present between channel data characters, but
//			the line buffer limit is 62 characters, including the command and channel# characters.
//			If any invalid data is received (non-space, non-numeric, non-hex), the command is aborted (with error message)
//			Serial buffer is limited to 64 characters (including <CR>).
//
//		<<<NOT IMPLEMENTED ON BKN>>> t00aaaaaaaabbbbbbbbccccccccddddddddeeeeeeeeffffffff
//			Programs temp channel with R0 (A), thru R5 (F) values.  It has the same syntax requirements
//			as the "M" command. The temp channel allows a register set to be input to the Orion without
//			setting it in FLASH.  Once set, the temp command overrides the BCD inputs.  PTT behaves normally
//			and will not change the temp channel configuration.
//			Selecting a new BCD channel, or programming a channel with the "M" command will cancel
//			the temp channel.  The temp channel command must then be be re-entered if needed.
//
//		rxx
//			Read channel "xx" (xx is BCD ASCII '00' thru '16')
//		<<<NOT IMPLEMENTED ON BKN>>> rr
//			Read temp channel
//		r-
//			Read all channels
//			Channel data is output in the "M" entry format described above with spaces inserted
//			between register fields.
//
//		<<<NOT IMPLEMENTED ON BKN>>> i
//			re-send last resister set
//
//		z hhhh
//		zm hhhh
//			Calc CRC for channel array (m for message array) and compare to HEX value "hhhh".  Display "Pass/Fail"
//		c
//		cm
//			Calc CRC for channel array (m for message array) and display as 4 digit HEX value
//
//		e
//			echo command line.  This is a debug command that will echo the characters on the command line.
//
//		Q
//			Query error status.
//		QC
//			Clear errors status.
//
//		Caaaaxxyyzz...\r
//			program CW message string.  This includes the key polarity, dit time, and message delay
//			params.
//
//		R
//			Read CW message array (ascii hex)
//
//		L
//			read PLL lock status
//
//		i
//			re-init channel/message.  Must be issued following a PLL or message update, or to change channels (not needed
//			for embedded message channel changes).
//
//		All commands are terminated with <CR> ('\r').
//		Serial port does not echo characters.
//
//		Serial port sends power on message when reset.
//		Serial port outputs current channel selection, CH#, when setting is changed or PTT is cycled.
//
//--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
// compile defines
//#define F360DK ON

#define IS_MAINC
#include "main.h"
#include "typedef.h"
#include "c8051F520.h"
#include "serial.h"
#include "channels.h"
#include "cwconst.h"
#include "flash.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

//  see init.h for publuc #defines

#define	PBMAX		50			// max channel #s (2-digit BCD input)
#define	MAX_REG		24			// max bytes in an ADF4351 reg set
#define	MAX_CHAN	49			// max # bcd channels allowed
#define	PB_MASK		0x0F		// BCD port valid inputs
//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------

U8  spi_tmr = 0;

//-----------------------------------------------------------------------------
// Main Variables
//-----------------------------------------------------------------------------

// I/O port assignments

sbit PB0		= P1^0;				// (i) button input 0
sbit PB1		= P1^1;				// (i) button input 1
sbit PB2		= P1^2;				// (i) button input 2
sbit KEYOUT		= P1^7;				// (i) button input 7, retasked as CW KEY out
sbit SCK        = P0^0;				// (o) SPI SCLK
sbit MISO       = P0^1;				// (i) SPI MISO/LDET
sbit MOSI       = P0^2;				// (o) SPI MOSI
sbit nPTT		= P0^3;				// (i) /PTT input
sbit nPLL_LE	= P0^7;				// (o) SPI LE

#if REVC_HW == 1
#define	LE_ON	1
#define	LE_OFF	0
#else
#define	LE_ON	0
#define	LE_OFF	1
#endif

#if REVC_HW == 1
#define	PLL_LOCK	0
#else
#define	PLL_LOCK	1
#endif

//-----------------------------------------------------------------------------
// Local variables
//-----------------------------------------------------------------------------
U16 waittimer;						// wait() function timer
U16 msgtimer;						// msg delay timer
bit	elem_flag;						// element timer registers for CW output
bit	cw_on;
U16	elem_timer;						// 10 ms granularity
U16	elem_time;
bit	dacmode;						// set if keyout = LTC2630
#define	RF_ENAB		0x0020			// reg 4 bit 5 enables RF out
#define	VCO_DISAB	0x0800			// reg 4 bit 11 disables VCO out

U8	iplTMR; // = TMRIPL;            // timer IPL init flag
U32 code * pll_ch;					// pointer to base of channel array (initialized in main())

//-----------------------------------------------------------------------------
// Local Prototypes
//-----------------------------------------------------------------------------

U8 valid_cw(U8 code * ptr);
void send_spi32(U32 plldata);
void delay_us(U16 dly);
U16 calcrc(U8 c, U16 oldcrc);
void wait(U16 waitms);
//void pb_state(U8 imode);
U32 *get_chan(U8 chanum);
U8 conv_to_chnum(U8 portbits);
void put_hex(U8 dhex);
void put_dec(U8 dhex);
U8 convnyb(U8 c);
U8 getbyte(U8* dataptr);
U8 whitespc(char c);
void ramp(U8 updn);
void send_spi8(U8 daccmd, U8 dacdata);
void setkeyout(U8 updn);

//******************************************************************************
// main()
//  The main function inits I/O and process inputs.
//	sends PLL data to ADF4351 when channel select inputs change
//
//******************************************************************************
void main(void) //using 0
{
	char c;				// temp term chr
	bit	ipl;			// initial loop flag
	bit	ipl2;			// PLL init flag
	U8	i;				// loop counter
	U8	j;				// loop counter
	U8	k;				// loop counter
	bit	flag;			// temp flag
	bit	goteol;			// temp flag
	U8	PBreg;			// PB memory
	U8	PBtemp;			// PB temp holding
	U8	PTTreg;			// PTT memory
	U8	PTTenab;		// PTT enable latch
	U8	CHtemp;			// channel temp
	U8	CHdelta;		// channel temp
	U8	CHrun;			// dynamic channel temp
	bit	temp_active;	// temp reg active flag
	bit loaderr;		// channel pgm error flag
	bit	fsk_enable;		// holds the fsk enable mode
	bit	last_key;		// last key status (for FSK)
	U8	pgm_chnum;		// prog chan temp
	U8	tempbyte;		// temp
	U8	tempbyte2;		// temp
	U8	temp_chan[24];	// temp channel register set (bytes)
	U16 tempword;		// temp16
	bit	z_temp;			// "z" cmd flag
	U16	temp_crc;		// crc temp
	U16 ii;				// crc temp
	U32	reg4_32;		// ADF4351 reg 4 holding
	U32	reg2_32;		// ADF4351 reg 2 holding
	U32	reg1_32;		// ADF4351 reg 1 holding
	U32	reg0_32;		// ADF4351 reg 0 holding
	U32	reg01_32;		// ADF4351 reg 1 holding (FSK "off")
	U32	reg00_32;		// ADF4351 reg 0 holding (FSK "off")
	U32* tptr;			// reg pointer
	U8* cwptr;			// cw pointer
	U8	cwmask;			// cw bitmask
	U8 xdata * fptr;	// flash pointer
	U8 code * rptr;		// flash pointer
	bit	key_dn;			// RF key (timer)
	bit	erase_hold;		// erase hold flag
	
	// start of main
	PCA0MD = 0x00;								// disable watchdog
	// init MCU system
	Init_Device();								// init MCU
#ifndef	BB_SPI
    XBR0      = 0x03;							// enable hdwr SPI on xbar
    SPI0CN    = 0x01;							// enable hdwr SPI
#endif
	SCK = 0;									// init SPI pins
	MISO = 1;
	nPLL_LE = LE_OFF;
	init_flash();								// init FLASH
	P1 = 0x7F;									// enable port for input
	PBreg = P1;									// init PB memory
	PTTreg = ~nPTT;								// force PTT edge det for POR
	pll_ch = pll_ch_array;						// set array to point to fixed location
	init_serial();								// init serial module
	// init module vars
	iplTMR = TMRIPL;                        	// timer IPL init flag
	cw_on = 0;									// turn off CW
	msgtimer = 0;
	key_dn = 0;
	EA = 1;
	wait(50);                               	// 50 ms delay
	
#if REVC_HW == 1
	putss("\nADF4351 Beacon Exctr Ver 0.26, de ke0ff\n");	// send sw version msg to serial port
#else
	putss("\nADF4351 Beacon Exctr Ver A0.26, de ke0ff\n");	// send sw version msg to serial port
#endif
	put_dec(NUM_CHAN);							// include # channels supported
	putss(" CH, gnd-true BCD, PTTin low = key down,\n");		// send help screen
	putss("Serial cmd enabled\n");				// send help screen
	if(RSTSRC & 0x40){
		putss("FLERR\n");
		RSTSRC &= ~0x40;
	}
//	KEYOUT = (diode_matrix[KEY_IDX] & KEY_MASK) ^ 0x01;		// make key output inactive
	KEYOUT = 1;									// make key output inactive (assume DAC ramp mode for now)
	temp_active = 0;							// de-activate temp reg
	loaderr = 0;								// init chan error status
	ipl = 1;									// set initial loop
	ipl2 = 1;									// init PLL
	CHrun = 0xff;								// disable dynamic channel
	last_key = 0;								// init last key memory
	setkeyout(0xAA);							// init rampdac key mem
	PTTenab = 0;								// open PTT latch

	// main loop
	while(1){
		if(nPTT == 0){
			ipl2 = 1;												// re-latch CH inputs (reset PLL)
			CHrun = 0xff;
			PTTenab = 1;											// enable PTT logic
		}
		if(ipl2 == 1){
			ipl2 = 0;
			if(CHrun == 0xff){
				CHrun = 0;											// clear semaphore
				CHdelta = 0;
				msgtimer = 0;
				key_dn = 0;
				// process PTT/channels
				PBtemp = (~P1) & PB_MASK;							// convert port to POS logic
//				CHtemp = conv_to_chnum(PBtemp);						// convert port state to channel#
				CHtemp = PBtemp & 0x0f;								// convert port state to channel#
				putss("CH ");										// send status msg (at 9600 baud, this gives us > 10ms of debounce)
				put_dec(CHtemp);									// print ch#
				fsk_enable = (U8)diode_matrix[KEY_IDX] & FSK_MASK;	// get fsk mode bit
				if(fsk_enable){
					putss("FSK mode\n");
				}
				dacmode = (U8)diode_matrix[KEY_IDX] & DAC_MASK;		// get DAC mode bit
				if(dacmode){
					send_spi8(DAC_IREF, 0);							// set DAC to use internal ref
					send_spi8(DAC_SET, 0);							// clear DAC
					putss("DAC-RAMP enabled\n");
				}
				elem_time = ((U16)diode_matrix[DIT_IDX] << 8) | ((U16)diode_matrix[DIT_IDX+1]); // init element timer to slowest value
				if(elem_time == 0xffff){
					erase_hold = TRUE;
					putss("dit time invalid\n");
					cw_on = 0;
				}else{
					// validate message
					rptr = (U8 code *) SECTCW_ADDR;					// set pointer to start of CW msg
					rptr += MSG_IDX;
					if(!valid_cw(rptr)){
						erase_hold = TRUE;
						putss("msg invalid\n");
						cw_on = 0;
					}else{
						erase_hold = FALSE;
					}
				}
				elem_timer = 1;
			}
			tempbyte2 = (CHtemp + CHdelta) & 0x0f;
			tptr = get_chan(tempbyte2);								// calc tptr to R5 of correct channel array
			if(*tptr == 0xffffffff){
				tptr = get_chan(0);									// default to ch#00 if R5 is 0xffffffff (i.e., ch is empty)
			}
			tptr = tptr - 1;
			reg4_32 = *tptr;										// get R4 value
			tptr = tptr - 2;
			reg2_32 = *tptr;										// get R2 value
			tptr = tptr - 1;
			reg1_32 = *tptr;										// get R1 value
			reg01_32 = *(tptr+6);									// get R1 value from next channel
			tptr = tptr - 1;
			reg0_32 = *tptr;										// get R0 value
			reg00_32 = *(tptr+6);									// get R0 value from next channel
			tptr = tptr + 5;
			for(i=6;i!=0;i--){
				send_spi32(*tptr--);								// transfer channel data to PLL
			}
			wait(2);
			if(!fsk_enable){
				reg4_32 |= RF_ENAB | VCO_DISAB;						// set R4 value
			}
			send_spi32(reg4_32);									// keyup
		}
		if(!erase_hold){
			// process PTT
			// PTT = 1 turns off RF out (reg 4, bit 05 = 0), = 0 turns on RF (reg 4, bit 05 = 1).
			//	!! channel data must have reg 4, bit 05 = 0 !!
			if(PTTenab != 0){
				if(nPTT == 0){
					PTTenab = 0;									// disable PTT logic
					cwptr = &diode_matrix[MSG_IDX-1];				// reset cw pointer
					cwmask = 0;
					msgtimer = 0;
					cw_on = 0;
					send_spi32(reg4_32 & (~VCO_DISAB));				// keydn
					setkeyout(1);
	//				KEYOUT = diode_matrix[KEY_IDX] & KEY_MASK;
					while(nPTT == 0){
					}
					cw_on = 1;
					setkeyout(0);
	//				KEYOUT = (diode_matrix[KEY_IDX] & KEY_MASK) ^ 0x01;
	//				wait((U16)diode_matrix[RMP_IDX] & 0xff);
					send_spi32(reg4_32);							// keyup
					wait(100);										// wait to re-start
				}
				PTTenab = 0;
			}
			if((msgtimer == 0) && (cw_on == 0)){
				cwptr = &diode_matrix[MSG_IDX-1];					// reset cw pointer
				cwmask = 0;
				cw_on = 1;
			}
			if(elem_flag){													// process element edge
				elem_flag = 0;
				if(!cwmask){
					cwmask = 0x80;
					cwptr += 1;
					if(*cwptr == CW_STOP){									// CW_STOP = 0x18 is now a command indicator
						++cwptr;											// point to parameter byte
						tempbyte = *cwptr;									// get parameter byte (next byte after CW_STOP)
						switch(tempbyte & 0xf0){							// mask cmd nybble and process switch
							default:										// unrecognized params process as EOM
							case CW_EOM:									// end of message
								setkeyout(0);
//								KEYOUT = (diode_matrix[KEY_IDX] & KEY_MASK) ^ 0x01;		// turn off keyIO
								wait((U16)diode_matrix[RMP_IDX] & 0xff);	// delay <ramp_delay> for wave shaping
								send_spi32(reg4_32);						// transfer channel data to PLL (turn off RF)
								msgtimer = ((U16)diode_matrix[DLY_IDX] << 8) | ((U16)diode_matrix[DLY_IDX+1] & 0xff);
								cw_on = 0;
								break;
							
							case CW_IOP:									// I/O++
								i = P1 & 0x70;								// mask I/O bits
								i = (i + 0x10) & 0x70;						// add 1 & mask I/O bits
								P1 = (P1 & 0x8f) | i;						// update I/O
								break;
							
							case CW_IOM:									// I/O--
								i = P1 & 0x70;								// mask I/O bits
								i = (i - 0x10) & 0x70;						// subtract 1 & mask I/O bits
								P1 = (P1 & 0x8f) | i;						// update I/O
								break;
							
							case CW_CHSET:									// set ch
								CHtemp = tempbyte & 0x0f;					// get ch# (only recognizes lower 4 bits)
								CHrun = 0;									// make sure semaphore is clear
								CHdelta = 0;
								ipl2 = 1;									// reset channel only
								break;
							
							case CW_CHADD:									// add ch
								CHdelta = (CHdelta + tempbyte) & 0x0f;		// add ch# (only recognizes lower 4 bits)
								CHrun = 0;									// make sure semaphore is clear
								ipl2 = 1;									// reset channel only
								break;
							
							case CW_CHCLR:									// clr deltach
								if(CHdelta > (tempbyte & 0x0f)){
									CHdelta = 0;							// add ch# (only recognizes lower 4 bits)
									CHrun = 0;								// make sure semaphore is clear
									ipl2 = 1;								// reset channel only
								}
								break;
							
							case CW_IOSET:									// set I/O
								if((tempbyte & 0x08) == 0){					// if bit 3 set, it is a NOP cmd
									tempbyte &= 0x07;						// mask I/O bits
									tempbyte <<= 4;							// align to I/O bits
									P1 = (P1 & 0x8f) | tempbyte;			// mask off I/O bits and update I/O
								}
								break;
						}
						cwmask = 0;	 										// clear mask to trigger increment to next msg byte
					}
				}
				if(fsk_enable){													// is FSK mode
					if(*cwptr & cwmask){										// if element == "1"
						if(!last_key){
							setkeyout(1);
//							KEYOUT = diode_matrix[0] & KEY_MASK;				// turn on keyIO
							send_spi32(reg1_32);								// transfer FSK "ON" data to PLL
							send_spi32(reg0_32);								// transfer FSK "ON" data to PLL
						}
						last_key = 1;											// update key memory
					}else{														// else element == "0"
						if(last_key){
							setkeyout(0);
//							KEYOUT = (diode_matrix[KEY_IDX] & KEY_MASK) ^ 0x01;	// element = "0", turn off keyIO
							send_spi32(reg01_32);								// transfer FSK "OFF" data to PLL
							send_spi32(reg00_32);								// transfer FSK "OFF" data to PLL
						}
						last_key = 0;											// update key memory
					}
				}else{															// is OOK mode
					if(*cwptr & cwmask){										// if element == "1"
						if(dacmode){
							send_spi32(reg4_32 & (~VCO_DISAB));					// transfer channel data to PLL (use DAC to ramp)
							setkeyout(1);
						}else{
							setkeyout(1);
							send_spi32(reg4_32 & (~VCO_DISAB));					// transfer channel data to PLL (use RC ramp)
						}
//						KEYOUT = diode_matrix[0] & KEY_MASK;					// turn on keyIO
//						wait(1);
					}else{														// else element == "0"
						setkeyout(0);
//						KEYOUT = (diode_matrix[KEY_IDX] & KEY_MASK) ^ 0x01;		// element = "0", turn off keyIO
//						wait((U16)diode_matrix[RMP_IDX] & 0xff);				// delay <ramp_delay> for wave shaping
						send_spi32(reg4_32 & (~RF_ENAB));						// transfer channel data to PLL (turn off RF)
					}
				}
				cwmask >>= 1;												// update cwmask
			}
		}
		// process serial input
		if(gotcr()){												// wait for a cr ('\r') to be entered
			z_temp = 0;												// pre-clear "z" flag
			do{
				c = getch00();										// skip over leading control chrs
			}while((c <= ESC) && (c != '\0'));
			putch(c);
			switch(c){
				default:											// invalid command chr
					do{
						c = getch00();								// skip to EOL or end of input
					}while((c != '\r') && (c != '\0'));
				case '\r':											// empty line
					break;
				
				case 'i':
					rptr = (U8 code *) SECTCW_ADDR;					// set pointer to start of CW msg
					rptr += MSG_IDX;
					if((valid_cw(rptr)) && !((diode_matrix[DIT_IDX] == 0xff) && (diode_matrix[DIT_IDX+1] == 0xff))){
						putss("\nRe-init");							// post prompt
						erase_hold = FALSE;							// clear erase hold
						cwptr = &diode_matrix[MSG_IDX];				// reset cw pointer
						cwmask = 0x80;
						msgtimer = 0;
						cw_on = 1;
						CHrun = 0xff;
						ipl2 = 1;									// re-init PLL and dit time
					}else{
						putss("msg invalid\n");
					}
					break;
				
				case 'E':
					c = getch00();
					//erase first 3 sectors of FLASH where the channel data lives				
					while(getch00());								// clean out serial buffer
					if(c == 'C'){
						putss("\nErase All PLL data, Press \"Y\" to cont...");		// Are you sure? prompt
						fptr = (U8 xdata *)SECTCH_ADDR;				// set pointer to 1st sector of ch data
						j = 1;										// # sect to erase
					}
					if(c == 'M'){
						putss("\nErase CW Message, Press \"Y\" to cont...");		// Are you sure? prompt
						fptr = (U8 xdata *)SECTCW_ADDR;				// set pointer to 1st sector of msg data
						j = 3;										// # sect to erase
					}
					waittimer = 5000;								// set 5 sec timer
					while((gotch00() == '\0') && (waittimer != 0)); // wait for user input
					if(getch00() == 'Y'){							// if timeout, getch00 will return '\0' which will abort
						putss("\nerasing:");
						for(i=0; i<j; i++){
							if(fptr < (FLASH_END - 1)){
								erase_flash(fptr);					// erase sector
							}
							fptr += SECTOR_SIZE;					// set next sector
							putch('.');								// display progress
						}
						putss("Erased!\n");							// announce completion
						erase_hold = TRUE;							// set erase hold
						cw_on = 0;
						send_spi32(reg4_32);						// keyup
					}else{
						putss("Aborted.\n");						// abort msg
					}
					break;
				
				case 'e':
					// echo terminal buffer
					do{
						c = getch00();				 				// get byte
						putch(c);									// echo
					}while(c != '\r');								// until cr
					putch('\n');
					break;
				
				case 'Q':
					// error querry/clear
					if(loaderr){									// display error status
						putss("\nLoad errors\n");
					}else{
						putss("\nNO errors\n");
					}
					if(gotch00()){
						if(getch00() == 'C'){
							putss("Err status cleared\n");
							loaderr = 0;							// clear error status
						}
					}
					putch('\n');
					break;

				case 'z':
					z_temp = 1;										// set "z" cmd flag
				case 'c':
					// calc CRC16 on channels
					c = getch00();									// see if for CW msg
					temp_crc = 0;
					tempword = 0;
					if(c == 'm'){
						rptr = (U8 code *) SECTCW_ADDR;
						do{
							tempbyte = *rptr++;
							tempword <<= 8;
							tempword |= (U16)tempbyte & 0x00ff;
							temp_crc = calcrc(tempbyte,temp_crc);
						}while((tempword != CW_STOPW) && (rptr < (FLASH_END - 1)));
					}else{
						rptr = (U8 code *) CHAN_ADDR;
						for(ii=0; ii<(24 * NUM_CHAN); ii++){
							temp_crc = calcrc(*rptr++,temp_crc);
						}
					}
					if(z_temp){										// do CRC compare if true
						if(c == 'm'){
							putss(" CMP CW CRC...");
						}else{
							putss(" CMP CRC16...");
						}
						j = 1;										// preset PASS
						if(getbyte(&tempbyte)) j = 0;				// 1st CRC byte -- compare data fail
						if((U8)(temp_crc >> 8) != tempbyte) j = 0;	// crc fail
						if(getbyte(&tempbyte)) j = 0;				// 2nd CRC byte -- compare data fail
						if((U8)(temp_crc & 0xff) != tempbyte) j = 0;	// crc fail
						wait(1000);									// wait 1 sec
						if(j){
							putss("\nPASS\n");
						}else{
							loaderr = 1;							// set global fail
							putss("\nFAIL\n");
						}
					}else{
						if(c == 'm'){
							putss("\nCW CRC = 0x");
						}else{
							putss("\nCRC16 = 0x");
						}
						put_hex((U8)(temp_crc >> 8));
						put_hex((U8)(temp_crc & 0xff));
						putss("\n");
					}
					break;
				
//				case 't':
				case 'M':
					// program reg
					// syntax: Mxxaaaaaaaabbbbbbbbccccccccddddddddeeeeeeeeffffffff
					// First, validate buffered data (xfr to temp array)
					k = c;											// save cmd chr
					flag = TRUE;									// default to data good
					temp_active = 0;								// default to temp = inactive
					c = getch00();
					if((c < '0') || (c > '9')){						// check for valid bcd
						flag = FALSE;
					}
					i = (c & 0x0f) << 4;							// ms nyb
					c = getch00();
					if((c < '0') || (c > '9')){
						flag = FALSE;
					}
					i |= (c & 0x0f);								// ls nyb
					pgm_chnum = conv_to_chnum(i);					// convert BCD to hex
					if(pgm_chnum >= NUM_CHAN){
						flag = FALSE;								// error
					}
					goteol = 0;
					i = 0;											// init reg counter
					while(flag && !goteol){
						goteol = getbyte(&tempbyte); 				// get byte
						if(!goteol){
							temp_chan[i++] = tempbyte;
						}
						if(i == MAX_REG){
							goteol = 1;								// force end if 24 bytes
						}else{
							if(goteol) flag = FALSE;				// end of data too soon, error
						}
					}
					// Program data to FLASH
					if(flag && (k == 'M')){
						fptr = (U8 xdata *) CHAN_ADDR;				// set pointer to 1st ch
						fptr += 24 * pgm_chnum;						// jump to ch#
						for(i=0; i<24; i++){
							wr_flash(temp_chan[i], fptr++);
						}
						putss("Chan pgmd!\n");						// announce completion
					}else{
						if(flag){
							temp_active = 1;						// temp channel active
							putss("Temp reg pgmd\n");				// announce temp reg programmed
						}else{
							putss("ERROR!\n");						// announce err
							loaderr = 1;							// set error
						}
					}
					// Complete
					break;
			
				case 'r':
					// read reg
					// syntax: rxx
					flag = TRUE;
					goteol = TRUE;
					putss("\n");
					c = getch00();
					if(c == '-'){
						i = NUM_CHAN;								// send all chnnels
						j = 0;
						pgm_chnum = conv_to_chnum(j);				// convert BCD to hex
					}else{
						if((c < '0') || (c > '9')){					// check for valid BCD
							flag = FALSE;
						}
						i = (c & 0x0f) << 4;						// ms nyb
						c = getch00();
						if((c < '0') || (c > '9')){
							flag = FALSE;
						}
						i |= (c & 0x0f);							// ls nyb
						pgm_chnum = conv_to_chnum(i);				// convert BCD to hex
						if(pgm_chnum >= NUM_CHAN){
							flag = FALSE;							// error
							pgm_chnum = NUM_CHAN;
						}
						i = 1;										// just send 1 chan
						j = pgm_chnum;
					}
					// read data from FLASH
					if(flag){
						if(goteol){
							rptr = (U8 code *) CHAN_ADDR;			// set pointer to 1st ch
							rptr += 24 * pgm_chnum;					// jump to ch#
						}
						do{
							if(goteol){
								putch('M');							// pre-amble
								put_dec(j++);						// print ch#
								putch(' ');
							}else{
								putss("t00 ");						// temp chan preamble
							}
							tempbyte = 0;							// use tempbyte to format register fields
							for(k=0; k<24; k++){
								if(goteol){
									put_hex(*rptr++);				// display FLASH data
								}else{
									put_hex(temp_chan[k]);			// display temp reg data
								}
								if(++tempbyte == 4){
									putch(' ');						// insert some formatting between 32bit words
									tempbyte = 0;
								}
							}
							putss("\n");
						}while(--i != 0);
					}else{
						putss("CH_ERR!\n");
					}
					break;
			
				case 'R':
					// read CW msg
					// syntax: R
					flag = TRUE;
					goteol = TRUE;
					putss("\n");
					j = 28;
					ii = 0;
					// read data from FLASH
					if(flag){
						rptr = (U8 code *) SECTCW_ADDR;				// set pointer to 1st ch
						tempbyte = 0;								// init end of message flag
						tempbyte2 = 0;								// init cmd det flag
						do{
							putch('C');								// pre-amble
							put_hex((U8)(ii >> 8));					// print idx
							put_hex((U8)(ii & 0xff));
							putch(' ');
							do{
								i = *rptr++;						// get msg byte
								if(tempbyte2 == CW_STOP){			// if last byte == stop &&
									if(i == CW_EOM){				// this byte == EOM,
										tempbyte = CW_STOP;			// end the whole listing
									}
								}
								put_hex(i);							// display msg data
								if(ii < MSG_IDX){
									tempbyte2 = 0;					// no CW stop in 1st 5 bytes
								}else{
									tempbyte2 = i;					// save current chr for EOM det (requires 2 different chrs to end)
								}
								ii++;								// next index
								j--;
								if((ii == MSG_IDX) || (j == 0) || (tempbyte == CW_STOP)){
									putss("\n");					// insert some formatting between lines
									j = 28;							// reset line byte counter
								}
								if(rptr >= (FLASH_END - 1)){
									tempbyte = CW_STOP;				// end of flash, terminate listing
								}
							}while((tempbyte != CW_STOP) && (j != 28) && (ii != MSG_IDX));
						}while(tempbyte != CW_STOP);
					}
					break;

				case 'C':
					// program message string
					// syntax: Caaaa xxyyzz...
					//			aaaa = index to message string
					//			xx/yy/zz = ASCII hex byte pairs to program
					flag = TRUE;									// default to data good
					goteol = getbyte(&i);							// get upper
					goteol |= getbyte(&j);							// get lower
					if(goteol){
						flag = FALSE;								// index error
					}
					ii = ((U16)i << 8) | ((U16)j & 0x00ff);			// combine to get index
					fptr = ((U8 xdata *) SECTCW_ADDR);	 			// set pointer to 1st msg array
					fptr += ii;										// set index for incoming 
					while(flag && !goteol){							// program string to flash
						goteol = getbyte(&j);						// get data
						if(!goteol){
							wr_flash(j, fptr);
/*							if(*fptr != j){
								flag = FALSE;
								putss("vfy err sb/is: ");
								put_hex(j);
								putss("/");
								put_hex(*fptr);
								putss("\n");
							}*/
							fptr += 1;
						}
						if(fptr == ((U8 xdata *) FLASH_END)){
							flag = FALSE;							// force end if overrun
						}
					}
					if(flag){
						putss("Chan pgmd!\n");						// announce completion
					}else{
						putss("ERROR!\n");							// announce err
						loaderr = 1;								// set error
					}
					// Complete
					break;
			
				case 'l':
				case 'L':
					// read PLL lock bit
					// syntax: l, return "1" or "0"
					if(MISO == PLL_LOCK){
						putss("1\n");
					}else{
						putss("0\n");
					}
					break;

				case '?':
					// Help screen
					putss("\nOrion Help Ver0.26 11-19-18\n");
					putss("Mnna..f: PGM CH nn\n");
					putss("EC: erase all CH\t\tEM: erase msg\n");
					putss("c: disp ch CRC16 (0x1021 poly)\tz hhhh: cmp CRC16\n");
					putss("cm: disp msg crc \t\tzm hhhh: cmp msg crc\n");
					putss("rnn: read CH nn\t\t\tr-: read all CH\n");
					putss("Q: querry errs\t\t\tQC: Clr errs\n");
					putss("i: re-send CH\t\t\te: echo cmdln\n");
					putss("Ciiiidd..: Pgm CWmsg @IDX iiii\tL: read PLL lock stat\n");
					putss("R: read CW msg\n");
					break;
			}
			cleanline();											// clean up rest of current line
			putss("\nbkn>");										// post prompt
		}
	}
}  // end main()

// *********************************************
//  *************** SUBROUTINES ***************
// *********************************************

//-----------------------------------------------------------------------------
// valid_cw()
//-----------------------------------------------------------------------------
//
// scans CW message space for a CW_STOP.  If none found before end of FLASH,
//	return FALSE, else TRUE
//
U8 valid_cw(U8 code * ptr){
	U8	i = FALSE;		// temp
	
	while((ptr != (FLASH_END - 1)) && !i){
		if(*ptr++ == CW_STOP){
			i = TRUE;
		}
	}
	return i;
}

//-----------------------------------------------------------------------------
// send_spi32
//-----------------------------------------------------------------------------
//
// sends 32 bit word to ADS4351 via portB bit-bang SPI
//
void send_spi32(U32 plldata){
#ifdef BB_SPI
	U32	mask;

	nPLL_LE = LE_ON;								// latch enab = low to clock in data
	for(mask = 0x80000000; mask != 0; mask >>= 1){	// start shifting 32 bits starting at MSb
		if(mask & plldata) MOSI = 1;				// set MOSI
		else MOSI = 0;
		delay_us(HAFBIT);							// delay half clock
		SCK = 1;									// clock = high
		delay_us(HAFBIT);							// delay remaining half
		SCK = 0;									// clock low
	}
	delay_us(HAFBIT);								// delay for LE
	nPLL_LE = LE_OFF;								// latch enab = high to latch data
	delay_us(HAFBIT);								// pad intra-word xfers by a half bit
	return;
	
#else
	U8	i;	// loop temps
	U8	d;
	union Data32 {	// temp union to parse out 32b word to 8b pieces
	   U32 l;
	   U8 b[4];
	} pllu;  

	pllu.l = plldata;
	nPLL_LE = LE_ON;								// latch enab = low to clock in data
	delay_us(SH_DLY);								// pad intra-word xfers by a half bit
	for(i=0; i < 4; i++){							// shifting 32 bits 8 bits at a time
		d = (U8)(pllu.b[i]);
		while(!(SPI0CN & 0x02));					// wait for buffer to clear
		SPI0DAT = d;
	}
	while(!(SPI0CN & 0x02));						// wait for buffer to clear
	delay_us(BYTDLY);								// delay for LE
	nPLL_LE = LE_OFF;								// latch enab = high to latch data
	delay_us(SH_DLY);								// delay for RC pullup on revC CS line
	return;
#endif
}

//-----------------------------------------------------------------------------
// delay_us
//-----------------------------------------------------------------------------
//
// sets delay for spi SCK and for CS setup/hold
//
	// BitBang version uses for-loop to establish ~~200 us delay
/*void delay_halfbit(void){
	U16	i;
	
	for(i = 0; i < 500;){							// cheezy for-next-loop to set delay for bit-bang-spi
													// 500 loops is approx 200us
		i += 1;
	}
	return;	
}*/

void delay_us(U16 dly){								// for HWSPI, this fnd is just used to set reg-reg xfr delay

	TH0 = (dly >> 8);								// prep timer registers for delay
	TL0 = (dly & 0xFF);
	TF0 = 0;
	TR0 = 1;										// start timer
	while(TF0 == 0);								// loop
	TR0 = 0;										// stop timer
	return;	
}

//-----------------------------------------------------------------------------
// send_spi8
//-----------------------------------------------------------------------------
//
// sends 8 bit DAC word to LTC2630 DAC via SPI
//	Uses SPI for clock and data, and KEYOUT for /CS
//	daccmd is the DAC command word
//
void send_spi8(U8 daccmd, U8 dacdata){
#ifdef BB_SPI
	U8	bi;			// loop temp
	U8	mask;
	U8	temp;

	KEYOUT = 0;										// latch enab = low to clock in data
	delay_us(SH_DLY);								// delay for LE
	for(bi=0; bi<3; bi++){
		temp = 0;
		if(bi == 0) temp = daccmd;
		if(bi == 1) temp = dacdata;
		for(mask = 0x80; mask != 0x00; mask >>= 1){	// start shifting 24 bits starting at MSb
			if(mask & temp) MOSI = 1;				// set MOSI
			else MOSI = 0;
			delay_us(HAFBIT);							// delay half clock
			SCK = 1;									// clock = high
			delay_us(HAFBIT);							// delay remaining half
			SCK = 0;									// clock low
		}
	}
	delay_us(SH_DLY);								// delay for LE
	KEYOUT = 1;										// latch enab = high to latch data
	delay_us(SH_DLY);								// pad intra-word xfers by a half bit
	return;
	
#else
	KEYOUT = 0;										// latch enab = low to clock in data
	delay_us(SH_DLY);								// pad intra-word xfers by a half bit
	while(!(SPI0CN & 0x02));						// wait for buffer to clear
	SPI0DAT = daccmd;								// send cmd
	while(!(SPI0CN & 0x02));						// wait for buffer to clear
	SPI0DAT = dacdata;								// send data
	while(!(SPI0CN & 0x02));						// wait for buffer to clear
	SPI0CN = SPI0CN & 0x7f;							// clear irq
	SPI0DAT = 0;									// send pads
	while(!(SPI0CN & 0x80));						// wait for xfr dne
	delay_us(BYTDLY);								// delay for LE
	KEYOUT = 1;										// latch enab = high to latch data
	delay_us(SH_DLY);								// delay for RC pullup on revC CS line
	return;
#endif
}

//-----------------------------------------------------------------------------
// ramp() sends data to DAC to do 5ms ramp up (1) or dn (0)
//	
//-----------------------------------------------------------------------------
//
//	Index	%Full scale		RAW DAC (8b)	Chart of ramp (1ms per step)
//	000		0				0				*
//	001		5.3				14				 *
//	010		15.8			40				    *
//	011		36.8			94				         *
//	100		63.1			161				                *
//	101		84.2			215				                     *
//	110		94.7			241				                        *
//	111		100				255				                         *

//U8	code ramp_array[] = { 0, 14, 40,  94, 161, 215, 241, 255};
//U8	code ramp_array[] = {25, 37, 61, 110, 170, 219, 243, 255};		// ramp table with 0.6V low-end offset
#define	RAMPLEN 8

void ramp(U8 updn){
	U8	ramp_idx;			// ramp array index
	S8	i;					// temp
	
	if(updn){
		for(i=0, ramp_idx=RTBL_IDX; i<RAMPLEN; i++, ramp_idx++){
			send_spi8(DAC_SET, diode_matrix[ramp_idx]);
			delay_us(MS_DLY);							// wait 1ms
		}
	}else{
		for(i=0, ramp_idx=RTBLE_IDX-1; i<RAMPLEN; i++, ramp_idx--){
			send_spi8(DAC_SET, diode_matrix[ramp_idx]);
			delay_us(MS_DLY);							// wait 1ms
		}
	}
	return;
}

//-----------------------------------------------------------------------------
// setkeyout() sets/clears keyout or does DAC ramp according to key mode status
//	
//-----------------------------------------------------------------------------
//
void setkeyout(U8 updn){
static	U8	last_updn;			// ramp array index

	if(updn == 0xAA){
		last_updn = 0;
		return;
	}
	if(dacmode){
		if(last_updn != updn){
			ramp(updn);												// perform ramp up or dn
			last_updn = updn;
		}
	}else{
		if(updn){
			KEYOUT = (diode_matrix[KEY_IDX] & KEY_MASK);			// KEYOUT = tone on
			delay_us(MS_DLY);
		}else{
			KEYOUT = (diode_matrix[KEY_IDX] & KEY_MASK) ^ 0x01;		// KEYOUT = tone off
			wait((U16)diode_matrix[RMP_IDX] & 0xff);
		}
	}
	return;
}

//-----------------------------------------------------------------------------
// calcrc() calculates incremental crcsum using defined poly
//	(xmodem poly = 0x1021).  oldcrc = 0x0000 for first call, c = data byte
//-----------------------------------------------------------------------------
U16 calcrc(U8 c, U16 oldcrc){
#define	POLY 0x1021	// xmodem polynomial

	U16 crc;
	U8	i;
	
	crc = oldcrc ^ ((U16)c << 8);
	for (i = 0; i < 8; ++i){
		if (crc & 0x8000) crc = (crc << 1) ^ POLY; //0x1021;
		else crc = crc << 1;
	 }
	 return crc;
}

//-----------------------------------------------------------------------------
// wait() uses ms timer to establish a defined delay
//-----------------------------------------------------------------------------

void wait(U16 waitms)
{

    waittimer = waitms/MS_PER_TIC;					// convert ms to timer ticks
	if(waittimer == 0) waittimer = 1;				// force at least 1 tick
    while(waittimer != 0);							// wait for timer to expire
	return;
}

//-----------------------------------------------------------------------------
// put_hex
//-----------------------------------------------------------------------------
//
// sends 8b hex to serial port as ASCII
//
void put_hex(U8 dhex){
	char	c;
	
	c = (dhex >> 4) + '0';							// convert low nybble to BCD
	if(c > '9') c += 'A' - '9' - 1;					// if hex, convert to "A" - "F"
	putch(c);										// send to terminal
	c = (dhex & 0x0f) + '0';						// repeat above for hi nyb
	if(c > '9') c += 'A' - '9' - 1;
	putch(c);
	return;
}

//-----------------------------------------------------------------------------
// put_dec
//-----------------------------------------------------------------------------
//
// sends 8b hex to serial port as decimal ASCII
//
void put_dec(U8 dhex){
	U8		d;		// bcd temp
	U8 		i;		// temps
	U8 		j;

	if(dhex > 99){
		putss(">>");								// error display
	}else{
		j = dhex / 10;								// convert to BCD
		i = dhex - (j * 10);
		d = (j << 4) | i;
		put_hex(d);									// display as hex = display as BCD
	}
	return;
}

//--------------------------------------------------------------------------------------
// getbyte() returns 1 if no EOL is encountered: processes ASCII byte into pointer location.
//	skips spaces.  Other chars are data error.
//	returns 1 if EOL or data error
//--------------------------------------------------------------------------------------
U8 getbyte(U8* dataptr){
	U8	c;		// temps
	U8	cc;
	U8	temp;
	U8	rtn = 0;	// default to !goteol rtn

	do{												// 1st nyb, skip spaces & trap EOL
		c = (U8)getch00();
	}while(whitespc(c));
	do{												// 2nd nyb, skip spaces & trap EOL
		cc = (U8)getch00();
	}while(whitespc(cc));
	if((c <= ESC) || (cc <= ESC)){
		rtn = 1;									// any cntl chr is interpreted as EOL
	}else{
		temp = convnyb(c);
		if(temp > 0x0f) rtn = 2;					// error data
		c = convnyb(cc);
		if(c > 0x0f) rtn = 2;						// error data
	}
	if(!rtn){
		*dataptr = (temp << 4) | c;
	}
	return rtn;
}

//--------------------------------------------------------------------------------------
// whitespc() returns 1 if chr = space, comma, or tab, else returns 0
//--------------------------------------------------------------------------------------
U8 whitespc(char c){
	U8 rtn = 0;	// temp rtn, default false

	switch(c){
		case ' ':
		case ',':
		case '\t':
			rtn = 1;								// if defined whitespace chr, return true
			break;
	}
	return rtn;
}

//--------------------------------------------------------------------------------------
// convnyb() converts ASCII to bin nybble.  returns 0xff if non-hex ascii
//--------------------------------------------------------------------------------------
U8 convnyb(U8 c){
	U8	rtn = 0xff;

	if((c >= 'a') && (c <= 'z')) c -= 'a' - 'A';	// upcase
	if((c >= '0') && (c <= 'F')){					// 1st validation step
		rtn = (c - '0');							// passed, convert to BIN
		if(rtn > 9){
			rtn -= 'A' - '9' - 1;					// convert letters
			if(rtn < 0x0a) rtn = 0xff;				// 2nd validation step (if true, fail)
		}
	}
	return rtn;										// return result
}

//--------------------------------------------------------------------------------------
// get_chan() returns pointer to R5 of specified channel data
//--------------------------------------------------------------------------------------
U32 *get_chan(U8 chanum){
	U32 *ptemp;
	
	ptemp = pll_ch + (6 * chanum) + 5;				// channel pointer is base + #regs * ch# + 5
	return ptemp;
}

//--------------------------------------------------------------------------------------
// conv_to_chnum() converts dual-BCD port bits to channel# (0 = first channel)
//	if invalid BCD, returns ch#0
//--------------------------------------------------------------------------------------

U8 conv_to_chnum(U8 portbits){
	U8	i;				// temp return value
	U8	r;				// temp regs
	U8	s;
	
	r = portbits & 0x0f;							// get low BCD nyb
	s = (portbits >> 4) & 0x0f;
	if((r > 9) || (s > 9)){
		i = 0;										// bcd error, default to ch#0
	}else{
		i = r + (s * 10);							// get integer version of 2 digit BCD
	}
//	if(i > MAX_CHAN){
//		i = 0;										// if channel error, set to 0
//	}
	return i;
}

//-----------------------------------------------------------------------------
// Timer2_ISR
//-----------------------------------------------------------------------------
//
// Called when timer 2 overflows (NORM mode):
//      updates app timers @ 10ms rate
//		rate = (sysclk/12) / (65536 - TH:L)
//
//-----------------------------------------------------------------------------

void Timer2_ISR(void) interrupt 5 using 2
{

    TF2H = 0;                           			// Clear Timer2 interrupt flag
    if(waittimer != 0){                 			// g.p. delay timer
        waittimer--;
    }
    if(msgtimer != 0){                 				// g.p. delay timer
        msgtimer--;
    }
	if(cw_on){
		if(--elem_timer == 0){
			elem_flag = 1;
			elem_timer = elem_time;
		}
	}
}

#undef IS_MAINC
//**************
// End Of File
//**************
