#include <xc.h>         /* XC8 General Include File */
#include <stdio.h>
#include <stdint.h>        /* For u8 definition */
//#include <stdbool.h>       /* For true/false definition */

#include "shared.h"
#include "ByteBuffer.h"
#include "SoftwareTimer.h"
#include "PlugController.h"
#include "serverHelpers.h"
#include "I2C.h"
#include "PlugCron.h"

void configureOscillator(void);
void initApp(void);
u8 runDS3231RTC(char *);

uint32_t SystemCounter = 0;
uint8_t numberOfRestarts = 0;
u8 hasHadReset = 1;

void main(void){
    configureOscillator();
    initApp();
    serverSetup();
    swTimerSetup();
    plugControllerSetup();
    setupI2CMaster();
    setupPlugCron();

    numberOfRestarts = eeprom_read(EEPROM_Resets) + 1;
    eeprom_write(EEPROM_Resets, numberOfRestarts);

    while(1){
        SystemCounter++;
        swTimerRun();
        serverRun();
    }
}

void configureOscillator(void){
    //sets clock to 64MHz
    OSCCONbits.SCS = 0b00;
    OSCCONbits.IRCF = 0b111; //16MHz
    OSCTUNEbits.PLLEN = 1; //pll on
    OSCTUNEbits.TUN = 0b011111; //maximum overclock B)
}

void initApp(void){
    INTCONbits.GIE  = 1; //enable global interrupts
    INTCONbits.PEIE = 1; //perhipheral interrupts enabled
    RCONbits.IPEN = 1; //allow interrupt priorities
    INTCONbits.GIEH = 1; //high
    INTCONbits.GIEL = 1; //low
}

