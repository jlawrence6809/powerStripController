#include <xc.h>
#include <stdio.h>
#include <stdint.h>        /* For u8 definition */
#include <stdbool.h>       /* For true/false definition */
//#include <conio.h>
#include "shared.h"
#include "serverHelpers.h"
#include "ByteBuffer.h"
#include "AM2302.h"
#include "SoftwareTimer.h"
#include "PlugController.h"
#include "I2C.h"

ByteBuffer txBuffer1;
//ByteBuffer rxBuffer1;

void serverSetup(void);
void serverRun(void);
void findAndRun(char*);
void async_Eusart_1_Setup(u8, u8);
u8 hexCharToNum(char);

void reportErrors(char*);
void resetCommand(char*);
void printSystemCounter(char*);
void printNumberOfRestarts(char*);
void reportAM2302(char*);
void softwareTimerTest(char*);
void runDs3231RTC(char *);

void allDs3231RTC(void);
u8 readDs3231(u8);
u8 writeDs3231(u8, u8);

void tmpCallback(u8);

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

void tmpCallback(u8 tmrId){
    printf("\ntmr done: %x\n", tmrId);
}

void softwareTimerTest(char* args){
    ParsedU32 p;
    nextHexU32(args, &p);
    if(!p.passed){
        printf("invalid tmr\n");
        return;
    }
    uint32_t tmr = p.result;
    u8 tmrId = swTimerRegister(tmr, &tmpCallback);
    printf("set: %x to ", tmrId);
    printHexU32(tmr);
    putch('\n');
}

void findAndRun(char* cmd){
    u8 cmdLen = 10;
    char * cmds[] =  {"setplug",         "printplug", "envmeas",     "reset",       "syscnt",            "numrst",               "errs",        "swt",              "plugivl", "rtc"};
    void * cmdsF[] = {&setPlugServer,   &printPlugs, &reportAM2302, &resetCommand, &printSystemCounter, &printNumberOfRestarts, &reportErrors, &softwareTimerTest, &plugTimer, &runDs3231RTC};

    bool passed;
    for(u8 i = 0; i < cmdLen; i++){
        passed = true;
        char* tmpCmd = cmd;
        char* cur = cmds[i];
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

void reportAM2302(char* args){
    printf("m:{RH: ");
    AM2302 data;
    runAM2302(&data);

     printf("%f%%, Tc: %f, Tf: %f}\n", data.RH, data.Tc, data.Tf);

     if(data.BB){
         printf("BB:%d\n", data.BB);
     }

     if(data.chkA != data.chkE){
         printf("CHK FAIL! E: %d, G: %d\n", data.chkE, data.chkA);
     }
}

void runDs3231RTC(char * args){
    //max 100khz i2c (400khz fast mode)
    //adx = 7b1101000
    u8 adx;
    u8 readNWrite = 0;
    u8 val = 0;

    ParsedU8 p;
    nextHexU8(args, &p);
    args = p.newArgs;
    if(!p.passed){
        //print time, tmp status
        allDs3231RTC();
        return;
    }
    adx = p.result;

    nextHexU8(args, &p);
    args = p.newArgs;
    val = p.result;
    if(!p.passed){
        readNWrite = 1;
    }
    u8 res;
    if(!readNWrite){
        printf("wrt: 0x%x to 0x%x\n", adx, val);
        res = writeDs3231(adx, val);
    }else{
        res = readDs3231(adx);
    }
    printf("0x%x: 0x%x\n", adx, res);
}

void allDs3231RTC(){
    u8 secReg = readDs3231(0);
    u8 seconds = 0b00001111 & secReg;
    seconds += (secReg >> 4)*10;

    u8 minReg = readDs3231(1);
    u8 minutes = 0b00001111 & minReg;
    minutes += (minReg >> 4)*10;

    u8 hrReg = readDs3231(2);
    u8 hours = 0b00001111 & hrReg;
    hours += (1 & (hrReg >> 4))*10;
    u8 using24hrClk = !(hrReg>>6);
    if(using24hrClk){
        hours += (1 & (hrReg >> 5))*20;
    }else{
        //convert to 24hr time
        if(hours == 12){
            hours = 0;
        }
        u8 pmNam = (1 & (hrReg >> 5));
        if(pmNam){
            hours += 12;
        }
    }

    u8 days = readDs3231(3);

    u8 dateReg = readDs3231(4);
    u8 date = 0b00001111 & dateReg;
    date += (dateReg >> 4)*10;

    u8 monthReg = readDs3231(5);
    u8 month = 0b00001111 & monthReg;
    month += (0b00001111 & (monthReg >> 4)) * 10;

    u8 century = monthReg >> 7;

    u8 yearReg = readDs3231(6);
    u8 year = 0b00001111 & yearReg;
    year += (yearReg >> 4)*10;

    u8 control0 = readDs3231(7);

    u8 control1 = readDs3231(0x0F);

    u8 celcius = readDs3231(0x11);

    printf("%d:%d:%d:%d(%d):%d:%d:%d\n", century, year, month, date, days, hours, minutes, seconds);
    printf("%dC, c0:0x%x, c1:0x%x\n", celcius, control0, control1);
}

u8 readDs3231(u8 adx){
    u8 i2cAdx = 0b1101000;
    putI2CMaster(i2cAdx, adx);
    u8 out = getI2CMaster(i2cAdx);
    return out;
}

u8 writeDs3231(u8 adx, u8 val){
    u8 i2cAdx = 0b1101000;
    char adxVal[2];
    adxVal[0] = adx;
    adxVal[1] = val;
    putI2CMasterStr(i2cAdx, 2, adxVal);
    u8 out = readDs3231(adx);
    return out;
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
