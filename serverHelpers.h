/* 
 * File:   serverHelpers.h
 * Author: Buremy
 *
 * Created on June 21, 2015, 12:25 AM
 */

#ifndef SERVERHELPERS_H
#define	SERVERHELPERS_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef
    struct nextHexU32s{
        uint32_t result;
        u8 passed;
        char* newArgs;
    }
ParsedU32;

typedef
    struct nextHexU8s{
        u8 result;
        u8 passed;
        char* newArgs;
    }
ParsedU8;

void nextHexU32(char*, ParsedU32*);
void nextHexU8(char*, ParsedU8*);
void printHexU32(uint32_t);
u8 hexCharToNum(char);


#ifdef	__cplusplus
}
#endif

#endif	/* SERVERHELPERS_H */

