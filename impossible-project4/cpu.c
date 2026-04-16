#include "cpu.h"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_filesystem.h>
#define MEM_SIZE 0x0100000

unsigned char acc;
unsigned char x;
unsigned char y;
unsigned char stack_pointer;
flag flags;
unsigned char memory[MEM_SIZE]; // include bytes
unsigned short program_counter = 0x8000; // first instruction should be here
/* The data types above are defined with the assumption
 that a char is one byte, and that a short is two bytes
 
 For safety, we can switch these out with bit fielded data types in the future
 */

// file load in....
/*
 Check follwoing instrctions
 JSR,
 */



unsigned char get_flag_bits(void) {
    unsigned char val = 0x00;
    val = val + (flags.negative << 7) + (flags.overflow << 6) + 32 + (flags.b << 4) + (flags.decimal << 3) + (flags.interrupt_disable << 2) + (flags.zero << 1) + flags.carry;
    return val;
    
}




static void storeb(unsigned short addr, unsigned char v) {
    if (addr == 0x4016) {
        // io function
        return;
    }
    
    if(addr >= 0x4000 && addr <= 0x4017) {
        // apu code here
        
        return;
    }
    memory[addr] = v;
}

static unsigned char func_operand(unsigned short address) { // for io stuff
    // It gets the real operand by making register reads give side effects
    if (address >= 0x2000 && address <= 0x2007) memory[address]; // ppu get memory function with side effects.
    switch(address) {
        case 0x4015:
            // read side effects, then return address
            return memory[address];
            break;
        case 0x4016:
            return memory[address]; // io function
            break;
        case 0x4017:
            return memory[address]; // same io function with different parameters
            break;
        default:
            return memory[address];
    }
}

void postStats(void) {
 SDL_Log("Acc: %d\n", acc);
 SDL_Log("x: %d\n", x);
 SDL_Log("y: %d\n", y);
 SDL_Log("stack: %d\n", stack_pointer);
 SDL_Log("Program counter: %d\n", program_counter);
 SDL_Log("interrupt disable: %d\n", flags.interrupt_disable);
}
 

