#include <xc.h>
#include <stdio.h>
#include <stdint.h>        /* For u8 definition */
#include <stdbool.h>       /* For true/false definition */
//#include <conio.h>
#include "shared.h"
#include "serverHelpers.h"
#include "ByteBuffer.h"
#include "SoftwareTimer.h"
#include "PlugController.h"
#include "I2C.h"
#include "RTC_DS3231.h"
#include "PlugCron.h"

ByteBuffer txBuffer1;
//ByteBuffer rxBuffer1;

void serverSetup(void);
void serverRun(void);
void findAndRun(char*);
void async_Eusart_1_Setup(u8, u8);
u8 hexCharToNum(char);

void clearHasHadReset(char*);
void reportErrors(char*);
void resetCommand(char*);
void printSystemCounter(char*);
void printNumberOfRestarts(char*);

void serverSetup(){
    async_Eusart_1_Setup(0x06, 0x82); //9600 baud
    ByteBuffer_Construct(&txBuffer1);
//    ByteBuffer_Construct(&rxBuffer1);
    __delay_ms(1);
    printf("start...\n#");
}

void serverRun(){
    if(EUSART1_Rx_Overrun_Error){//rx overrun, clear buffer
        EUSART1_Rx_Overrun_Clear = 0;
        EUSART1_Rx_Overrun_Clear = 1;
    }
    if(EUSART1_Rx_Frame_Error && !EUSART1_Rx_Port){
        printf("BL_reset...\n");
        delay_10ms(15);
        Reset();
    }
    if(rxStrCnt > 0){
        if(rxStr[rxStrCnt-1] == '\n'){
            rxStr[rxStrCnt] = 0;
            findAndRun(rxStr);
        }else if(rxStrCnt >= ByteBufferSize){
            printf("Rx Overflow!\n");
            rxStrCnt = 0;
        }
    }
}
#define cmdLen 9
void findAndRun(char* cmd){
    const char * const cmds[cmdLen] =  {"setplug",      "printplug", "hhr",             "reset",       "syscnt",            "numrst",               "errs",        "rwrtc",       "pcron"};
    void * const cmdsF[cmdLen] = {&setPlugServer, &printPlugs, &clearHasHadReset, &resetCommand, &printSystemCounter, &printNumberOfRestarts, &reportErrors, &rwRTC_DS3231, &plugCron};

//    char blah = 'h';

//    char blah[] = {1,2};

    bool passed;
    for(u8 i = 0; i < cmdLen; i++){
        passed = true;
        char* tmpCmd = cmd;
        const char* cur = cmds[i];
        while(*cur != 0){
            if(*cur != *tmpCmd){
                passed = false;
                break;
            }
            cur++;
            tmpCmd++;
        }
        if(*tmpCmd != '\n' && *tmpCmd != ' '){
            passed = false;
        }
        if(passed){
            while(*tmpCmd == ' '){
                tmpCmd++;
            }
            void (*runThis)() = cmdsF[i];
            runThis(tmpCmd);
            break;
        }
    }
    if(!passed){
        printf("Not Found. Available:\n");
        for(u8 i = 0; i < cmdLen; i++){
            printf("%s\n", cmds[i]);
        }
    }
    putch('#');
    rxStrCnt = 0;
}

void clearHasHadReset(char * args){
    printf("b4 clr: %x\n", hasHadReset);
    hasHadReset = 0;
}

void reportErrors(char* args){
    Nop();
    u8 flags0 = RCONbits.BOR;
    flags0 = (flags0 << 1) + RCONbits.POR;
    flags0 = (flags0 << 1) + RCONbits.TO;
    flags0 = (flags0 << 1) + RCONbits.PD;
    flags0 = (flags0 << 1) + STKPTRbits.STKFUL;
    flags0 = (flags0 << 1) + STKPTRbits.STKOVF;
    flags0 = (flags0 << 1) + STKPTRbits.STKUNF;
    printf("sysF:0x%X\n", flags0);
}

void resetCommand(char* args){
    printf("reset...\n");
    delay_10ms(15);
    Reset();
}

void printSystemCounter(char* args){
   printf("C:0x");
   printHexU32(SystemCounter);
   putch('\n');
}

void printNumberOfRestarts(char* args){
    printf("#Rst:0x%X\n", numberOfRestarts);
}

void putch(char in){
    while(!ByteBuffer_Write(&txBuffer1, in)); //blocking
    EUSART1_Tx_Interrupt_Enable = 1; //enable interrupt
}

void async_Eusart_1_Setup(u8 baudH, u8 baudL){
    EUSART1_Baud_High_Byte_En = 1;
    EUSART1_16bit_Baud_En     = 1;

    EUSART1_Baud_High_Byte = baudH;//0x06;
    EUSART1_Baud_Low_Byte   = baudL;//0x82;

    EUSART1_Tx_Ansel   = 0;//set to not analog
    EUSART1_Rx_Ansel   = 0;
    EUSART1_Tx_Tris    = 1;//set to input, the eusart circuitry handles setting these
    EUSART1_Rx_Tris    = 1;
    EUSART1_Synchronous_En    = 0; //async mode
    EUSART1_Tx_En    = 1; //tx enable
    EUSART1_Rx_Overrun_Clear    = 1; //rx enable

    EUSART1_En_And_Set_Ports     = 1; //serial port enable

    EUSART1_Tx_Int_En = 1; //interrupt enable
    EUSART1_Rx_Int_En = 1;
    EUSART1_Tx_Int_Priority = 1; //high
    EUSART1_Rx_Int_Priority = 1;
}
