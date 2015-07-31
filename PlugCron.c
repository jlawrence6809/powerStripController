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

#define Update_Invl_Sec 25
#define cronLen 4

void setupPlugCron(void);
void plugCron(char*);
void runPlugCron(u8);
void printPlugCron(void);
u8 plugCron_OnlyErrors(char *);

u8 allOff = 0;

uint16_t times[cronLen];
u8 masks[cronLen];
u8 timeFlag; //if chronLen goes over 8 will have to redo the logic
u16 lastMinOfDay;

void setupPlugCron(){
    u16 minOfDay = getDS3231MinOfDay();
    lastMinOfDay = minOfDay;

    u8 cnt = 0;
    for(u8 i = EEPROM_SWT_MASK_START; i <= EEPROM_SWT_MASK_STOP; i++){
        u8 mask = eeprom_read(i);
        masks[cnt] = mask;
        cnt++;
    }

    cnt = 0;
    for(u8 i = EEPROM_SWT_START; i <= EEPROM_SWT_STOP; i++){
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
            timeFlag |= (1 << i);
        }
    }
    swTimerRegister(Update_Invl_Sec, &runPlugCron);

    allOff = eeprom_read(EEPROM_ALLOFF);
}

void plugCron(char * args){
    u8 error = plugCron_OnlyErrors(args);
    if(error){
        printf("i msk m, %d\n", error);
    }
    printPlugCron();
}

u8 plugCron_OnlyErrors(char * args){
    //if has hour and duration then apply mask and reverse after duration
    //if has hour and no duration then apply mask
    ParsedU32 p;
    nextHexU32(args, &p);
    if(!p.passed){
        if(((u8) p.result) == 'r'){
            putch('r');
            allOff = 0;
            eeprom_write(EEPROM_ALLOFF, 0);
        }
        return 1;
    }
    args = p.newArgs;
    u8 idx = (u8) p.result;

    if(idx >= cronLen){
        return 2;
    }

    nextHexU32(args, &p);
    if(!p.passed){
        return 3;
    }
    args = p.newArgs;
    u8 mask = (u8) p.result;

    nextHexU32(args, &p);
    if(!p.passed){
        return 4;
    }
    args = p.newArgs;
    u16 time = (u16) p.result;

    if(time >= 1440 /*24*60=1440*/){
        return 5;
    }

    times[idx] = time;
    masks[idx] = mask;
    timeFlag &= 0xFF - (1 << idx);

    eeprom_write(EEPROM_SWT_MASK_START + idx, mask);
    eeprom_write(EEPROM_SWT_START + (idx << 1), (u8) time);
    eeprom_write(EEPROM_SWT_START + (idx << 1) + 1, (u8) (time >> 8));

    return 0;
}

#define tempsLen 8
u8 temps[tempsLen];
u8 tempsIdx = 0;
u8 heatIdx = 0;
u8 heatStopIdx = 0;

u8 waitCronBusy(){
    u8 to = 255;
    while(isDS3231Busy() && to){
        to--;
        __delay_ms(10);
    }
    if(!to){
        printf("\ntimeout");
        return 1;
    }
    return 0;
}

u8 getCelForCron(){
//    if(waitCronBusy()){
//        return 0;
//    }
//    setConv();
    if(waitCronBusy()){
        return 0;
    }
    u8 cel = getCelcius();
    return cel;
}

void runPlugCron(u8 tmrId){
    u16 minOfDay = getDS3231MinOfDay();
    u8 cel = getCelForCron();
//    if(cel > 50 || cel == 0){
//        cel = getCelForCron();
//    }
    printf("\n0x%x m, %dc\n", minOfDay, cel);

    if(cel > 50 ||  cel == 0){
        if(!allOff){
            heatIdx = tempsIdx;
            heatStopIdx = ((tempsLen >> 2) + tempsIdx - 1)%tempsLen;
            setPlug('a', PlugOff);
            eeprom_write(EEPROM_ALLOFF, 1);
        }
        if(allOff < 255){
            allOff++;
        }
    }else if(allOff == 1){
        //if on the second measurement we don't fail then reset it
        allOff = 0;
    }

    if(allOff >= 2){
        printf("heat! 0x%x, idx:0x%x\n", allOff, heatIdx);
        if(tempsIdx != heatStopIdx){
            temps[tempsIdx] = cel;
            tempsIdx++;
            tempsIdx %= tempsLen;
        }
        //else{
            for(u8 i = 0; i < tempsLen; i++){
                printf("0x%x, ", temps[i]);
            }
            putch('\n');
        //}

    }else{
        temps[tempsIdx] = cel;
        tempsIdx++;
        tempsIdx %= tempsLen;
        if(minOfDay < lastMinOfDay){
            //day transition
            timeFlag = 0;
        }

        for(u8 i = 0; i < cronLen; i++){
            if(!((timeFlag >> i) & 1) && times[i] == minOfDay){
                u8 mask = masks[i];
                printf("pc 0x%x: 0x%x", times[i], mask);
                for(u8 i = 0; i < 8; i++){
                    u8 toggle = (mask >> i) & 1;
                    if(toggle){
                        setPlug(i, PlugToggle);
                    }
                }
                timeFlag |= (1 << i);
            }
        }
    }
    lastMinOfDay = minOfDay;
    swTimerRegister(Update_Invl_Sec, &runPlugCron);
}

void printPlugCron(){
    printf("cron: %d:0x%x@0x%x", 0, masks[0], times[0]);
    for(u8 i = 1; i < cronLen; i++){
        printf(", %d:0x%x@0x%x", i, masks[i], times[i]);
    }
    putch('\n');
}

