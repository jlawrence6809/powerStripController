#include <xc.h>
#include <stdio.h>
#include <stdint.h>        /* For u8 definition */
#include <stdbool.h>       /* For true/false definition */
//#include <conio.h>
#include "shared.h"
#include "I2C.h"

void i2cWait();
void setupI2CMaster();
u8 putI2CMasterStr(u8, u8, char*);
u8 putI2CMaster(u8, u8);
u8 getI2CMaster(u8);

void setupI2CMaster(){
    I2C_SCL_AN = 0;
    I2C_SDA_AN = 0;
    I2C_SCL_TRIS = 1;
    I2C_SDA_TRIS = 1;

    MSSP_MODE = MSSP_I2C_MASTER_MODE;
    MSSP_EN = 1;
    MSSP_RATE = 255; //Fclock = 62.5 kHz
}

u8 putI2CMasterStr(u8 adx, u8 count, char* data){
    u8 readNWrite = 0;
//    while(SSP1CON2bits.SEN); //wait until start is done
//    SSP1BUF = 1;
//    while(BF); //buffer full
//    SSP1CON2bits.RSEN = 1; //repeated start

    //Master Tx
//    1. The user generates a Start condition by setting
//    the SEN bit of the SSPxCON2 register.
    I2C_START = 1;
//    2. SSPxIF is set by hardware on completion of the
//    Start.
    i2cWait();
//    3. SSPxIF is cleared by software.
//    4. The MSSPx module will wait the required start
//    time before any other operation takes place.
//    5. The user loads the SSPxBUF with the slave
//    address to transmit.
    I2C_BUF = (adx << 1) | readNWrite; //adx
//    6. Address is shifted out the SDAx pin until all eight
//    bits are transmitted. Transmission begins as
//    soon as SSPxBUF is written to.
//    7. The MSSPx module shifts in the ACK bit from
//    the slave device and writes its value into the
//    ACKSTAT bit of the SSPxCON2 register.
//    8. The MSSPx module generates an interrupt at
//    the end of the ninth clock cycle by setting the
//    SSPxIF bit.
    i2cWait();
    u8 fail = I2C_ACK;
    for(u8 i = 0; i < count; i++){
    //    9. The user loads the SSPxBUF with eight bits of
    //    data.
        SSP1BUF = data[i];
    //    10. Data is shifted out the SDAx pin until all eight
    //    bits are transmitted.
    //    11. The MSSPx module shifts in the ACK bit from
    //    the slave device and writes its value into the
    //    ACKSTAT bit of the SSPxCON2 register.
    //    12. Steps 8-11 are repeated for all transmitted data
    //    bytes.
        i2cWait();
        fail |= I2C_ACK;
    }
//    13. The user generates a Stop or Restart condition
//    by setting the PEN or RSEN bits of the
//    SSPxCON2 register. Interrupt is generated once
//    the Stop/Restart condition is complete
    I2C_STOP = 1;
    i2cWait();
    return fail;
}

u8 putI2CMaster(u8 adx, u8 data){
    return putI2CMasterStr(adx, 1, &data);
}

u8 getI2CMaster(u8 adx){
    u8 readNWrite = 1;
    //1. The user generates a Start condition by setting
    //the SEN bit of the SSPxCON2 register.
    I2C_START = 1;
    //2. SSPxIF is set by hardware on completion of the
    //Start.
    i2cWait();
    //3. SSPxIF is cleared by software.
    //4. User writes SSPxBUF with the slave address to
    //transmit and the R/W bit set.
    I2C_BUF = (adx << 1) | readNWrite;
    //5. Address is shifted out the SDAx pin until all eight
    //bits are transmitted. Transmission begins as
    //soon as SSPxBUF is written to.
    //6. The MSSPx module shifts in the ACK bit from
    //the slave device and writes its value into the
    //ACKSTAT bit of the SSPxCON2 register.
    //7. The MSSPx module generates an interrupt at
    //the end of the ninth clock cycle by setting the
    //SSPxIF bit.
    i2cWait();
    u8 fail = I2C_ACK;
    //8. User sets the RCEN bit of the SSPxCON2 register
    //and the Master clocks in a byte from the slave.
    I2C_RX = 1;
    //9. After the 8th falling edge of SCLx, SSPxIF and
    //BF are set.
    i2cWait();
    //10. Master clears SSPxIF and reads the received
    //byte from SSPxUF, clears BF.
    u8 data = I2C_BUF;
    I2C_FULL_BUF = 0;
    //11. Master sets ACK value sent to slave in ACKDT
    //bit of the SSPxCON2 register and initiates the
    //ACK by setting the ACKEN bit.
//    SSP1CON2bits.ACKDT = 0; //ack
//    SSP1CON2bits.ACKEN = 1;
    //12. Masters ACK is clocked out to the slave and
    //SSPxIF is set.
//    i2cWait();
    //13. User clears SSPxIF.
    //14. Steps 8-13 are repeated for each received byte
    //from the slave.
    //15. Master sends a not ACK or Stop to end
    //communication.
    I2C_OUT_ACK = 1; //nack
    I2C_START_ACK = 1;
    i2cWait();
    I2C_STOP = 1;
    i2cWait();
    return data;
}

void i2cWait(){
    u8 cnt = 255;
    while(!I2C_INT && cnt > 0){
        cnt--;
        __delay_us(1);
    }
    if(cnt == 0){
        putch('!');
    }else if(cnt == 255){
        putch('$');
    }
    I2C_INT = 0;
}

