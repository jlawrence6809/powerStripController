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
void swTimerUnregister(u8);

uint32_t secToCyc(uint32_t);

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
    printf("cyc: ");
    printHexU32(cycles);
    putch('\n');
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
    uint32_t out = (251*sec) - 3;
    return out;
}

void swTimerUnregister(u8 timer){
    if(timer < swTimersLength){
        swTimers[timer] = 0;
    }
}
