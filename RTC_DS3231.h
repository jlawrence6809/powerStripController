/* 
 * File:   RTC_DS3231.h
 * Author: Buremy
 *
 * Created on July 20, 2015, 11:33 PM
 */

#ifndef RTC_DS3231_H
#define	RTC_DS3231_H

#ifdef	__cplusplus
extern "C" {
#endif

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
void setConv();
u8 readDS3231(u8);
u8 writeDS3231(u8, u8);


#ifdef	__cplusplus
}
#endif

#endif	/* RTC_DS3231_H */

