/*************************************************************************
* This library was built to write data to a 28C256 256k Parallel EEPROM
* chip to be used as ROM for a W65C02 processor.
*************************************************************************/


#ifndef EEPROM_COMM_H
#define EEPROM_COMM_H

#include <Arduino.h>
#include <SD.h>
#include <stdio.h>
#include <string.h>

#define SERIAL_SPEED 115200
#define PROGRAM_LENGTH 0x008f

#define FILE_NAME_LEN 30
#define addrPins_LEN 15
#define dataPins_LEN 8

class EEPROM {
/*************************************************************************
*    Function Declarations
**************************************************************************/
    public:
        // Constructors
        EEPROM(File *file, const byte clockTime, const byte *dataPins,
            const byte *addrPins, const byte writePin, const byte OEPin);
        EEPROM(File *file);

        // Sets up Serial Monitor, opens up SD card file, and sets all of the
        // pins (except for the data bus which is handled in startRead and
        // startWrite)
        void autoSetup(void);

        // Used for debugging the internal values of the object
        void printInternalVals(void);

        void writeDataSD(int programLength);

        void readData(long int startAddress, long int howManyAddresses);
        void hexdump(int numOfLines);
/************************************************************************/
    private:
        // Setting the address is used in reading and writing. Could have
        // made it private, but it comes in handy occasionally while debugging
        void setAddress(unsigned int twoByte);
        void startWrite();
        void writeData(byte inputData, unsigned int address);
        void startRead();

/*************************************************************************
*    Variable declarations (create space in memory for these private
*        variables to be given values at construction)
**************************************************************************/
        File *_file;
        byte _clockTime;
        // SoftwareSerial _serial;
/************************************************************************/

/*************************************************************************
*    Arduino Pin declarations
**************************************************************************/
        // For EEPROM
        const byte *_dataPins;
        const byte *_addrPins;
        byte _writePin;
        byte _OEPin;
/************************************************************************/
};
#endif
    
