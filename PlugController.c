#include <xc.h>
#include <stdio.h>
#include <stdint.h>        /* For u8 definition */
#include <stdbool.h>       /* For true/false definition */

#include "shared.h"
#include "PlugController.h"
#include "serverHelpers.h"
#include "SoftwareTimer.h"

void plugControllerSetup(void);
void setPlugServer(char*);
u8 setPlug(u8, u8);
void printPlugs(char*);
void printSetPlugUsage(void);
void plugTimer(char*);
void plugTimerTO(u8);

void plugControllerSetup(){
    Plug_Ansel = 0x00;
    Plug_Wpu = 0x00;
    Plug_Lat = 0x00;
    Plug_Tris = 0x00;
    delay_10ms(50);
    Plug_Lat = eeprom_read(EEPROM_Plug);
}

void setPlugServer(char* args){
    ParsedU8 v8;
    nextHexU8(args, &v8);
    if(!v8.passed){
        printSetPlugUsage();
        return;
    }

    char plugToSet = v8.result;
    if(plugToSet == 10){
        plugToSet = 'a';
    }
    args = v8.newArgs;
    nextHexU8(args, &v8);

    if(!v8.passed){
        printSetPlugUsage();
        return;
    }
    char valToSet = v8.result;
    args = v8.newArgs;

    setPlug(plugToSet, valToSet);
    printPlugs(args);
}

u8 setPlug(u8 plug, u8 val){
    if(plug >= '0' && plug <= '7'){
        plug -= '0';
    }else if(plug == 'a'){
        plug = 8;
    }else if(plug > 7){
        return 0;
    }

    if(val >= '0'){
        val -= '0';
    }

    if(val != PlugOff && val != PlugOn && val != PlugToggle){
        return 0;
    }

    u8 masks[9] = {1,2,4,8,16,32,64,128,255};
    if(val == PlugOff){
        Plug_Port = Plug_Port & (~masks[plug]);
    }else if(val == PlugOn){
        Plug_Port = Plug_Port | masks[plug];
    }else if(val == PlugToggle){
        Plug_Port = Plug_Port ^ masks[plug];
    }
    eeprom_write(EEPROM_Plug,Plug_Port);
    return 1;
}

void printPlugs(char* args){
    printf("pv: 0x%x\n", Plug_Port);
}

void printSetPlugUsage(){
    printf( "# #\n");
}
