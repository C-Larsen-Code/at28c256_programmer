/*************************************************************************
* This library was built to write data to a 28C256 256k Parallel EEPROM
* chip to be used as ROM for a W65C02 processor.
*************************************************************************/


#ifndef EEPROM_COMM_H
#define EEPROM_COMM_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
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
		EEPROM(char fileName[], const byte clockTime,int chipSelectPin,
			   const byte *DATA, const byte *ADDR, const byte writePin,
			   const byte OEPin);
		void setAddress(unsigned int twoByte);
		void startWrite();
		void startRead();
		void readData(int startAddress, int howManyAddresses);
		void writeData(byte inputData, unsigned int address);
		void writeDataSD(int programLength);
		void autoSetup(void);
/************************************************************************/
	private:
/*************************************************************************
*	Variable declarations (create space in memory for these private
*		variables to be given values at construction)
**************************************************************************/
		File _myFile;
		byte _clockTime;
		bool _SDCardReady;
/************************************************************************/

/*************************************************************************
*	Arduino Pin declarations
**************************************************************************/
		int _chipSelectPin;
		const byte *_DATA;
		const byte *_ADDR;
		byte _writePin;
		byte _OEPin;
/************************************************************************/
};
#endif
	