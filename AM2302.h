#ifndef AM2302_H
#define	AM2302_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef
    struct AM2302s{
        float RH;
        float Tc;
        float Tf;
        uint8_t chkE;
        uint8_t chkA;
        uint8_t BB;
    }
AM2302;

void runAM2302(AM2302*);
void setupAM2302(void);

#ifdef	__cplusplus
}
#endif

#endif	/* AM2302_H */

