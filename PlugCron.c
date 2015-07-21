#include <xc.h>         /* XC8 General Include File */
#include <stdint.h>         /* For u8 definition */
#include <stdbool.h>        /* For true/false definition */
#include <stdio.h>
#include <conio.h>
#include "shared.h"
#include "PlugCron.h"
#include "PlugController.h"
#include "RTC_DS3231.h"
#include "SoftwareTimer.h"
#include "serverHelpers.h"

#define Update_Invl_Sec 55
#define cronLen 4

void setupPlugCron(void);
void plugCron(char*);
void runPlugCron(u8);
void printPlugCron(void);

uint16_t times[cronLen];
u8 masks[cronLen];
u8 timeFlag; //if chronLen goes over 8 will have to redo the logic
u16 lastMinOfDay;

void setupPlugCron(){
    u16 minOfDay = getDS3231MinOfDay();
    lastMinOfDay = minOfDay;

    u8 cnt = 0;
    for(u8 i = EEPROM_SWT_MASK_START; i < EEPROM_SWT_MASK_STOP; i++){
        u8 mask = eeprom_read(i);
        masks[cnt] = mask;
        cnt++;
    }

    cnt = 0;
    for(u8 i = EEPROM_SWT_START; i < EEPROM_SWT_STOP; i++){
        u8 lower = eeprom_read(i);
        i++;
        u8 upper = eeprom_read(i);
        u16 time = ((u16) lower) + (((u16) upper) << 8);
        times[cnt] = time;
        cnt++;
    }

    //set all timers that are in the passed to 'done'
    for(u8 i = 0; i < cronLen; i++){
        if(times[i] < minOfDay){
            masks[i] |= (1 << i);
        }
    }
    swTimerRegister(Update_Invl_Sec, &runPlugCron);
}

void plugCron(char * args){
    //if has hour and duration then apply mask and reverse after duration
    //if has hour and no duration then apply mask
    ParsedU32 p;
    nextHexU32(args, &p);
    if(!p.passed){
        return;
    }
    args = p.newArgs;
    u8 idx = (u8) p.result;

    if(idx >= cronLen){
        return;
    }

    nextHexU32(args, &p);
    if(!p.passed){
        return;
    }
    args = p.newArgs;
    u8 mask = (u8) p.result;

    nextHexU32(args, &p);
    if(!p.passed){
        return;
    }
    args = p.newArgs;
    u16 time = (u8) p.result;

    if(time >= 1440 /*24*60=1440*/){
        return;
    }

    times[idx] = time;
    masks[idx] = mask;

    eeprom_write(EEPROM_SWT_MASK_START + idx, mask);
    eeprom_write(EEPROM_SWT_START + (idx << 1), (u8) time);
    eeprom_write(EEPROM_SWT_START + (idx << 1) + 1, (u8) (time >> 8));

    printPlugCron();
}

void runPlugCron(u8 tmrId){
    u16 minOfDay = getDS3231MinOfDay();
    if(lastMinOfDay > minOfDay){
        //day transition
        timeFlag = 0;
    }

    for(u8 i = 0; i < cronLen; i++){
        u8 alreadyDone = (timeFlag >> i) & 1;
        if(!alreadyDone && times[i] < minOfDay){
            u8 mask = masks[i];
            for(u8 i = 0; i < 8; i++){
                u8 toggle = (mask >> i) & 1;
                if(toggle){
                    setPlug(i, PlugToggle);
                }
            }
            timeFlag |= (1 << i);
        }
    }
    lastMinOfDay = minOfDay;
    swTimerRegister(Update_Invl_Sec, &runPlugCron);
}

void printPlugCron(){
    printf("cron: %d:0x%x@0x%x", 0, times[0], masks[0]);
    for(u8 i = 1; i < cronLen; i++){
        printf(", %d:0x%x@0x%x", i, times[i], masks[i]);
    }
    putch('\n');
}

