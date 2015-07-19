/* 
 * File:   SoftwareToMetal.h
 * Author: Buremy
 *
 * Created on June 14, 2015, 10:11 PM
 */

#ifndef SOFTWARETOMETAL_H
#define	SOFTWARETOMETAL_H

#ifdef	__cplusplus
extern "C" {
#endif

#define EUSART1_Tx_Ansel                    ANSELCbits.ANSC6
#define EUSART1_Rx_Ansel                    ANSELCbits.ANSC7
#define EUSART1_Tx_Tris                     TRISCbits.RC6
#define EUSART1_Rx_Tris                     TRISCbits.RC7
#define EUSART1_Tx_Port                     PORTCbits.RC6
#define EUSART1_Rx_Port                     PORTCbits.RC7
#define EUSART1_Tx_Interrupt_Flag           PIR1bits.TX1IF
#define EUSART1_Tx_Register                 TXREG1
#define EUSART1_Tx_Interrupt_Enable         PIE1bits.TX1IE
#define EUSART1_Rx_Register                 RCREG1
#define EUSART1_Rx_Interrupt_Flag           PIR1bits.RC1IF
#define EUSART1_Rx_Interrupt_Enable         PIE1bits.TX1IE
#define EUSART1_Rx_Overrun_Error            RCSTA1bits.OERR
#define EUSART1_Rx_Overrun_Clear            RCSTA1bits.CREN
#define EUSART1_Rx_Frame_Error              RCSTAbits.FERR
#define EUSART1_Baud_High_Byte_En           TXSTA1bits.BRGH
#define EUSART1_16bit_Baud_En               BAUDCON1bits.BRG16
#define EUSART1_Baud_High_Byte              SPBRGH1bits.SP1BRGH
#define EUSART1_Baud_Low_Byte               SPBRG1bits.SP1BRG
#define EUSART1_Synchronous_En              TXSTA1bits.SYNC
#define EUSART1_Tx_En                       TXSTA1bits.TXEN
#define EUSART1_En_And_Set_Ports            RCSTA1bits.SPEN
#define EUSART1_Tx_Int_En                   PIE1bits.TX1IE
#define EUSART1_Rx_Int_En                   PIE1bits.RC1IE
#define EUSART1_Tx_Int_Priority             IPR1bits.TX1IP
#define EUSART1_Rx_Int_Priority             IPR1bits.RC1IP


#define AM2302_InNOut TRISAbits.RA7
#define AM2302_Read PORTAbits.RA7
#define AM2302_Lat LATAbits.LA7

#define Plug_Port PORTB
#define Plug_Ansel ANSELB
#define Plug_Wpu WPUB
#define Plug_Lat LATB
#define Plug_Tris TRISB

#define EEPROM_Resets 9
#define EEPROM_Plug 7


#define SwTimer_Enable PIE1bits.TMR2IE
#define SwTimer_Interrupt_Priority IPR1bits.TMR2IP
#define SwTimer_Prescale T2CONbits.T2CKPS
#define SwTimer_Postcale T2CONbits.T2OUTPS
#define SwTimer_Interrupt_Count PR2
#define SwTimer_On T2CONbits.TMR2ON
#define SwTimer_Interrupt_Flag PIR1bits.TMR2IF

#define I2C_INT SSP1IF
#define I2C_SCL_AN ANSELCbits.ANSC3
#define I2C_SDA_AN ANSELCbits.ANSC4
#define I2C_SCL_TRIS TRISCbits.RC3
#define I2C_SDA_TRIS TRISCbits.RC4
#define I2C_START SSP1CON2bits.SEN
#define I2C_BUF SSP1BUF
#define I2C_ACK SSP1CON2bits.ACKSTAT
#define I2C_STOP SSP1CON2bits.PEN
#define I2C_RX SSP1CON2bits.RCEN
#define I2C_FULL_BUF SSP1STATbits.BF
#define I2C_OUT_ACK SSP1CON2bits.ACKDT
#define I2C_START_ACK SSP1CON2bits.ACKEN
#define MSSP_MODE SSP1CON1bits.SSPM
#define MSSP_I2C_MASTER_MODE 0b1000
#define MSSP_EN SSP1CON1bits.SSPEN
#define MSSP_RATE SSP1ADD

#ifdef	__cplusplus
}
#endif

#endif	/* SOFTWARETOMETAL_H */

