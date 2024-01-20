//========================================================================
//    Command Line Interface (for choosing file to write to EEPROM)
//========================================================================

#ifndef AT28C256_SERIAL_H
#define AT28C256_SERIAL_H

#include "Arduino.h"
#include <SD.h>
#include "AT28C256_Programmer.h"

// Set up a state machine for the simple command-line interface
enum CmdLnState { SDCardError, FileSelect, ModeSelect, Read, Write };

class EepromCmdLine {
    private:
        // Variables
        CmdLnState currState;
        File file;
        EEPROM eeprom;
        const byte sdCSPin;
        // Methods
        void fileSelectCase();
        void modeSelectCase();
        void readCase();
        void writeCase();
        // Helper Functions
        String getInput(const char *request);
        int toHexNum(const String input);
    public:
        EepromCmdLine(const byte sdCSPin);
        void runCurrCase();
};
#endif
