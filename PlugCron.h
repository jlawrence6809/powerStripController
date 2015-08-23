/* 
 * File:   PlugCron.h
 * Author: Buremy
 *
 * Created on July 21, 2015, 12:33 AM
 */

#ifndef PLUGCRON_H
#define	PLUGCRON_H

#ifdef	__cplusplus
extern "C" {
#endif

void setupPlugCron(void);
void plugCronServer(char*);
void tmrPlugCron(u8);
u8 setPlugCron(u8, u8, u16);


#ifdef	__cplusplus
}
#endif

#endif	/* PLUGCRON_H */

