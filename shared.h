#ifndef SHARED_H
#define SHARED_H

#ifdef  __cplusplus
extern "C" {
#endif
    typedef uint8_t u8;

    #define SYS_FREQ        64000000L
    #define _XTAL_FREQ      SYS_FREQ
    #define FCY             SYS_FREQ/4

    #include "SoftwareToMetal.h"
    #include "ByteBuffer.h"
    #define delay_10ms(val) u8 tmptmp = val; while(tmptmp--)__delay_ms(10)

    u8 numberOfRestarts;

    uint32_t SystemCounter;

    char rxStr[ByteBufferSize]; //64
    u8 rxStrCnt;
    
    void serverRun(void);
    void serverSetup(void);
    void putch(char);

#ifdef  __cplusplus
}
#endif

#endif  /* SHARED_H */
