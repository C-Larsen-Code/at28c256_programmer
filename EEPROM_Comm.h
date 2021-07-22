/*************************************************************************
* This library was built to write data to a 28C256 256k Parallel EEPROM
* chip to be used as ROM for a W65C02 processor.
*************************************************************************/


#ifndef EEPROM_COMM_H
#define EEPROM_COMM_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <stdio.h>
#include <string.h>

#define SERIAL_SPEED 38400
#define PROGRAM_LENGTH 0x008f

#define FILE_NAME_LEN 30
#define ADDR_LEN 15
#define DATA_LEN 8

class EEPROM {
/*************************************************************************
*	Function Declarations
**************************************************************************/
	public:
		// Constructors
		EEPROM(char fileName[], const int clockTime,int chipSelectPin,
			   const byte *DATA, const byte *ADDR, const byte writePin,
			   const byte OEPin);
		EEPROM(char fileName[]);

		// Setting the address is used in reading and writing. Could have
		// made it private, but it comes in handy occasionally while debugging
		void setAddress(unsigned int twoByte);

		// Sets up Serial Monitor, opens up SD card file, and sets all of the
		// pins (except for the data bus which is handled in startRead and
		// startWrite)
		void autoSetup(void);

		// Used for debugging the internal values of the object
		void printInternalVals(void);

		// Writing functions
		void startWrite();
		void writeData(byte inputData, unsigned int address);
		void writeDataSD(int programLength);

		// Reading functions
		void startRead();
		void readData(long int startAddress, long int howManyAddresses);
		void hexdump(int numOfLines);
/************************************************************************/
	private:
/*************************************************************************
*	Variable declarations (create space in memory for these private
*		variables to be given values at construction)
**************************************************************************/
		File _myFile;
		char* _fileName;
		byte _clockTime;
		bool _SDCardReady;
/************************************************************************/

/*************************************************************************
*	Arduino Pin declarations
**************************************************************************/
		int _chipSelectPin;
		const byte *_DATA;
		const byte *_ADDR;
		// Pin A7 is the highest order Data bit (D7)
		const byte default_DATA[8] = {A7, A6, A5, A4, A3, A2, A1, A0};
		// Pin 47 is the (second) highest order Address bit (A14)
		const byte default_ADDR[15] = {33, 34, 35, 36, 37, 38, 39, 40,
									 41, 42, 43, 44, 45, 46, 47};
		byte _writePin;
		byte _OEPin;
/************************************************************************/
};
#endif
	