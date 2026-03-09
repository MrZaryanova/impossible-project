//
//  ppu.h
//  impossible-project4
//
//  Created by luis on 3/6/26.
//
#ifndef PPU_H
#define PPU_H

typedef struct internal {
    unsigned short v:15;
    unsigned short t:15;
    unsigned char x:3;
    unsigned char w:1;
} internal;

void draw(void);

void ppuStats(void);

void ppu_loop(unsigned short ppu_addr);

#endif
