//
//  cpu.h
//  impossible-project4
//
//  Created by luis on 2/22/26.
//

#ifndef CPU_H
#define CPU_H

typedef struct flag {
    unsigned char carry:1;
    unsigned char zero:1;
    unsigned char interrupt_disable:1;
    unsigned char decimal:1;
    unsigned char b:1;
    unsigned char flag_6:1;
    unsigned char overflow:1;
    unsigned char negative:1;
} flag;

void decode(unsigned short opcode);

void testM(void);
#endif
