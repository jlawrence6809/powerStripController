#include <xc.h>
//#include <stdio.h>
#include <stdint.h>        /* For u8 definition */
#include <stdbool.h>       /* For true/false definition */
//#include <conio.h>
#include "shared.h"
#include "AM2302.h"

#define getSensBit(data) ((data >= 9 && data <= 23)?0:(data >= 36 && data <=47)?1:3)

void runAM2302(AM2302*);
void setupAM2302(void);

void setupAM2303(){
    AM2302_InNOut = 1; //input, float high, idle mode
    AM2302_Lat = 0;
}

void runAM2302(AM2302* results){
    AM2302_InNOut = 0; //output low
     __delay_ms(10);
     AM2302_InNOut = 1; //float high`
     __delay_us(1);

     u8 waitCount = 1;
     while(AM2302_Read == 1 && waitCount){ //wait for sens pulls down (20-40us)
         waitCount++;
         __delay_us(1);
     }
     waitCount = 1;
     while(AM2302_Read == 0 && waitCount){ //wait for sens lets up (80us)
         waitCount++;
         __delay_us(1);
     }
     waitCount = 1;
     while(AM2302_Read == 1 && waitCount){ //wait for start of tx (80us)
         waitCount++;
         __delay_us(1);
     }
     u8 data[40];
     for(u8 i = 0; i < 40; i++){
        waitCount = 1;
        while(AM2302_Read == 0 && waitCount){ //50us start of bit
            waitCount++;
            __delay_us(1);
        }
        waitCount = 1;
        while(AM2302_Read == 1 && waitCount){
            waitCount++;
            __delay_us(1);
        }
        data[i] = waitCount;
     }

     //convert data to bits
     u8 bits[40];
     u8 badBitFlag = 0;
     for(u8 i = 0; i < 40; i++){
         bits[i] = getSensBit(data[i]);
         if(bits[i] == 3){
             badBitFlag++;
         }
     }

     //relative humidity
     float rhf = 0;
     for(u8 i = 0; i < 16; i++){
         rhf = (rhf * 2) + bits[i];
     }
     rhf /= 10;

     //temperature
     float cel = 0;
     for(u8 i = 16; i < 32; i++){
         cel = (cel * 2) + bits[i];
     }
     cel /= 10;
     float fah = cel*1.8+32;

     //checksum
     u8 chk = 0;
     for(u8 i = 32; i < 40; i++){
         chk = (chk << 1) + bits[i];
     }

     //checksum = four bytes added together modulo a byte
     u8 chkag = 0;
     for(u8 i = 0; i < 32; i){
         u8 acum = 0;
         for(u8 j = 0; j < 8; j++){
             acum = (acum << 1) + bits[i];
             i++;
         }
         chkag += acum;
     }

     results->BB = badBitFlag;
     results->RH = rhf;
     results->Tc = cel;
     results->Tf = fah;
     results->chkE = chk;
     results->chkA = chkag;
}
