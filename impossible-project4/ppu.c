//
//  ppu.c
//  impossible-project4
//
//  Created by luis on 3/6/26.
//
#include "ppu.h"
#include "cpu.h"
#include <SDL3/SDL_log.h>

internal internals;

void draw(void) {
    ppu_setMemory(0x2002, ppu_getMemory(0x2002)| 0x80); // set v blank
}

void ppuStats(void) {
    SDL_Log("ppu 2000 %d\n", ppu_getMemory(0x2000));
    SDL_Log("ppu 2001 %d\n", ppu_getMemory(0x2001));
    SDL_Log("ppu 2002 %d\n", ppu_getMemory(0x2002));
}

void ppu_loop(unsigned short ppu_addr) {
    

    draw();
}
