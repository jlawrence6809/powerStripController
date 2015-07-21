#ifndef SOFTWARETIMER_H
#define	SOFTWARETIMER_H
 void swTimerSetup(void);
 void swTimerRun(void);
 void swTimerCycle(void);
 u8 swTimerRegister(uint32_t, void*);
 void swTimerUnregister(u8);

 void swTimerTrain(char *);

#endif	/* SOFTWARETIMER_H */

