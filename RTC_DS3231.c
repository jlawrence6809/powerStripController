#include <xc.h>
#include <stdio.h>
#include <stdint.h>        /* For u8 definition */
#include <stdbool.h>       /* For true/false definition */

#include "shared.h"
#include "I2C.h"
#include "PlugController.h"
#include "serverHelpers.h"
#include "RTC_DS3231.h"

void rwRTC_DS3231(char*);
u16 getDS3231MinOfDay();
u8 getSec();
u8 getMin();
u8 getHour();
u8 getDay();
u8 getDate();
u8 getMonth();
u8 getYear();
u8 getControl0();
u8 getControl1();
u8 getCelcius();
u8 readDs3231(u8);
u8 writeDs3231(u8, u8);

void rwRTC_DS3231(char * args){
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

u16 getDS3231MinOfDay(){
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

    u16 minTotal = (u16) minutes;
    minTotal += ((u16) hours)*60;
    return minTotal;
}

u8 getSec(){
    u8 secReg = readDs3231(0);
    u8 seconds = 0b00001111 & secReg;
    seconds += (secReg >> 4)*10;
    return seconds;
}

u8 getMin(){
    u8 minReg = readDs3231(1);
    u8 minutes = 0b00001111 & minReg;
    minutes += (minReg >> 4)*10;
    return minutes;
}

u8 getHour(){
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
    return hours;
}

u8 getDay(){
    u8 days = readDs3231(3);
    return days;
}

u8 getDate(){
    u8 dateReg = readDs3231(4);
    u8 date = 0b00001111 & dateReg;
    date += (dateReg >> 4)*10;
    return date;
}

u8 getMonth(){
    u8 monthReg = readDs3231(5);
    u8 month = 0b00001111 & monthReg;
    month += (0b00001111 & (monthReg >> 4)) * 10;
    u8 century = monthReg >> 7;
    return month;
}

u8 getYear(){
    u8 yearReg = readDs3231(6);
    u8 year = 0b00001111 & yearReg;
    year += (yearReg >> 4)*10;
    return year;
}

u8 getControl0(){
    u8 control0 = readDs3231(7);
    return control0;
}

u8 getControl1(){
    u8 control1 = readDs3231(0x0F);
    return control1;
}

u8 getCelcius(){
    u8 celcius = readDs3231(0x11);
    return celcius;
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