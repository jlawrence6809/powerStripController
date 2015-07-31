#include <xc.h>
#include <stdio.h>
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include "shared.h"
#include "serverHelpers.h"

void nextHexU32(char*, ParsedU32*);
void nextHexU8(char*, ParsedU8*);
void printHexU32(uint32_t);
uint8_t hexCharToNum(char);
char* advanceToNext(char* );

char* advanceToNext(char* args){
    if(*args == '\n'){
        return args;
    }
    while(*args != ' ' && *args != '\n'){
        args++;
    }
    while(*args == ' ' && *args != '\n'){
        args++;
    }
    return args;
}

void nextHexU32(char * args, ParsedU32* res){
    uint32_t val = 0;
    u8 passed = 0;
    while(*args == ' '){
        args++;
    }
    for(u8 i = 0; i < 8; i++){
        if(*args == '\n' || *args == ' '){
            //note that if this happens in the first loop, passed is never set to 1
            break;
        }
        passed = 1;
        u8 cur = hexCharToNum(*args);
        if(cur == 0xFF){
            passed = 0;
            val = (u32) *args;
            break;
        }
        val = val << 4;
        val += cur;
        args++;
    }
    res->result = val;
    res->passed = passed;
    res->newArgs = advanceToNext(args);
}

void nextHexU8(char * args, ParsedU8* res){
    u8 val = 0;
    u8 passed = 0;
    while(*args == ' '){
        args++;
    }
    for(u8 i = 0; i < 2; i++){
        if(*args == '\n' || *args == ' '){
            //note that if this happens in the first loop, passed is never set to 1
            break;
        }
        passed = 1;
        u8 cur = hexCharToNum(*args);
        if(cur == 0xFF){
            passed = 0;
            val = *args;
            break;
        }
        val = val << 4;
        val += cur;
        args++;
    }
    res->result = val;
    res->passed = passed;
    res->newArgs = advanceToNext(args);
}

void printHexU32(uint32_t v){
   char numToCharArray[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
   putch('0');
   putch('x');
   for(u8 i = 0; i < 8; i++){
        v = (v << 4) | (v >> 28);
        u8 tmpSC8 = ((u8) v) & 0x0F;
        putch(numToCharArray[tmpSC8]);
    }
}

void printBinU8(u8 v){
    putch('0');
    putch('b');
    for(u8 i = 0; i < 8; i++){
        v = (v << 1) | (v >> 7);
        u8 t = v & 1;
        t += '0';
        putch(t);
    }
}

u8 hexCharToNum(char v){
    if(v >= '0' && v <= '9'){
        return v - '0';
    }else if(v >= 'a' && v <= 'f'){
        return v - 'a' + 10;
    }else if(v >= 'A' && v <='F'){
        return v - 'A' + 10;
    }
    return 0xFF;
}

