/* 
 * File:   ByteBuffer.h
 * Author: jeremy
 *
 * Created on November 28, 2014, 6:15 PM
 */

#ifndef BYTEBUFFER_H
#define	BYTEBUFFER_H

#ifdef	__cplusplus
extern "C" {
#endif

    #define ByteBufferSize 64
    #define ByteBufferSizeMask 0x3F

    typedef struct ByteBufferS{
        uint8_t Count;
        uint8_t ReadIdx;
        uint8_t WriteIdx;
        uint8_t OverUnderflowFlag;
        char Data[ByteBufferSize];
    } ByteBuffer;

    ByteBuffer txBuffer1;
//    ByteBuffer rxBuffer1;
//    ByteBuffer txBuffer2;
//    ByteBuffer rxBuffer2;
//    ByteBuffer irBuffer;

    extern void ByteBuffer_Construct(ByteBuffer*);//, void*, void*);
    extern uint8_t ByteBuffer_Read(ByteBuffer*);
//    extern uint8_t ByteBuffer_Peek(ByteBuffer*);
    extern uint8_t ByteBuffer_Last(ByteBuffer*);
    extern void ByteBuffer_Write_Str(ByteBuffer*, char*);
    extern uint8_t ByteBuffer_Write(ByteBuffer*, uint8_t);


#ifdef	__cplusplus
}
#endif

#endif	/* BYTEBUFFER_H */

