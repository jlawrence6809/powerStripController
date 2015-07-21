/* 
 * File:   PlugController.h
 * Author: Buremy
 *
 * Created on June 14, 2015, 10:16 PM
 */

#ifndef PLUGCONTROLLER_H
#define	PLUGCONTROLLER_H

#ifdef	__cplusplus
extern "C" {
#endif

#define PlugOff 0
#define PlugOn 1
#define PlugToggle 2

void plugControllerSetup(void);
void setPlugServer(char*);
u8 setPlug(u8, u8);
void printPlugs(char*);

#ifdef	__cplusplus
}
#endif

#endif	/* PLUGCONTROLLER_H */

