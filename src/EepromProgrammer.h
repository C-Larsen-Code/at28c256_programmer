/*************************************************************************
* This library was built to write data to a 28C256 256k Parallel EEPROM
* chip to be used as ROM for a W65C02 processor.
*************************************************************************/


#ifndef EEPROM_PROGRAMMER_H
#define EEPROM_PROGRAMMER_H

#include <Arduino.h>

// An enum to track reading/writing mode
enum CommMode { read, write, none };

class EepromProgrammer {
    public:
        // Constructors
        EepromProgrammer(const byte clockTime, const byte *dataPins,
            const byte *addrPins, const byte writePin, const byte OEPin);
        EepromProgrammer();

        void writeData(Stream &inpText,
            unsigned int startAddr, unsigned int finalAddr);

        void hexdump(unsigned int startAddr, unsigned int numOfLines);

    private:
        // Setting the address is used in reading and writing. Could have
        // made it private, but it comes in handy occasionally while debugging
        void setAddress(unsigned int twoByte) const;
        void startWrite(void);
        void writeByte(byte inputData, unsigned int address) const;
        void startRead(void);
        // Setup all of the pins (except data pins; their modes depend on
        // if we are reading or writing). This is a constructor helper
        // function.
        void setup(void);

        // Variable declarations (create space in memory for these private
        unsigned int _clockTime;
        // To keep track of what the current communication mode is (read/write)
        CommMode _currCommMode;
        // Serial print functions that check if the Serial port was initialized
        // or not.
        void (*serialPrint)(const char str[]);
        void (*serialPrintLn)(const char str[]);

        // For EEPROM
        const byte *_dataPins;
        const byte *_addrPins;
        byte _writePin;
        byte _OEPin;
};

#endif
