#include <xc.h>         /* XC8 General Include File */
#include <stdint.h>         /* For u8 definition */
#include <stdbool.h>        /* For true/false definition */
#include <stdio.h>
#include <conio.h>
#include "shared.h"
#include "SoftwareTimer.h"
#include "serverHelpers.h"

#define swTimersLength 5

void swTimerSetup(void);
void swTimerRun(void);
void swTimerCycle(void);
u8 swTimerRegister(uint32_t, void*);
u8 swTimerRegister_Cycles(uint32_t, void*);
void swTimerUnregister(u8);

uint32_t secToCyc(uint32_t);

uint32_t swtTimerTrain(char *);
uint32_t trainCase(u8);

uint32_t swTimers[swTimersLength];
void (*swTimerCallbacks[swTimersLength]);

void swTimerSetup(){
    SwTimer_Enable              = 1; //timer 2 interrupt enable
    SwTimer_Interrupt_Priority  = 0; //low priority
    SwTimer_Prescale            = 0b11; //max prescale
    SwTimer_Postcale            = 0b1111; //max postscaler
    SwTimer_Interrupt_Count     = 0xFF; //what count to interrupt on
    SwTimer_On                  = 1; //timer 2 on
}

void swTimerRun(){
    for(u8 i = 0; i < swTimersLength; i++){
        if(swTimers[i] == 1){
            NOP();
            swTimers[i] = 0;
            void (*func)() = swTimerCallbacks[i];
            (*func)(i);
        }
    }
}

void swTimerCycle(){
    for(u8 i = 0; i < swTimersLength; i++){
        if(swTimers[i] > 1){
            swTimers[i]--;
        }
    }
}

u8 swTimerRegister(uint32_t sec, void* callback){
//    (251L*(sec.longValue()) - 3L);
    uint32_t cycles = secToCyc(sec);
//    printf("cyc: ");
//    printHexU32(cycles);
//    putch('\n');
    return swTimerRegister_Cycles(cycles, callback);
}

u8 swTimerRegister_Cycles(uint32_t cycles, void* callback){
    if(cycles == 0){
        cycles = 1;
    }
    for(u8 i = 0; i < swTimersLength; i++){
        if(swTimers[i] == 0){
            swTimers[i] = cycles;
            swTimerCallbacks[i] = callback;
            return i;
        }
    }
    return 0xFF;
}

uint32_t secToCyc(uint32_t sec){
    uint32_t out = (sec*251) - 3;
    return out;
}

void swTimerUnregister(u8 timer){
    if(timer < swTimersLength){
        swTimers[timer] = 0;
    }
}

//void swTimerTrain(char *){
//    uint32_t sec30;
//    uint32_t min;
//    uint32_t min5;
//    u8 id = swTimerRegister_Cycles(0xFFFFFFFF, 0);
//
//    printf("sec30");
//    sec30 = trainCase(id);
//    printf("min");
//    min = trainCase(id);
//    printf("min5");
//    min5 = trainCase(id);
//
//    printf("sec30: ");
//    printHexU32(sec30);
//    putch('\n');
//
//    printf("min: ");
//    printHexU32(min);
//    putch('\n');
//
//    printf("min5: ");
//    printHexU32(min5);
//    putch('\n');
//}
//
//uint32_t trainCase(u8 id){
//    uint32_t cnt;
//    u8 cur = rxStrCnt;
//    while(rxStrCnt == cur);
//    cnt = swTimers[id];
//    printf("start");
//    u8 cur = rxStrCnt;
//    while(rxStrCnt == cur);
//    cnt = swTimers[id] - cnt;
//    printf("done");
//    return cnt;
//}


//
////uint32_t swtCnt = 0;
//u8 swtState = 0;
//uint32_t swt0ASec = 0;
//uint32_t swt0BSec = 0;
//uint32_t swt1ASec = 0;
//uint32_t swt1BSec = 0;
//uint8_t swt0Id = 255;
//uint8_t swt1Id = 255;
//u8 toggleMask0 = 0;
//u8 toggleMask1 = 0;
//
//void swt0Callback(u8 tmrId){
//    u8 isA = swtState & 1;
//    uint32_t swtSec;
//    if(isA){
//        swtSec = swt0BSec;
//    }else{
//        swtSec = swt0ASec;
//    }
//    putch(swtState);
//    swtState ^= 1;
//    swt0Id = swTimerRegister(swtSec, &swt0Callback);
//}
//
//void swt1Callback(u8 tmrId){
//    u8 isA = swtState & 0b10;
//    uint32_t swtSec;
//    if(isA){
//        swtSec = swt1BSec;
//    }else{
//        swtSec = swt1ASec;
//    }
//    putch(swtState);
//    swtState ^= 0b10;
//    swt1Id = swTimerRegister(swtSec, &swt1Callback);
//}
//
//void plugSwt(char* args){
//    ParsedU8 p8;
//    nextHexU8(args, &p8);
//    if(!p8.passed){
//        return;
//    }
//    args = p8.newArgs;
//    u8 timerIdx = p8.result;
//
//    nextHexU8(args, &p8);
//    if(!p8.passed){
//        return;
//    }
//    args = p8.newArgs;
//    u8 toggleMask = p8.result;
//
//    ParsedU32 p;
//    nextHexU32(args, &p);
//    if(!p.passed){
////        printf("invalid tmr\n");
//        return;
//    }
//    args = p.newArgs;
//    uint32_t swtASec = p.result;
//    if(swtASec < 10){
//        swtASec = 10;
//    }
//    nextHexU32(args, &p);
//    if(!p.passed){
//        return;
//    }
//    args = p.newArgs;
//    uint32_t swtBSec = p.result;
//    if(swtBSec < 10){
//        swtBSec = 10;
//    }
//    u8 swtId;
//    if(timerIdx == 0){
//        swt0ASec = swtASec;
//        swt0BSec = swtBSec;
//        toggleMask0 = toggleMask;
//        swTimerUnregister(swt0Id);
//        swt0Id = swTimerRegister(swtASec, &swt0Callback);
//        swtId = swt0Id;
//    }else if(timerIdx == 1){
//        swt1ASec = swtASec;
//        swt1BSec = swtBSec;
//        toggleMask1 = toggleMask;
//        swTimerUnregister(swt1Id);
//        swt1Id = swTimerRegister(swtASec, &swt1Callback);
//        swtId = swt1Id;
//    }
//
//    printf("tmr %d: 0x", swtId);
//    printHexU32(swtASec);
//    printf(", 0x");
//    printHexU32(swtBSec);
//    putch('\n');
//}
