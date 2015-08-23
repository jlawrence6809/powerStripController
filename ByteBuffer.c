#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

//#include "system.h"
#include "ByteBuffer.h"
#include "shared.h"

uint8_t ByteBuffer_Read(ByteBuffer*);
uint8_t ByteBuffer_Write(ByteBuffer*, uint8_t);
//uint8_t ByteBuffer_Peek(ByteBuffer*);
uint8_t ByteBuffer_Last(ByteBuffer*);
void ByteBuffer_Construct(ByteBuffer*);//, void*, void*);
void ByteBuffer_Write_Str(ByteBuffer*, char*);
volatile u8 inUse = 0;

//Implemented in a circular buffer. Does not overwrite if full.

void ByteBuffer_Construct(ByteBuffer* bb){
    bb->ReadIdx = ByteBufferSize;
    bb->WriteIdx = ByteBufferSize;
}

//uint8_t ByteBuffer_Peek(ByteBuffer* bb){
//    return bb->Data[bb->ReadIdx];
//}

uint8_t ByteBuffer_Last(ByteBuffer* bb){
    return bb->Data[bb->WriteIdx];
}

uint8_t ByteBuffer_Read(ByteBuffer* bb){
    if(inUse){
        return 0;
    }
    inUse = 1;
    if(bb->Count == 0){
        inUse = 0;
        return 0;
    }
    bb->Count--;
    bb->ReadIdx = (bb->ReadIdx+1)&ByteBufferSizeMask;
    uint8_t out = bb->Data[bb->ReadIdx];
    inUse = 0;
    return out;
}

void ByteBuffer_Write_Str(ByteBuffer* bb, char* str){
    while(*str != '\0'){
        ByteBuffer_Write(bb,(uint8_t) *str);
        str++;
    }
}

uint8_t ByteBuffer_Write(ByteBuffer* bb, uint8_t val){
    if(inUse){
        return 0;
    }
    inUse = 1;
    if(bb->Count == ByteBufferSize){
        inUse = 0;
        return 0;
    }
    bb->Count++;
    bb->WriteIdx = (bb->WriteIdx+1)&ByteBufferSizeMask;
    bb->Data[bb->WriteIdx] = val;
    inUse = 0;
    return 1;
}
