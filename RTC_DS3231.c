#include <xc.h>
#include <stdio.h>
#include <stdint.h>        /* For u8 definition */
#include <stdbool.h>       /* For true/false definition */

#include "shared.h"
#include "I2C.h"
#include "PlugController.h"
#include "serverHelpers.h"
#include "RTC_DS3231.h"

#define DS3232_I2C_ADX 0b1101000

#define MINUTES_PER_DAY 1440

#define SEC_ADX 0
#define MIN_ADX 1
#define HR_ADX 2
#define DAY_ADX 3
#define DATE_ADX 4
#define MONTH_ADX 5
#define YEAR_ADX 6
#define CONTROL0_ADX 0xE
#define CONTROL1_ADX 0xF
#define TEMP_MSB_ADX 0x11

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
u8 isDS3231Busy();
u8 getConv();
u8 setMinOfDay(u16);
void setConv();
u8 readDS3231(u8);
u8 writeDS3231(u8, u8);
void printAll();

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
        u8 c = p.result;
        if(c == 'm'){
            ParsedU32 min;
            nextHexU32(args, &min);
            if(!min.passed){
                return;
            }
            u16 min16 = (u16) min.result;
            setMinOfDay(min16);
        }
        printAll();
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
        res = writeDS3231(adx, val);
    }else{
        res = readDS3231(adx);
    }
    printf("0x%x: 0x%x\n", adx, res);
}

u16 getDS3231MinOfDay(){
    u8 minReg = readDS3231(1);
    u8 minutes = 0b00001111 & minReg;
    minutes += (minReg >> 4)*10;

    u8 hrReg = readDS3231(2);
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

void printAll(){
    u8 sec = getSec();
    u8 min = getMin();
    u8 hour = getHour();
    u8 day = getDay();
    u8 date = getDate();
    u8 month = getMonth();
    u8 year = getYear();
    u8 ctrl0 = getControl0();
    u8 ctrl1 = getControl1();
    u8 cel = getCelcius();
    printf("%d:%d:%d %d/%d/%d\n", hour, min, sec, month, date, year);
    printf("ctrl0: ");
    printBinU8(ctrl0);
    printf("\nctrl1: ");
    printBinU8(ctrl1);
    printf("\ncel: %dc\n", cel);
}

u8 getSec(){
    u8 secReg = readDS3231(SEC_ADX);
    u8 seconds = 0b00001111 & secReg;
    seconds += (secReg >> 4)*10;
    return seconds;
}

u8 getMin(){
    u8 minReg = readDS3231(MIN_ADX);
    u8 minutes = 0b00001111 & minReg;
    minutes += (minReg >> 4)*10;
    return minutes;
}

u8 getHour(){
    u8 hrReg = readDS3231(HR_ADX);
    u8 hours = 0b00001111 & hrReg;
    hours += (1 & (hrReg >> 4))*10;
    u8 using24hrClk = !((hrReg>>6) & 1);
    if(using24hrClk){
        hours += (1 & (hrReg >> 5))*20;
    }else{
        //convert to 24hr time
        putch('z');
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
    u8 days = readDS3231(DAY_ADX);
    return days;
}

u8 getDate(){
    u8 dateReg = readDS3231(DATE_ADX);
    u8 date = 0b00001111 & dateReg;
    date += (dateReg >> 4)*10;
    return date;
}

u8 getMonth(){
    u8 monthReg = readDS3231(MONTH_ADX);
    u8 month = 0b00001111 & monthReg;
    month += (0b00001111 & (monthReg >> 4)) * 10;
    u8 century = monthReg >> 7;
    return month;
}

u8 getYear(){
    u8 yearReg = readDS3231(YEAR_ADX);
    u8 year = 0b00001111 & yearReg;
    year += (yearReg >> 4)*10;
    return year;
}

u8 getControl0(){
    u8 control0 = readDS3231(CONTROL0_ADX);
    return control0;
}

u8 getControl1(){
    u8 control1 = readDS3231(CONTROL1_ADX);
    return control1;
}

u8 getCelcius(){
    u8 celcius = readDS3231(TEMP_MSB_ADX);
    return celcius;
}

u8 isDS3231Busy(){
   u8 busy = (getControl1() >> 2) & 1;
   return busy;
}

u8 setMinOfDay(u16 minOfDay){
    if(minOfDay >= MINUTES_PER_DAY){
        return 1;
    }
    u8 min = minOfDay%60;
    u8 minU = min/10;
    u8 minL = min%10;
    min = (minU << 4) + minL;
    writeDS3231(MIN_ADX, min);

    u8 hr = minOfDay/60;
    u8 hrU = hr/10;
    u8 hrL = hr%10;
    hr = (hrU << 4) + hrL;
    writeDS3231(HR_ADX, hr);

    return 0;
}

u8 getConv(){
    u8 control0 = getControl0();
    return (control0 >> 5) & 1;
}

void setConv(){
    u8 cur = getControl0();
    cur |= 0b00100000;
    writeDS3231(7, cur);
    __delay_ms(5);//datasheet says wait at least 2ms
}

u8 readDS3231(u8 adx){
    putI2CMaster(DS3232_I2C_ADX, adx);
    u8 out = getI2CMaster(DS3232_I2C_ADX);
    return out;
}

u8 writeDS3231(u8 adx, u8 val){
    char adxVal[2];
    adxVal[0] = adx;
    adxVal[1] = val;
    putI2CMasterStr(DS3232_I2C_ADX, 2, adxVal);
    u8 out = readDS3231(adx);
    return out;
}