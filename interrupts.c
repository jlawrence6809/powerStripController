#include <xc.h>         /* XC8 General Include File */
#include <stdint.h>         /* For u8 definition */
#include <stdbool.h>        /* For true/false definition */

#include "shared.h"
#include "SoftwareTimer.h"

char rxStr[ByteBufferSize]; //64
u8 rxStrCnt = 0;

void interrupt high_priority isrh(void){
    //tx eusart
    if(EUSART1_Tx_Interrupt_Flag){
        if(txBuffer1.Count > 0){
            EUSART1_Tx_Register = ByteBuffer_Read(&txBuffer1);
        }else{
            EUSART1_Tx_Interrupt_Enable = 0; //disable interrupt
        }
    }
    //rx eusart
    if(EUSART1_Rx_Interrupt_Flag){
        char in = EUSART1_Rx_Register;
        ByteBuffer_Write(&txBuffer1, in);
        EUSART1_Rx_Interrupt_Enable = 1; //enable interrupt
        rxStr[rxStrCnt] = in;
        rxStrCnt++;
        EUSART1_Rx_Interrupt_Flag = 0;
    }
}

void interrupt low_priority isrl(void) {
    if(SwTimer_Interrupt_Flag){ //timer 2
        swTimerCycle();
        SwTimer_Interrupt_Flag = 0;
    }
}
