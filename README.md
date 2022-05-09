# orion_bkn
Orion Beacon application

Operates the Orion ADF-4351 as a Morse code (On-Off Keyed) message device.

SiLabs C8051 and Kiel (V5) Project for the C8051F530/ADF-4351. Configures the ADF-4351 via a SPI interface.  Uses either a BCD logic interface to set a channel (00 - 16) or can accept serial commands (9600 baud) to set channel.  Serial command line format is used to set channels data, message data, and perform FLASH maintenance (programming and erasure).  Requires channel and message data to be previously determined using either the ADF PLL application software, or a spreadsheet (available at the hardware project URL, below).

See https://ke0ff.github.io/Orion/index.html for project hardware details.
