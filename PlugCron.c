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

//#define CRON_EEPROM

#define Update_Invl_Sec 25
#define cronLen 4
#define MINUTES_PER_DAY 1440

void setupPlugCron(void);
void plugCronServer(char*);
void tmrPlugCron(u8);
u8 setPlugCron(u8, u8, u16);
void printPlugCron(void);
u8 plugCron(char *);
u8 plugCronTempCheck(void);

u8 allOff = 0;

uint16_t times[cronLen];
u8 masks[cronLen];
u8 doneFlag; //if chronLen goes over 8 will have to redo the logic
u16 lastMinOfDay;

void setupPlugCron(){
    u16 minOfDay = getDS3231MinOfDay();
    lastMinOfDay = minOfDay;

    #ifdef CRON_EEPROM
    u8 cnt = 0;
    for(u8 i = EEPROM_SWT_MASK_START; i <= EEPROM_SWT_MASK_STOP; i++){
        u8 mask = eeprom_read(i);
        masks[cnt] = mask;
        cnt++;
    }

    cnt = 0;
    for(u8 i = EEPROM_SWT_START; i <= EEPROM_SWT_STOP; i++){
        u16 lower = (u16) eeprom_read(i);
        i++;
        u16 upper = (u16) eeprom_read(i);
        u16 time = lower + (upper << 8);
        times[cnt] = time;
        cnt++;
    }
    #endif
    //set all timers that are in the passed to 'done'
    for(u8 i = 0; i < cronLen; i++){
        if(times[i] < minOfDay){
            doneFlag |= (1 << i);
        }
    }

    //0:0x7@0x1a4, 1:0x2@0x1ab, 2:0x7@0x474, 3:0x2@0x47b
    setPlugCron(0, 7, 0x1a4 );
    setPlugCron(1, 2, 0x1ab );
    setPlugCron(2, 7, 0x474 );
    setPlugCron(3, 2, 0x47b );
    swTimerRegister(Update_Invl_Sec, &tmrPlugCron);
    #ifdef CRON_EEPROM
    allOff = eeprom_read(EEPROM_ALLOFF);
    #endif
}

void plugCronServer(char * args){
    u8 error = plugCron(args);
    if(error){
        printf("err:%d\n", error);
    }
    printPlugCron();
}

u8 plugCron(char * args){
    //if has hour and duration then apply mask and reverse after duration
    //if has hour and no duration then apply mask
    ParsedU32 p;
    nextHexU32(args, &p);
    if(!p.passed){
        if(((u8) p.result) == 'r'){
            putch('r');
            allOff = 0;
            #ifdef CRON_EEPROM
            eeprom_write(EEPROM_ALLOFF, 0);
            #endif
        }
        return 1;
    }
    args = p.newArgs;
    u8 idx = (u8) p.result;

    nextHexU32(args, &p);
    if(!p.passed){
        return 2;
    }
    args = p.newArgs;
    u8 mask = (u8) p.result;

    nextHexU32(args, &p);
    if(!p.passed){
        return 3;
    }
    args = p.newArgs;
    u16 time = (u16) p.result;

    return setPlugCron(idx, mask, time);
}

u8 setPlugCron(u8 idx, u8 mask, u16 time){
    if(idx >= cronLen){
        return 4;
    }
    if(time >= MINUTES_PER_DAY){
        return 5;
    }
    times[idx] = time;
    masks[idx] = mask;
    doneFlag &= 0xFF - (1 << idx);

    #ifdef CRON_EEPROM
    eeprom_write(EEPROM_SWT_MASK_START + idx, mask);
    eeprom_write(EEPROM_SWT_START + (idx << 1), (u8) time);
    eeprom_write(EEPROM_SWT_START + (idx << 1) + 1, (u8) (time >> 8));
    #endif
    return 0;
}

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
//u16 testMinOfDay = 0;
void tmrPlugCron(u8 tmrId){
    u16 minOfDay = getDS3231MinOfDay();
    u8 tempCheck = 1;//!plugCronTempCheck();
    if(tempCheck){
        //printf("\n0x%x m\n", minOfDay);
        if(minOfDay < lastMinOfDay){
            //day transition
            doneFlag = 0;
            printf("\nday\n");
        }

        for(u8 i = 0; i < cronLen; i++){
            if(!((doneFlag >> i) & 1) && times[i] == minOfDay){
                u8 mask = masks[i];
                printf("\npc 0x%x: 0x%x, 0x%x\n", times[i], mask, doneFlag);
                for(u8 i = 0; i < 8; i++){
                    u8 toggle = (mask >> i) & 1;
                    if(toggle){
                        setPlug(i, PlugToggle);
                    }
                }
                printPlugs(0);
                doneFlag |= (1 << i);
            }
        }
    }
    lastMinOfDay = minOfDay;
    swTimerRegister(Update_Invl_Sec, &tmrPlugCron);
}

#define tempsLen 8
#define HIGH_TEMP 50
#define LOW_TEMP 0
u8 temps[tempsLen];
u8 tempsIdx = 0;
u8 heatIdx = 0;
u8 heatStopIdx = tempsLen + 1;
u8 plugCronTempCheck(){
    u8 failed = 0;
    u8 cel = getCelForCron();
    u8 isHarmfulTemp = (cel > HIGH_TEMP || cel == LOW_TEMP)?1:0;
    u8 newAllOff = allOff;
    switch(newAllOff){
        case 0: //normal
            if(isHarmfulTemp){
                newAllOff = 1;
                heatIdx = tempsIdx;
                heatStopIdx = ((tempsLen >> 2) + tempsIdx - 1)%tempsLen;
                setPlug('a', PlugOff);
            }else{
                temps[tempsIdx] = cel;
            }
            break;
        case 1:
            if(isHarmfulTemp){
                newAllOff = 2;
            }else{
                newAllOff = 0; //back to normal
                break;
            }
        default:
            failed = 1;
            if(isHarmfulTemp && newAllOff < 255){
                newAllOff++;
            }

            printf("heat! 0x%x, idx:0x%x\n", newAllOff, heatIdx);
            for(u8 i = 0; i < tempsLen; i++){
                printf("0x%x, ", temps[i]);
            }
            putch('\n');
            break;
    }
    #ifdef CRON_EEPROM
    if(allOff != newAllOff){
        eeprom_write(EEPROM_ALLOFF, 1);
    }
    #endif
    if(tempsIdx != heatStopIdx){
        temps[tempsIdx] = cel;
        tempsIdx++;
        tempsIdx %= tempsLen;
    }

    return failed;
}

void printPlugCron(){
    printf("cron: %d:0x%x@0x%x", 0, masks[0], times[0]);
    for(u8 i = 1; i < cronLen; i++){
        printf(", %d:0x%x@0x%x", i, masks[i], times[i]);
    }
    putch('\n');
}

