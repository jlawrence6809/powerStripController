/* 
 * File:   I2C.h
 * Author: Buremy
 *
 * Created on July 19, 2015, 12:23 PM
 */

#ifndef I2C_H
#define	I2C_H

#ifdef	__cplusplus
extern "C" {
#endif

void i2cWait();
void setupI2CMaster();
u8 putI2CMasterStr(u8, u8, char*);
u8 putI2CMaster(u8, u8);
u8 getI2CMaster(u8);


#ifdef	__cplusplus
}
#endif

#endif	/* I2C_H */