void decode (unsigned short opcode) {
    unsigned short instruction_length = 1;
    //  Naive approach - table driven approach can be implemented once we get this project going
    if((opcode & 0xF) == 8) { // lower nibble / last 4 bits
        
        switch (opcode) {
            case 0x08: // push processor status
                flags.b = 0x1;
                memory[0x0100 + stack_pointer] = get_flag_bits();
                stack_pointer--;
                // 3 cycles
                break;
            case 0x02: // Clear Carry
                flags.carry = 0;
                // two cycles
                break;
            case 0x28: // Pull Processor Status
                stack_pointer++;
                flags.negative = (memory[0x0100 + stack_pointer] >> 7) & 1;
                flags.overflow = (memory[0x0100 + stack_pointer] >> 6) & 1;
                flags.decimal = (memory[0x0100 + stack_pointer] >> 5) & 1;
                flags.interrupt_disable = (memory[0x0100 + stack_pointer] >> 4) & 1; // delayed
                flags.zero = (memory[0x0100 + stack_pointer] >> 1) & 1;
                flags.carry = (memory[0x0100 + stack_pointer] >> 0) & 1;
                // 4 cycles
                break;
            case 0x38: // Set carry
                flags.carry = 1;
                break;
            case 0x48: // Push Accumulator
                memory[0x0100 + stack_pointer] = acc;
                stack_pointer--;
                // 3 cycles
                break;
            case 0x58: // Clear Interrupt disable
                flags.interrupt_disable = 0; // delayed one instruction
                break;
            case 0x68: // Pull accumulator
                stack_pointer++;
                acc = memory[0x0100 + stack_pointer];
                flags.zero = acc == 0;
                flags.negative = (acc >> 7) & 1;
                // 4 cycles
                break;
            case 0x78: // Set interrupt disable
                flags.interrupt_disable = 1; // delayed one instruction
                break;
            case 0x88: // Decrement Y
                y--;
                flags.zero = y == 0;
                flags.negative = (y >> 7) & 1;
                break;
            case 0x98: //  transfer Y to accumulator
                acc = y;
                flags.zero = acc == 0;
                flags.negative = (acc >> 7) & 1;
                break;
            case 0xA8: // Transfer accumulator to y
                y = acc;
                flags.zero = y == 0;
                flags.negative = (y >> 7) & 1;
                break;
            case 0xB8: // Clear overflow
                flags.overflow = 0;
                break;
            case 0xC8:  // Increment Y
                y++;
                flags.zero = y == 0;
                flags.negative = (y >> 7) & 1;
                break;
            case 0xE8: // Increment X
                x++;
                flags.zero = x == 0;
                flags.negative = (x >> 7) & 1;
                break;
            default:
                // print out unused/illegal opcode when tracer is done
                break;
        }
    
    } else if((((opcode & 0xF0) >> 4) == 7) && ((opcode & 0xF) == 0xA)) { // shift is for first 4 bits / upper nibble
        switch(opcode) {
            case 0xEA:
                // No operation 2 cycles
                break;
            case 0x8A: // Transfer X to accumulator
                acc = x;
                flags.zero = acc == 0;
                flags.negative = (acc >> 7) & 1;
                break;
            case 0x9A: // Transfer X to Stack pointer
                stack_pointer = x;
                break;
            case 0xAA: // Transfer accumulator to X
                x = acc;
                flags.zero = x == 0;
                flags.negative = (x >> 7) & 1;
                break;
            case 0xBA:
                x = stack_pointer;
                flags.zero = x == 0;
                flags.negative = (x >> 7) & 1;
                break;
            case 0xCA:
                x--;
                flags.zero = x == 0;
                flags.negative = (x >> 7) & 1;
                break;
        }
                
    } else {
        unsigned char aaa = (opcode & 0xE0) >> 5; // all opcodes are in format 0xaaabbbcc
        unsigned char bbb = (opcode & 0x1C) >> 2; // determines addressing mode
        unsigned char cc = opcode & 0x03;
        // adressing variables
        unsigned short addr = 0;
        unsigned char operand = 0;
        unsigned char zp = memory[program_counter + 1];
        switch (cc) {
            case 1:
                switch(bbb) {
                    case 2: // immediate addressing
                        operand = memory[program_counter + 1];
                        instruction_length = 2;
                        break;
                    case 0: // indirect X;
                        ;
                        unsigned char ptr = (zp + x) & 0xFF;
                        addr = memory[ptr] | (memory[(ptr + 1) & 0xFF] << 8);
                        operand = func_operand(addr);
                        instruction_length = 2;
                        break;
                    case 1: // Zero page
                        addr = zp;
                        operand = func_operand(addr);
                        instruction_length = 2;
                        break;
                    case 3: // absolute;
                        addr = memory[program_counter + 1] | (memory[program_counter + 2] << 8);
                        instruction_length = 3;
                        operand = func_operand(addr);
                        break;
                    case 4: // indirect Y
                        addr = memory[zp] | (memory[(zp + 1) & 0xFF] << 8);
                        addr += y;
                        operand = func_operand(addr);
                        instruction_length = 2;
                        break;
                    case 5: // Zero page x
                        addr = (zp + x) & 0xFF;
                        operand = func_operand(addr);
                        instruction_length = 2;
                        break;
                    case 6: // absolute Y
                        addr = memory[program_counter + 1] |
(memory[program_counter + 2] << 8);
                        instruction_length = 3;
                        addr += y;
                        operand = func_operand(addr);
                        break;
                    case 7: //Absolute X
                        addr = memory[program_counter + 1] |
(memory[program_counter + 2] << 8);
                        instruction_length = 3;
                        addr += x;
                        operand = func_operand(addr);
                        break;
                }
                
                switch(aaa) {
                    case 0: // bitwise or
                        acc = acc | operand;
                        flags.zero = acc == 0;
                        flags.negative = (acc >> 7) & 1;
                        break;
                    case 1: // bitwise and
                        acc = acc & operand;
                        flags.zero = acc == 0;
                        flags.negative = (acc >> 7) & 1;
                        break;
                    case 2: // bitwise XOR
                        acc = acc ^ operand;
                        flags.zero = acc == 0;
                        flags.negative = (acc >> 7) & 1;
                        break;
                    case 3: // add with carry
                        ;
                        unsigned short sum =
                             (unsigned short)acc +
                             (unsigned short)operand +
                             (unsigned short)flags.carry;

                         unsigned char result = (unsigned char)sum;

                         // Carry flag (unsigned overflow)
                         flags.carry = (sum > 0xFF);

                         // Zero flag
                         flags.zero = (result == 0);

                         // Negative flag
                         flags.negative = (result >> 7) & 1;

                         // Overflow flag (signed overflow)
                         flags.overflow =
                             (~(acc ^ operand) & (acc ^ result) & 0x80) != 0;

                         acc = result;
                        break;
                    case 4: // Store accumulator
                        storeb(addr, acc);
                        break;
                    case 5: // Load into accumulator
                        acc = operand;
                        flags.zero = acc == 0;
                        flags.negative = (acc >> 7) & 1;
                        break;
                    case 6: // Compare A
                        flags.zero = acc == operand;
                        flags.carry = acc >= operand;
                        flags.negative = acc < operand;
                        break;
                    case 7: // Subtract with carry
                        ;
                        unsigned short value = (unsigned short)operand ^ 0x00FF;

                            unsigned short sum1 =
                                (unsigned short)acc +
                                value +
                                (unsigned short)flags.carry;

                            unsigned char result2 = (unsigned char)sum1;

                            // Carry flag (no borrow)
                            flags.carry = (sum1 > 0xFF);

                            // Zero flag
                            flags.zero = (result2 == 0);

                            // Negative flag
                            flags.negative = (result2 >> 7) & 1;

                            // Overflow flag
                            flags.overflow =
                        ((acc ^ result2) & (acc ^ operand) & 0x80) != 0;

                            acc = result2;
                        break;
                }
                break;
            case 2:
                // aaa:
                
                switch(bbb) {
                    case 0: // immediate
                        operand = memory[program_counter + 1];
                        instruction_length = 2;
                        break;
                    case 1: // zero page
                        zp = memory[program_counter + 1];
                        addr = zp;
                        operand = func_operand(addr);
                        instruction_length = 2;
                        break;
                    case 2: // accumulator
                        operand = acc;
                        break;
                    case 3: // absolute
                        addr = memory[program_counter + 1] |
                                   (memory[program_counter + 2] << 8);
                            operand = func_operand(addr);
                        instruction_length = 3;
                        break;
                    case 5: // zero page x
                        zp = memory[program_counter + 1];
                        if (aaa == 5) // LDX
                            addr = (zp + y) & 0xFF;
                        else
                            addr = (zp + x) & 0xFF;
                        operand = func_operand(addr);
                        instruction_length = 2;
                        break;
                    case 7: // absolute x
                        addr = memory[program_counter + 1] |
(memory[program_counter + 2] << 8);
                        instruction_length = 3;
                        
                        if (aaa == 5) // LDX
                            addr += y;
                        else
                            addr += x;
                        
                        operand = func_operand(addr);
                        break;
                        
                }
                // variables for the switch
                unsigned char result;
                unsigned char old_carry = flags.carry;
                switch(aaa) {
                    case 0: // Arithmetric left shift
                        if (bbb == 2) {  // Accumulator
                            flags.carry = (acc >> 7) & 1;
                            result = acc << 1;
                            acc = result;
                        } else {
                            flags.carry = (operand >> 7) & 1;
                            result = operand << 1;
                            storeb(addr, result);
                        }
                        
                        flags.zero = (result == 0);
                        flags.negative = (result & 0x80) != 0;
                        break;
                    case 1: // Rotate left
                        if (bbb == 2) {
                            flags.carry = (acc >> 7) & 1;
                            result = (acc << 1) | old_carry;
                            acc = result;
                        } else {
                            flags.carry = (operand >> 7) & 1;
                            result = (operand << 1) | old_carry;
                            storeb(addr, result);
                        }
                        flags.zero = (result == 0);
                        flags.negative = (result & 0x80) != 0;
                        break;
                    case 2: // logical shift right
                        if (bbb == 2) {
                            flags.carry = acc & 1;
                            result = acc >> 1;
                            acc = result;
                        } else {
                            flags.carry = operand & 1;
                            result = operand >> 1;
                            storeb(addr, result);
                        }
                        flags.zero = (result == 0);
                        flags.negative = 0;  // bit 7 always 0 after LSR
                        break;
                    case 3: // Rotate Right
                        if (bbb == 2) {
                            flags.carry = acc & 1;
                            result = (acc >> 1) | (old_carry << 7);
                            acc = result;
                        } else {
                            flags.carry = operand & 1;
                            result = (operand >> 1) | (old_carry << 7);
                            storeb(addr, result);
                        }
                        
                        flags.zero = (result == 0);
                        flags.negative = (result & 0x80) != 0;
                        break;
                    case 4: // Store X
                        storeb(addr, x);
                        break;
                    case 5: // load X
                        x = operand;
                        flags.zero = x == 0;
                        flags.negative = (x >> 7) & 1;
                        break;
                    case 6: // decrement memory
                        storeb(addr, operand - 1);
                        flags.zero = ((operand - 1) == 0);
                        flags.negative = ((operand - 1) & 0x80) != 0;
                        break;
                    case 7: // increment memory
                        storeb(addr, operand + 1);
                        flags.zero = ((operand + 1) == 0);
                        flags.negative = ((operand + 1) & 0x80) != 0;
                        break;
                }
                break;
            default:
                if (bbb == 4) {
                    signed char offset = (signed char)memory[program_counter + 1];
                    int take_branch = 0;
                    instruction_length = 2;
                    switch (aaa) {
                        case 0: // BPL (Branch if Positive)
                            take_branch = (flags.negative == 0);
                            break;
                            
                        case 1: // BMI (Branch if Negative)
                            take_branch = (flags.negative == 1);
                            break;
                            
                        case 2: // BVC (Branch if Overflow Clear)
                            take_branch = (flags.overflow == 0);
                            break;
                            
                        case 3: // BVS (Branch if Overflow Set)
                            take_branch = (flags.overflow == 1);
                            break;
                            
                        case 4: // BCC (Branch if Carry Clear)
                            take_branch = (flags.carry == 0);
                            break;
                            
                        case 5: // BCS (Branch if Carry Set)
                            take_branch = (flags.carry == 1);
                            break;
                            
                        case 6: // BNE (Branch if Not Equal)
                            take_branch = (flags.zero == 0);
                            break;
                            
                        case 7: // BEQ (Branch if Equal)
                            take_branch = (flags.zero == 1);
                            break;
                    }

                    if (take_branch) {
                        program_counter += 2;     // move past opcode + operand
                        program_counter += offset;
                        instruction_length = 0;
                    }
                } else if (bbb == 0 && !(aaa & 0x4)) {
                    // variables for switch
                    switch(opcode) {
                    
                        case 0x00: // Break
                            ;
                            unsigned short return_addrr = program_counter + 2;
                                // Push high byte
                                memory[0x0100 + stack_pointer--] = (return_addrr >> 8) & 0xFF;

                                // Push low byte
                                memory[0x0100 + stack_pointer--] = return_addrr & 0xFF;

                                // Set B flag before pushing status
                                flags.b = 1;
                                memory[0x0100 + stack_pointer--] = get_flag_bits();

                                // Set interrupt disable flag
                                flags.interrupt_disable = 1;

                                // Load IRQ/BRK vector
                                program_counter = memory[0xFFFE] |
                                                  (memory[0xFFFF] << 8);

                                return;
                            break;
                            // Interrupt / Stack (I/S) logic
                        case 0x20: // Jump to subroutine
                            ;
                            unsigned short target =
                                    memory[program_counter + 1] |
                                    (memory[program_counter + 2] << 8);

                                unsigned short return_addr = program_counter + 2;

                                memory[0x0100 + stack_pointer--] = (return_addr >> 8) & 0xFF;
                                memory[0x0100 + stack_pointer--] = return_addr & 0xFF;

                                program_counter = target; // only time we modify PC during decoding
                            instruction_length = 0;
                            break;
                        case 0x40: // Return from interrupt
                            stack_pointer++;
                            unsigned char status = memory[0x0100 + stack_pointer];
                            
                            flags.negative          = (status >> 7) & 1;
                            flags.overflow          = (status >> 6) & 1;
                            // bit 5 ignored (always set internally)
                            flags.b                 = (status >> 4) & 1;
                            flags.decimal           = (status >> 3) & 1;
                            flags.interrupt_disable = (status >> 2) & 1;
                            flags.zero              = (status >> 1) & 1;
                            flags.carry             =  status       & 1;
                            
                            stack_pointer++;
                                unsigned short low = memory[0x0100 + stack_pointer];
                                stack_pointer++;
                                unsigned short high = memory[0x0100 + stack_pointer];
                            program_counter = (high << 8) | low;
                            return;
                            break;
                        case 0x60: // Return from subroutine
                            // Pull PC low
                            stack_pointer++;
                                low = memory[0x0100 + stack_pointer];
                                stack_pointer++;
                                high = memory[0x0100 + stack_pointer];
                            program_counter = (high << 8) | low;
                            
                            // 6502 increments PC after pulling
                            program_counter++;
                            instruction_length = 0;
                            break;
                    }
                }
                else {
                    switch (bbb) {
                            
                        case 1: // Zero Page
                            addr = memory[program_counter + 1];
                            operand = func_operand(addr);
                            instruction_length = 2;
                            break;
                            
                        case 3: // Absolute
                            addr = memory[program_counter + 1] |
                            (memory[program_counter + 2] << 8);
                            instruction_length = 3;
                            operand = func_operand(addr);
                            break;
                            
                        case 5: // Zero Page,X  (used by STY/LDY)
                            addr = (memory[program_counter + 1] + x) & 0xFF;
                            operand = func_operand(addr);
                            instruction_length = 2;
                            break;
                            
                        case 7: // Absolute,X (LDY only)
                            addr = memory[program_counter + 1] |
                            (memory[program_counter + 2] << 8);
                            instruction_length = 3;
                            addr += x;
                            operand = func_operand(addr);
                            break;
                            
                        case 2: // Indirect (JMP only)
                            addr = memory[program_counter + 1] |
                            (memory[program_counter + 2] << 8);
                            instruction_length = 2;
                            if(opcode == 0x6C)
                                instruction_length++;
                            
                            // 6502 page boundary bug emulation
                        {
                            unsigned short lo = func_operand(addr);
                            unsigned short hi = memory[(addr & 0xFF00) |
                                                       ((addr + 1) & 0x00FF)];
                            addr = lo | (hi << 8);
                        }
                            break;
                    }
                    
                    switch (aaa) {
                        case 1: // Bit test
                            flags.zero = (acc & operand) == 0;
                            flags.negative = (operand >> 7) & 1;
                            flags.overflow = (operand >> 6) & 1;
                            break;
                        case 2: // Jump
                            program_counter = addr;
                            instruction_length = 0;
                            break;
                            
                        case 4: // Store y
                            storeb(addr, y);
                            break;
                            
                        case 5: // Load Y
                            y = operand;
                            flags.zero = (y == 0);
                            flags.negative = (y >> 7) & 1;
                            break;
                            
                        case 6: // Compare Y
                        {
                            unsigned char result = y - operand;
                            flags.carry = (y >= operand);
                            flags.zero = (result == 0);
                            flags.negative = (result >> 7) & 1;
                            break;
                        }
                            
                        case 7: // Compare x
                        {
                            unsigned char result = x - operand;
                            flags.carry = (x >= operand);
                            flags.zero = (result == 0);
                            flags.negative = (result >> 7) & 1;
                            break;
                        }
                            
                            
                    }
                    break;
                }
        }
    }
    program_counter += instruction_length;
}

void load(const char* filename) {
    SDL_IOStream* rom = SDL_IOFromFile(filename, "rb");
    if (rom == NULL) {
        SDL_Log("Base path: %s", SDL_GetBasePath());
        SDL_Log("fail"); // basic error logging, in later stages will crash program
        }
    Sint64 size = SDL_GetIOSize(rom);

    for(int i = 0; i < size; i++) {
        unsigned char value;
        SDL_ReadIO(rom, &value, 1);
        memory[32752 + i] = value; /* offset to make sure insturctions start in right place
                                    eventually we will have to make sure only PRG is loaded into CPU memory*/
    }
    SDL_CloseIO(rom);
    
}

void run(void) {
    load("example.nes");
    while(1) {
        decode(memory[program_counter]);
        SDL_Log("instruction %d\n at %d\n", memory[program_counter], program_counter);
    }
}
