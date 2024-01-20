//========================================================================
//    Command Line Interface (for choosing file to write to EEPROM)
//========================================================================

#ifndef EEPROM_SERIAL_CTRL_H
#define EEPROM_SERIAL_CTRL_H

#include "Arduino.h"
#include <SD.h>
#include "EepromProgrammer.h"

// Set up a state machine for the simple command-line interface
enum CmdLnState { SDCardError, FileSelect, ModeSelect, Read, Write };

class EepromSerialCtrl {
    private:
        // Variables
        CmdLnState currState;
        File file;
        EepromProgrammer eeprom;
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
        EepromSerialCtrl(const byte sdCSPin);
        void runCurrCase();
};
#endif
