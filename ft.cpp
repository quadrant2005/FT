 
// ft.cpp : Read and write EEPROM configuration data from FT817 and FT897
//
// Usage: ft <byte1> <byte2>, where byte1 and byte2 are the two hex values to put into address 4 and address 5 in the EEPROM
//        ft -r, reads the current EEPROM values only, does not write.
// 
// License: Freeware. If you break it, you own both halves. If you use it, please acknowledge the original author.
//                    If you screw your radio, your problem.
//                    There are no restrictions on distribution or copying, provided you mention the original author.
//					  If you modify the code, please send a copy back to me at vk2it@tpg.com.au
//					  I'd love to hear your comments and stories via a QSL card !
//
// Revision History
//
// Ver	Date		Who				Comments.
// 0.1	03/08/2003	Peter May		Original version
// 0.2	06/09/2003	Peter May		Updates for COM port selection and change my Call to VK2IT from VK2JCG!
// 1.0  27/03/2015  Richard Steele M6DVK (2E0DVK)

#include "stdafx.h"
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define READ_INFO		0xA7
#define READ_EEPROM	    0xBB 
#define WRITE_EEPROM	0xBC

#define LOCK_ON         0x00//[QBS]
#define SET_FREQUENCY   0x01//[QBS]

#define READ_FREQUENCY  0x03//[QBS]
#define SET_MODE        0x07//[QBS]
#define PTT_ON          0x08//[QBS]
#define LOCK_OFF        0x80//[QBS]
#define PTT_OFF         0x88//[QBS]

// '897 Jumper Values, inverted sense: 0 = Jumper, 1 = No Jumper, other bits unused/unknown.

// Byte 1:

#define FT897_J1001     0x40
#define FT897_J1002     0x20 
#define FT897_J1003     0x10 
#define FT897_J1007     0x04 
#define FT897_J1008     0x02 
#define FT897_J1009     0x01 

// Byte 2:

#define FT897_J1004     0x04 
#define FT897_J1005     0x02 
#define FT897_J1006     0x01 

//[QBS]s Moved here from Main to make global (this is not the best way to do this)
HANDLE			hCom;
unsigned long	count;
unsigned char	sbuf[6];
unsigned char	rbuf[32];
char			wbuf[32];
//[QBS]e
#define ON  1//[QBS]
#define OFF 0//[QBS]

#define Debugmode ON//[QBS]


void usage()
{
	printf( "Usage: ft [-#]               Reads the current EEPROM values only, does not\n" );
	printf( "                             write anything to the EEPROM\n\n" );
	printf( "       ft [-#] <B1> <B2>     Byte1 and byte2 are the two hex values\n" );
	printf( "                             to put into address 4 and address 5 in the EEPROM\n\n" );
	printf( "                             Optional: -#, use COM port COM#\n\n" );
	printf( "Example: ft 70 06            Writes the values 70 06 to the EEPROM via COM1:\n" );
	printf( "         ft -2               Reads the current values using COM2:\n" );
	printf( "         ft -2 F8 BF         Writes F8 BF using COM2:\n\n" );
	
	exit( 1 );
}

// Printcat simply prints the response from the radio in Hex.

void printcat( char *msg, unsigned long count, unsigned char *bfr )
{
	printf( "%s: ", msg );

	while( count )
	{
		printf( "%02X ", *bfr );
		count--;
		bfr++;
	}
	printf( "\n" );
}


// fromhex: Converts a HEX string to an unsigned char - only works for max of 2 character strings.

unsigned char fromhex( char *s )
{
	unsigned char rv = 0;

	if( *s >= '0' && *s <= '9' )
		rv = *s - '0';

//	*s = toupper( *s );//[QBS] why did toupper break i had to remove it
	if( *s >= 'A' && *s <= 'F' )
		rv = *s - 'A' + 10;

	*s++;

	if( ! *s )
		return( rv );

	rv = rv * 16;

	if( *s >= '0' && *s <= '9' )
		rv += *s - '0';

//	*s = toupper( *s );//[QBS] why did toupper break i had to remove it
	if( *s >= 'A' && *s <= 'F' )
		rv += *s - 'A' + 10;

	return( rv );
}

//====================================================================
//[QBS]s
unsigned char Read_Rx()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("00");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("E7");//Read Rx

	printf( "\nRead_Rx\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "Read_Rx(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);
if (Debugmode)
	{
		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
	}	
		if( rbuf[0] == 0 )
		{
			printf( "S0");
		}
		else if( rbuf[0] == fromhex("80"))
		{
			printf( "Squelch Closed");
		}
		else if( rbuf[0] == 1 )
		{
			printf( "S1");
		}
		else if( rbuf[0] == 2 )
		{
			printf( "S2");
		}
		else if( rbuf[0] == 3 )
		{
			printf( "S3");
		}
		else if( rbuf[0] == 4 )
		{
			printf( "S4");
		}
		else if( rbuf[0] == 5 )
		{
			printf( "S5");
		}
		else if( rbuf[0] == 6 )
		{
			printf( "S6");
		}
		else if( rbuf[0] == 7 )
		{
			printf( "S7");
		}
		else if( rbuf[0] == 8 )
		{
			printf( "S8");
		}
		else if( rbuf[0] == 9 )
		{
			printf( "S9");
		}
		else if( rbuf[0] == fromhex("0A") )
		{
			printf( "S9+10dB");
		}
		else if( rbuf[0] == fromhex("0B") )
		{
			printf( "S9+20dB");
		}
		else if( rbuf[0] == fromhex("0C") )
		{
			printf( "S9+30dB");
		}
		else if( rbuf[0] == fromhex("0D") )
		{
			printf( "S9+40dB");
		}
		else if( rbuf[0] == fromhex("2E") )// do i need to check this ? why repeated
		{
			printf( "S9+40dB");
		}
		else if( rbuf[0] == fromhex("0E"))
		{
			printf( "9+50dB");
		}
		else
		{
			printf( "UNKNOWN SIGNAL");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===============================================================
//[QBS]s
unsigned char Read_Tx()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("00");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("F7");//Read Tx

	printf( "\nRead_Tx\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "Read_Tx(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[0] == 0 )
		{
			printf( "S0");
		}
		else if( rbuf[0] == fromhex("BB"))
		{
			printf( "BB");
		}
		else
		{
			printf( "UNKNOWN SIGNAL");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//================================================================
//[QBS]s
unsigned char Read_Freq_and_Mode()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("00");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("03");//Read

	printf( "Read Frequency and Mode\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned frequency\n\n" );
		printcat( "DEBUG Read = ", count, rbuf );
	}
		else
	{
		printcat( "Read_Freq_and_Mode(1)Radio response to write command is ", count, rbuf );

		printf( "DEBUG rbuf[4]='%d' ",rbuf[4]);

		
		if( rbuf[4] == 0 )
		{
			printf( "LSB");
		}
		else if( rbuf[4] == 1 )
		{
			printf( "USB");
		}
		else if( rbuf[4] == 2 )
		{
			printf( "CW");
		}
		else if( rbuf[4] == 3 )
		{
			printf( "CWR");
		}
		else if( rbuf[4] == 4 )
		{
			printf( "AM");
		}
		else if( rbuf[4] == 6 )
		{
			printf( "WFM");
		}
		else if( rbuf[4] == 8 )
		{
			printf( "FM");
		}
		else if( rbuf[4] == 10 )
		{
			printf( "DIG");
		}
		else if( rbuf[4] == 82 )
		{
			printf( "CW-N");
		}
		else if( rbuf[4] == 88 )
		{
			printf( "FM-N");
		}
		else if( rbuf[4] == 252 )
		{
			printf( "PKT");
		}
		else
		{
			printf( "MODE UNKNOWN");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===========================================================================
//[QBS]s
unsigned char Read_EEPROM_0X0055()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("55");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("BB");//Read EEPROM

	printf( "\nRead_EEPROM_0X0055\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "EEPROM_0X0055(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[0] == 0 )
		{
			printf( "S0");
		}
		else if( rbuf[0] == fromhex("C1 00"))
		{
			printf( "C1 00??");
		}
		else
		{
			printf( "UNKNOWN");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===========================================================================
//[QBS]s
unsigned char Read_EEPROM_0X0057()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("57");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("BB");//Read EEPROM

	printf( "\nRead_EEPROM_0X0057\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "Read_Tx(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[0] == fromhex("A0 13") )
		{
			printf( "NB=on");
		}
		else if( rbuf[0] == fromhex("80 13"))
		{
			printf( "NB=off");
		}
		else
		{
			printf( "UNKNOWN");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===========================================================================
//[QBS]s
unsigned char Read_EEPROM_0X0059()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("59");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("BB");//Read EEPROM

	printf( "\nRead_EEPROM_0X0059\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "Read_Tx(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[0] == 0 )
		{
			printf( "S0");
		}
		else if( rbuf[0] == fromhex("FF"))
		{
			printf( "FF");
		}
		else
		{
			printf( "UNKNOWN");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===========================================================================
//[QBS]s
unsigned char Read_EEPROM_0X005F()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("5F");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("BB");//Read EEPROM

	printf( "\nRead_EEPROM_0X005F\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "Read_Tx(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[0] == 0 )
		{
			printf( "S0");
		}
		else if( rbuf[0] == fromhex("FF"))
		{
			printf( "FF");
		}
		else
		{
			printf( "UNKNOWN");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===========================================================================
//[QBS]s
unsigned char Read_EEPROM_0X0065()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("65");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("BB");//Read EEPROM

	printf( "\nRead_EEPROM_0X0065\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "Read_Tx(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[0] == 0 )
		{
			printf( "S0");
		}
		else if( rbuf[0] == fromhex("FF"))
		{
			printf( "FF");
		}
		else
		{
			printf( "UNKNOWN");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]s
//===========================================================================
//[QBS]s
unsigned char Read_EEPROM_0X0079()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("79");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("BB");//Read EEPROM

	printf( "\nRead_EEPROM_0X0079\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "Read_Tx(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[0] == 0 )
		{
			printf( "S0");
		}
		else if( rbuf[0] == fromhex("FF"))
		{
			printf( "FF");
		}
		else
		{
			printf( "UNKNOWN");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===========================================================================
//[QBS]s
unsigned char Read_EEPROM_0X00B1()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("B1");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("BB");//Read EEPROM

	printf( "\nRead_EEPROM_0X00B1\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "Read_Tx(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[0] == 0 )
		{
			printf( "S0");
		}
		else if( rbuf[0] == fromhex("FF"))
		{
			printf( "FF");
		}
		else
		{
			printf( "UNKNOWN");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===========================================================================
//[QBS]s
unsigned char Read_EEPROM_0X00BB()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("BB");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("BB");//Read EEPROM

	printf( "\nRead_EEPROM_0X00BB\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[2] == 00)// && rbuf[1] == 00 && rbuf[2] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf);
//		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);

		printf( "S0");
	}
		else
	{
		printcat( "Read_EEPROM(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[2] == 00 )
		{
			printf( "00");
		}
		else if( rbuf[2] == fromhex("FF"))
		{
			printf( "FF");
		}
		else
		{
			printf( "UNKNOWN");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===========================================================================
//[QBS]s
unsigned char Read_EEPROM_0X00BD()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("BD");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("BB");//Read EEPROM

	printf( "\nRead_EEPROM_0X00BD\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "Read_Tx(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[0] == 0 )
		{
			printf( "S0");
		}
		else if( rbuf[0] == fromhex("FF"))
		{
			printf( "FF");
		}
		else
		{
			printf( "UNKNOWN");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===========================================================================
//[QBS]s
unsigned char Read_EEPROM_0X0485()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("04");
	sbuf[1] = fromhex("85");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("BB");//Read EEPROM

	printf( "\nRead_EEPROM_0X0485\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "Read_Tx(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[0] == 0 )
		{
			printf( "S0");
		}
		else if( rbuf[0] == fromhex("FF"))
		{
			printf( "FF");
		}
		else
		{
			printf( "UNKNOWN");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===========================================================================
//[QBS]s
unsigned char Read_EEPROM_0X0487()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("04");
	sbuf[1] = fromhex("87");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("BB");//Read EEPROM

	printf( "\nRead_EEPROM_0X0487\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "Read_Tx(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[0] == 0 )
		{
			printf( "S0");
		}
		else if( rbuf[0] == fromhex("FF"))
		{
			printf( "FF");
		}
		else
		{
			printf( "UNKNOWN");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===========================================================================
//[QBS]s
unsigned char Read_EEPROM_0X048A()	
{
// Read Frequency and Mode

	sbuf[0] = fromhex("04");
	sbuf[1] = fromhex("8A");
	sbuf[2] = fromhex("00");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("BB");//Read EEPROM

	printf( "\nRead_EEPROM_0X048A\n");
	printf( "SENDING CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}


		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if(rbuf[0] == 00 && rbuf[1] == 00)
	{
		printf( "Error: Radio did not accept command, should have returned received signal\n\n" );
		printcat( "DEBUG Read = \n", count, rbuf );
		printf( "S0");
	}
		else
	{
		printcat( "Read_Tx(1)Radio response to write command is ", count, rbuf );

//		printf( "DEBUG rbuf[0]='%d' \n",rbuf[0]);

		printf( "DEBUG rbuf='[3]%d',[2]'%d',[1]'%d',[0]'%d' ",rbuf[3],rbuf[2],rbuf[1],rbuf[0]);
		
		if( rbuf[0] == 0 )
		{
			printf( "S0");
		}
		else if( rbuf[0] == fromhex("FF"))
		{
			printf( "FF");
		}
		else
		{
			printf( "UNKNOWN ");
		}
	}
//	fgets (wbuf, 31, stdin);//[QBS]
	return (sbuf[6]);
}
//[QBS]e
//===========================================================================
//[QBS]s
unsigned char Write_Freq()	
{
//	unsigned char	sbuf[6];
	sbuf[0] = fromhex("00");
	sbuf[1] = fromhex("52");
	sbuf[2] = fromhex("50");			
	sbuf[3] = fromhex("00");			
	sbuf[4] = fromhex("01");//Write

	printf( "\nSet Frequency\n");
/*
		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )//from 4 to 5
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}
*/

printf( "Sending CAT command: %02x %02x %02x %02x %02x to the Radio\n", sbuf[0], sbuf[1], sbuf[2], sbuf[3] ,sbuf[4]);

	if( WriteFile( hCom, sbuf, 5, &count, NULL ) == 0 )
	{
		printf ("WriteFile failed with error %d.\n", GetLastError());
		return (1);
	}

	if( count != 5 )
	{
		printf( "Write to the radio failed! RC = %d\n", count );
		return 0;
	}

//	printcat( "Write_Freq(1) Radio replied with= ", count, rbuf );

		if( ReadFile( hCom, rbuf, 5, &count, NULL ) == 0 )
	{
		printf ("ReadFile failed with error %d.\n", GetLastError());
		return (1);
	}

//	printcat( "Write_Freq(2)Radio replied with= ", count, rbuf );

	if( rbuf[0] != 0)
		printf( "Error: Radio did not accept command, should have returned 00\n\n" );
	else
	{
		printcat( "DEBUG Write_Freq(1)Radio replied with= ", count, rbuf );
		printf( "Radio accepted write EEPROM command\n\n" );
		return (sbuf[6]);
	}
	return (sbuf[6]);
}
//[QBS]e


//===========================================================================
	
int main(int argc, char *argv[] )
{
	DCB				dcb;
//	HANDLE			hCom;
	BOOL			fSuccess;
	COMMTIMEOUTS	cto;

	int				readflag;
	int				comport = 1;//1
	char			cpstr[8];
	char			*argp;
//	unsigned long	count;

//	unsigned char	sbuf[6];
//	unsigned char	rbuf[32];
//	char			wbuf[32];

	printf( "%s version 1.0 \n\n", argv[0]);
	printf( "This program uses undocumented CAT commands to read or update the EEPROM\n" );
	printf( "in the Yaesu FT817, FT857, and FT897 Radios.\n" );
	printf( "The program assumes that the radio CAT rate is set to 38400 baud\n\n" );
	printf( "Currently in BETA TESTING.\n" );
	printf ( "*** USE AT YOUR OWN RISK ***\n\n" );

	// check arguments

	if( ( argc > 1 ) && ( *argv[1] == '-' ) )		// Is the COM: port specified ?
	{
		argp = argv[1];
		++argp;				// skip over the -
		comport = atoi( argp );

		if( ( comport < 1 ) || ( comport > 9 ) )//qbs 9
			usage();

		argc--;				// we are done with this argument 
		argv++;				// skip to next

	}
	else
		comport = 1;//1//QBS

	sprintf( cpstr, "COM%d:", comport );
	
	if( argc == 1 )			// No arguments, read the EEPROM only.
		readflag = 1;
	else
	{
		readflag = 0;

		if( argc != 3 )			// must supply the two hex values
			usage();
	}

	// Setup the com port. This is a bit messy & from the Win32 API sample code.

	hCom = CreateFile( cpstr, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );

	if (hCom == INVALID_HANDLE_VALUE)
	{
		printf ("CreateFile failed opening %s with error %d.\n", cpstr, GetLastError());
		return (1);
	}

	fSuccess = GetCommState(hCom, &dcb);

	if (!fSuccess)
	{
		printf ("GetCommState failed with error %d.\n", GetLastError());
		return (1);
    }

	// Fill in the DCB: baud=38400,8,N,2
//Configure Serial Port
	dcb.BaudRate = CBR_38400;					// set the baud rate
	dcb.fBinary = TRUE;
	dcb.ByteSize = 8;							// data size, xmit, and rcv
	dcb.Parity = NOPARITY;						// no parity bit
	dcb.StopBits = TWOSTOPBITS;					// two stop bits
	dcb.fOutxCtsFlow = FALSE;					// no CTS output flow control
	dcb.fOutxDsrFlow = FALSE;					// no DSR monitoring for output
	dcb.fDtrControl = DTR_CONTROL_DISABLE;		// do not raise DTR, most interfaces TX the rig if this happens!
	dcb.fDsrSensitivity = FALSE;				// ignore DSR on input of data
	dcb.fOutX = FALSE;							// no XON/XOFF!
	dcb.fInX = FALSE;
	dcb.fNull = FALSE;							// do not discard NULLS
	dcb.fRtsControl = RTS_CONTROL_DISABLE;		// do not raise RTS.

	fSuccess = SetCommState(hCom, &dcb);

	if (!fSuccess)
	{
		printf ("SetCommState failed with error %d.\n", GetLastError());
		return (1);
	}

	cto.ReadIntervalTimeout = 100;				// max of 100ms between characters
	cto.ReadTotalTimeoutConstant = 0;
	cto.ReadTotalTimeoutMultiplier = 0;
	cto.WriteTotalTimeoutConstant = 0;
	cto.WriteTotalTimeoutMultiplier = 0;
	

	fSuccess = SetCommTimeouts( hCom, &cto );

	if ( !fSuccess )
	{
		printf( "SetCommTimeouts failed with error %d.\n", GetLastError());
		return (1);
	}

	printf ("Serial port %s successfully reconfigured.\n\n", cpstr );

//===========================
	printf( "\nReady\n" );
	Read_Freq_and_Mode();//[QBS]
	Read_Rx();//[QBS]


Read_Rx();//[QBS]
Read_Rx();//[QBS]
Read_Rx();//[QBS]
Read_Rx();//[QBS]
Read_Rx();//[QBS]



//	Read_Tx();//[QBS]
//	Read_EEPROM_0X0055();//[QBS]
//	Read_EEPROM_0X0057();//[QBS]
//	Read_EEPROM_0X0059();//[QBS]
//	Read_EEPROM_0X005F();//[QBS]
//	Read_EEPROM_0X0065();//[QBS]
//	Read_EEPROM_0X0079();//[QBS]
//	Read_EEPROM_0X00B1();//[QBS]
//	Read_EEPROM_0X00BB();//[QBS]
//	Read_EEPROM_0X00BD();//[QBS]
//	Read_EEPROM_0X0485();//[QBS]
//	Read_EEPROM_0X0487();//[QBS]
//	Read_EEPROM_0X048A();//[QBS]
//	Read_Tx();//[QBS]
//	Read_Freq_and_Mode();//[QBS]
//	Read_Tx();//[QBS]
//	Read_Rx();//[QBS]
//===========================


//	Write_Freq();//[QBS]
//	Read_Freq_and_Mode();//[QBS]

		fgets (wbuf, 31, stdin);//[QBS]
//Read_Rx();//[QBS]

	printf( "\nPRESS KEY TO EXIT\n\n" );
	fgets (wbuf, 31, stdin);//[QBS]

	CloseHandle( hCom );	// finished with config of port, now we use basic I/O
	return 0;
}

//BIT I MAY USE ARE STORED BELOW
/*
	readflag=1;//[QBS]


	if( readflag )			// if we only want to read the EEPROM data, leave now.
		return 0;
	
	sbuf[2] = fromhex( argv[1] );
	sbuf[3] = fromhex( argv[2] );

	printf( "\nAbout to write the values %02x %02x to the EEPROM\n", fromhex( argv[1] ), fromhex( argv[2] ) );
	printf( "Are you SURE you want to do this ?\nPlease answer YES to continue > " );

	fgets( wbuf, 31, stdin );

	if( strncmp( wbuf, "YES", 3 ) != 0 )
	{
		printf( "Aborted: you must enter YES in uppercase to continue and write to the EEPROM\n\n" );
		return 0 ;
	}
*/
