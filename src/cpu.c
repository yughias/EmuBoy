#include <cpu.h>
#include <stdio.h>
#include <memory.h>

typedef void (*rotateFunc)(cpu_t*, uint8_t*);
typedef void (*aluFunc)(cpu_t*, uint8_t*, uint8_t);

//cpu NEW INSTRUCTION
void NOP(cpu_t*);
void STOP(cpu_t*);
void JR(cpu_t*, int8_t);
void JRNZ(cpu_t*, int8_t);
void JRZ(cpu_t*, int8_t);
void JRNC(cpu_t*, int8_t);
void JRC(cpu_t*, int8_t);
void LD_8(cpu_t*, uint8_t*, uint8_t);
void LD_16(cpu_t*, uint16_t*, uint16_t);
void ADD_16(cpu_t*, uint16_t*, uint16_t*);
void INC_16(cpu_t*, uint16_t*);
void DEC_16(cpu_t*, uint16_t*);
void INC_8(cpu_t*, uint8_t*);
void DEC_8(cpu_t*, uint8_t*);
void DAA(cpu_t*);
void CPL(cpu_t*);
void SCF(cpu_t*);
void CCF(cpu_t*);
void HLT(cpu_t*);
void RETNZ(cpu_t*);
void RETZ(cpu_t*);
void RETNC(cpu_t*);
void RETC(cpu_t*);
void LDH1(cpu_t*, uint8_t);
void LDH2(cpu_t*, uint8_t);
void ADD_SP(cpu_t*, int8_t);
void LD_SP(cpu_t*, int8_t);
void POP(cpu_t*, uint16_t*);
void RET(cpu_t*);
void JP(cpu_t*, uint16_t);
void JPNZ(cpu_t*, uint16_t);
void JPZ(cpu_t*, uint16_t);
void JPNC(cpu_t*, uint16_t);
void JPC(cpu_t*, uint16_t);
void EI(cpu_t*);
void DI(cpu_t*);
void CALL(cpu_t*, uint16_t);
void CALLNZ(cpu_t*, uint16_t);
void CALLZ(cpu_t*, uint16_t);
void CALLNC(cpu_t*, uint16_t);
void CALLC(cpu_t*, uint16_t);
void PUSH(cpu_t*, uint16_t);
void RST(cpu_t*, uint8_t);
void RLC(cpu_t*, uint8_t*);
void RRC(cpu_t*, uint8_t*);
void RLCA(cpu_t*);
void RRCA(cpu_t*);
void RLA(cpu_t*);
void RRA(cpu_t*);
void RLC(cpu_t*, uint8_t*);
void RRC(cpu_t*, uint8_t*);
void RL(cpu_t*, uint8_t*);
void RR(cpu_t*, uint8_t*);
void SLA(cpu_t*, uint8_t*);
void SRA(cpu_t*, uint8_t*);
void SWAP(cpu_t*, uint8_t*);
void SRL(cpu_t*, uint8_t*);
void BIT(cpu_t*, uint8_t, uint8_t*);
void SET(cpu_t*, uint8_t, uint8_t*);
void RES(cpu_t*, uint8_t, uint8_t*);
void ADD(cpu_t*, uint8_t*, uint8_t);
void ADC(cpu_t*, uint8_t*, uint8_t);
void SUB(cpu_t*, uint8_t*, uint8_t);
void SBC(cpu_t*, uint8_t*, uint8_t);
void AND(cpu_t*, uint8_t*, uint8_t);
void XOR(cpu_t*, uint8_t*, uint8_t);
void OR(cpu_t*, uint8_t*, uint8_t);
void CP(cpu_t*, uint8_t*, uint8_t);
void ADC_16(cpu_t*, uint16_t*, uint16_t);
void SBC_16(cpu_t*, uint16_t*, uint16_t);
void NEG(cpu_t*, uint8_t*);
void RETI(cpu_t*);
void RRD(cpu_t*);
void RLD(cpu_t*);
void LDI(cpu_t*, uint8_t*, uint8_t);
void LDD(cpu_t*, uint8_t*, uint8_t);

// flag masks to set/clear registers
#define SET_Z   0b10000000
#define SET_N   0b01000000
#define SET_H   0b00100000
#define SET_C   0b00010000

#define CLEAR_Z 0b01111111
#define CLEAR_N 0b10111111
#define CLEAR_H 0b11011111
#define CLEAR_C 0b11101111

#define SET_FLAG(f)          *cpu->F |= SET_ ## f
#define CLEAR_FLAG(f)        *cpu->F &= CLEAR_ ## f
#define CHANGE_FLAG(f, val)  *cpu->F ^= (-!!(val) ^ *cpu->F) & SET_ ## f

// cpu utility function
void setZero(cpu_t*, uint16_t);
bool calculateCarry(int, uint16_t, uint16_t, bool);

void initCPU(cpu_t* cpu){
    cpu->cycles = 0;
    cpu->F = (uint8_t*)cpu->AF;
    cpu->A = cpu->F + 1;
    cpu->C = (uint8_t*)cpu->BC;
    cpu->B = cpu->C + 1;
    cpu->E = (uint8_t*)cpu->DE;
    cpu->D = cpu->E + 1;
    cpu->L = (uint8_t*)cpu->HL;
    cpu->H = cpu->L + 1;
 
    *cpu->AF = 0x0000;
    *cpu->BC = 0x0000;
    *cpu->DE = 0x0000;
    *cpu->HL = 0x0000;
    *cpu->SP = 0x0000;
    *cpu->PC = 0x0000;

    //*cpu->AF = 0x01B0;
    //*cpu->BC = 0x0013;
    //*cpu->DE = 0x00D8;
    //*cpu->HL = 0x014D;
    //*cpu->SP = 0xFFFE;
    //*cpu->PC = 0x0100;

    cpu->HALTED = false;
    cpu->IME = false;
    cpu->EI_DELAY = false;
    cpu->INTERRUPT_DISPATCH = NONE;
}

void infoCPU(cpu_t* cpu){
    fprintf(stderr, "A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X ",
            *cpu->A, *cpu->F, *cpu->B, *cpu->C, *cpu->D, *cpu->E, *cpu->H, *cpu->L);

    fprintf(stderr, "SP: %04X PC: 00:%04X (%02X %02X %02X %02X)\n",
            *cpu->SP, *cpu->PC, *cpu->readMemory(*cpu->PC), *cpu->readMemory(*cpu->PC+1), *cpu->readMemory(*cpu->PC+2), *cpu->readMemory(*cpu->PC+3));
}

void dispatchInterrupt(cpu_t* cpu){
    cpu->cycles += 4;
    switch(cpu->INTERRUPT_DISPATCH){
        case PUSH_1:
        *cpu->SP -= 1;
        *cpu->writeMemory(*cpu->SP) = *cpu->PC >> 8;
        cpu->INTERRUPT_DISPATCH = PUSH_2;
        return;

        case PUSH_2:
        *cpu->SP -= 1;
        *cpu->writeMemory(*cpu->SP) = *cpu->PC & 0xFF;
        cpu->INTERRUPT_DISPATCH = PC_JMP;
        return;

        case PC_JMP:
        uint8_t mask = *cpu->readMemory(IE_ADDR) & *cpu->readMemory(IF_ADDR);
        uint16_t jmp_addr = 0;
        for(int i = 0; i < 5; i++){
            bool bit = mask & 1;
            mask >>= 1;
            if(bit){
                *cpu->writeMemory(IF_ADDR) &= ~(uint8_t)(1 << i);
                jmp_addr = 0x40 + 0x08*i;
                break;
            }
        }
        JP(cpu, jmp_addr);
        cpu->INTERRUPT_DISPATCH = NONE;
        return;
    }
}

void stepCPU(cpu_t* cpu){
    if(cpu->INTERRUPT_DISPATCH != NONE){
        dispatchInterrupt(cpu);
        return;
    }

    if(*cpu->readMemory(IE_ADDR) & *cpu->readMemory(IF_ADDR)){
        cpu->HALTED = false;
        if(cpu->IME & !cpu->EI_DELAY){
            cpu->IME = false;
            cpu->INTERRUPT_DISPATCH = PUSH_1;
            cpu->cycles += 8;
            return;
        }
    }

    if(cpu->EI_DELAY)
        cpu->EI_DELAY = false;

    if(cpu->HALTED){
        cpu->cycles += 4;
        return;
    }

    const static rotateFunc rot[8]  = { RLC, RRC, RL,   RR,   SLA, SRA,   SWAP, SRL   };
    const static aluFunc    alu[8]  = { ADD, ADC, SUB,  SBC,  AND,  XOR,  OR,   CP    };

    uint8_t*  r[8] = { cpu->B, cpu->C, cpu->D, cpu->E, cpu->H, cpu->L, (uint8_t*)(uintptr_t)*cpu->HL, cpu->A };
    uint16_t* rp[4] = { cpu->BC, cpu->DE, cpu->HL, cpu->SP };
    uint16_t* rp2[4] = { cpu->BC, cpu->DE, cpu->HL, cpu->AF };

    uint8_t opcode = *cpu->readMemory(*cpu->PC);
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t q;
    uint8_t p;

    uint8_t  val8;
    uint16_t val16;
    uint16_t nn;
    uint16_t old_PC;

    switch(opcode){
        case 0xCB:
        *cpu->PC += 1;
        opcode = *cpu->readMemory(*cpu->PC);
        x = opcode >> 6;
        y = (opcode >> 3) & 0b111;
        z = opcode & 0b111;
        q = y & 0b1;
        p = (y >> 1) & 0b11;

        *cpu->PC += 1;
        uint8_t memptr = (uint8_t)(uintptr_t)r[6];
        if(z == 6){
            cpu->cycles += 4;
            if(x == 1)
                r[z] = cpu->readMemory((uintptr_t)r[z]);
            else
                r[z] = cpu->writeMemory((uintptr_t)r[z]);
        }
        switch(x){
            case 0:
            if(z == 6)
                cpu->cycles += 4;
            rotateFunc function = rot[y];
            (*function)(cpu, r[z]);
            break;

            case 1:
            BIT(cpu, y, r[z]);
            break;

            case 2:
            if(z == 6)
                cpu->cycles += 4;
            RES(cpu, y, r[z]);
            break;

            case 3:
            if(z == 6)
                cpu->cycles += 4;
            SET(cpu, y, r[z]);
            break;
        }
        cpu->cycles += 8;
        break;

        default:
        x = opcode >> 6;
        y = (opcode >> 3) & 0b111;
        z = opcode & 0b111;
        q = y & 0b1;
        p = (y >> 1) & 0b11;

        switch(x){
            case 0:
            switch(z){
                case 0:
                switch(y){
                    case 0:
                    *cpu->PC += 1;
                    NOP(cpu);
                    cpu->cycles += 4;
                    break;

                    case 1:
                    nn = *(uint16_t*)cpu->readMemory(*cpu->PC+1);
                    *cpu->PC += 3;
                    LD_16(cpu, (uint16_t*)cpu->writeMemory(nn), *cpu->SP);
                    cpu->cycles += 20;
                    break;

                    case 2:
                    *cpu->PC += 1;
                    STOP(cpu);
                    cpu->cycles += 4;
                    break;

                    case 3:
                    *cpu->PC += 2;
                    JR(cpu, *cpu->readMemory(*cpu->PC-1));
                    cpu->cycles += 12;
                    break;

                    case 4:
                    *cpu->PC += 2;
                    old_PC = *cpu->PC;
                    JRNZ(cpu, *cpu->readMemory(*cpu->PC-1));
                    if(old_PC == *cpu->PC)
                        cpu->cycles += 8;
                    else
                        cpu->cycles += 12;
                    break;

                    case 5:
                    *cpu->PC += 2;
                    JRZ(cpu, *cpu->readMemory(*cpu->PC-1));
                    if(old_PC == *cpu->PC)
                        cpu->cycles += 8;
                    else
                        cpu->cycles += 12;
                    break;

                    case 6:
                    *cpu->PC += 2;
                    old_PC = *cpu->PC;
                    JRNC(cpu, *cpu->readMemory(*cpu->PC-1));
                    if(old_PC == *cpu->PC)
                        cpu->cycles += 8;
                    else
                        cpu->cycles += 12;
                    break;

                    case 7:
                    *cpu->PC += 2;
                    old_PC = *cpu->PC;
                    JRC(cpu, *cpu->readMemory(*cpu->PC-1));
                    if(old_PC == *cpu->PC)
                        cpu->cycles += 8;
                    else
                        cpu->cycles += 12;
                    break;
                }
                break;

                case 1:
                switch(q){
                    case 0:
                    nn = *(uint16_t*)cpu->readMemory(*cpu->PC+1);
                    *cpu->PC += 3;
                    LD_16(cpu, rp[p], nn);
                    cpu->cycles += 12;
                    break;

                    case 1:
                    *cpu->PC += 1;
                    ADD_16(cpu, rp[2], rp[p]);
                    cpu->cycles += 8;
                    break;
                }
                break;

                case 2:
                switch(q){
                    case 0:
                    switch(p){
                        case 0:
                        *cpu->PC += 1;
                        LD_8(cpu, cpu->writeMemory(*cpu->BC), *cpu->A);
                        break;

                        case 1:
                        *cpu->PC += 1;
                        LD_8(cpu, cpu->writeMemory(*cpu->DE), *cpu->A);
                        break;

                        case 2:
                        *cpu->PC += 1;
                        LDI(cpu, cpu->writeMemory(*cpu->HL), *cpu->A);
                        break;

                        case 3:
                        *cpu->PC += 1;
                        LDD(cpu, cpu->writeMemory(*cpu->HL), *cpu->A);
                        break;
                    }
                    cpu->cycles += 8;
                    break;

                    case 1:
                    switch(p){
                        case 0:
                        *cpu->PC += 1;
                        LD_8(cpu, cpu->A, *cpu->readMemory(*cpu->BC));
                        break;

                        case 1:
                        *cpu->PC += 1;
                        LD_8(cpu, cpu->A, *cpu->readMemory(*cpu->DE));
                        break;

                        case 2:
                        *cpu->PC += 1;
                        LDI(cpu, cpu->A, *cpu->readMemory(*cpu->HL));
                        break;

                        case 3:
                        *cpu->PC += 1;
                        LDD(cpu, cpu->A, *cpu->readMemory(*cpu->HL));
                        break;
                    }
                    cpu->cycles += 8;
                    break;
                }
                break;

                case 3:
                switch(q){
                    case 0:
                    *cpu->PC += 1;
                    INC_16(cpu, rp[p]);
                    break;

                    case 1:
                    *cpu->PC += 1;
                    DEC_16(cpu, rp[p]);
                    break;
                }
                cpu->cycles += 8;
                break;

                case 4:
                *cpu->PC += 1;
                if(y == 6){
                    r[y] = cpu->writeMemory((uintptr_t)r[y]);
                    cpu->cycles += 8;
                }
                INC_8(cpu, r[y]);
                cpu->cycles += 4;
                break;

                case 5:
                *cpu->PC += 1;
                if(y == 6){
                    r[y] = cpu->writeMemory((uintptr_t)r[y]);
                    cpu->cycles += 8;
                }
                DEC_8(cpu, r[y]);
                cpu->cycles += 4;
                break;

                case 6:
                if(y == 6){
                    r[y] = cpu->writeMemory((uintptr_t)r[y]);
                    cpu->cycles += 4;
                }
                val8 = *cpu->readMemory(*cpu->PC+1);
                *cpu->PC += 2;
                LD_8(cpu, r[y], val8);
                cpu->cycles += 8;
                break;

                case 7:
                *cpu->PC += 1;
                switch(y){
                    case 0:
                    RLCA(cpu);
                    break;

                    case 1:
                    RRCA(cpu);
                    break;

                    case 2:
                    RLA(cpu);
                    break;

                    case 3:
                    RRA(cpu);
                    break;

                    case 4:
                    DAA(cpu);
                    break;
                    
                    case 5:
                    CPL(cpu);
                    break;

                    case 6:
                    SCF(cpu);
                    break;

                    case 7:
                    CCF(cpu);
                    break;
                }
                cpu->cycles += 4;
                break;
            }
            break;

            case 1:
            *cpu->PC += 1;
            if(z == 6 && y == 6){
                HLT(cpu);
                cpu->cycles += 4;
            } else {
                if(y == 6){
                    r[y] = cpu->writeMemory((uintptr_t)r[y]);
                    cpu->cycles += 4;
                }
                if(z == 6){
                    r[z] = cpu->readMemory((uintptr_t)r[z]);
                    cpu->cycles += 4;
                }
                LD_8(cpu, r[y], *r[z]);
            }
            cpu->cycles += 4;
            break;

            case 2:
            if(z == 6){
                r[z] = cpu->readMemory((uintptr_t)r[z]);
                cpu->cycles += 4;
            }
            aluFunc function = alu[y];
            *cpu->PC += 1;
            (*function)(cpu, cpu->A, *r[z]);
            cpu->cycles += 4;
            break;

            case 3:
            switch(z){
                case 0:
                *cpu->PC += 1;
                old_PC = *cpu->PC;
                switch(y){
                    case 0:
                    RETNZ(cpu);
                    if(*cpu->PC == old_PC)
                        cpu->cycles += 8;
                    else
                        cpu->cycles += 20;
                    break;

                    case 1:
                    RETZ(cpu);
                    if(*cpu->PC == old_PC)
                        cpu->cycles += 8;
                    else
                        cpu->cycles += 20;
                    break;

                    case 2:
                    RETNC(cpu);
                    if(*cpu->PC == old_PC)
                        cpu->cycles += 8;
                    else
                        cpu->cycles += 20;
                    break;

                    case 3:
                    RETC(cpu);
                    if(*cpu->PC == old_PC)
                        cpu->cycles += 8;
                    else
                        cpu->cycles += 20;
                    break;

                    case 4:
                    val8 = *cpu->readMemory(*cpu->PC);
                    *cpu->PC += 1;
                    LDH1(cpu, val8);
                    cpu->cycles += 12;
                    break;

                    case 5:
                    val8 = *cpu->readMemory(*cpu->PC);
                    *cpu->PC += 1;
                    ADD_SP(cpu, val8);
                    cpu->cycles += 16;
                    break;

                    case 6:
                    val8 = *cpu->readMemory(*cpu->PC);
                    *cpu->PC += 1;
                    LDH2(cpu, val8);
                    cpu->cycles += 12;
                    break;

                    case 7:
                    val8 = *cpu->readMemory(*cpu->PC);
                    *cpu->PC += 1;
                    LD_SP(cpu, val8);
                    cpu->cycles += 12;
                    break;
                }
                break;

                case 1:
                *cpu->PC += 1;
                switch(q){
                    case 0:
                    POP(cpu, rp2[p]);
                    cpu->cycles += 12;
                    break;

                    case 1:
                    switch(p){
                        case 0:
                        RET(cpu);
                        cpu->cycles += 16;
                        break;

                        case 1:
                        RETI(cpu);
                        cpu->cycles += 16;
                        break;

                        case 2:
                        JP(cpu, *rp[2]);
                        cpu->cycles += 4;
                        break;

                        case 3:
                        LD_16(cpu, cpu->SP, *rp[2]);
                        cpu->cycles += 8;
                        break;
                    }
                    break;
                }
                break;

                case 2:
                val16 = *(uint16_t*)cpu->readMemory(*cpu->PC+1);
                old_PC = *cpu->PC;
                switch(y){
                    case 0:
                    *cpu->PC += 3;
                    JPNZ(cpu, val16);
                    if(old_PC == *cpu->PC)
                        cpu->cycles += 12;
                    else
                        cpu->cycles += 16;
                    break;

                    case 1:
                    *cpu->PC += 3;
                    JPZ(cpu, val16);
                    if(old_PC == *cpu->PC)
                        cpu->cycles += 12;
                    else
                        cpu->cycles += 16;
                    break;

                    case 2:
                    *cpu->PC += 3;
                    JPNC(cpu, val16);
                    if(old_PC == *cpu->PC)
                        cpu->cycles += 12;
                    else
                        cpu->cycles += 16;
                    break;

                    case 3:
                    *cpu->PC += 3;
                    JPC(cpu, val16);
                    if(old_PC == *cpu->PC)
                        cpu->cycles += 12;
                    else
                        cpu->cycles += 16;
                    break;
                    
                    case 4:
                    *cpu->PC += 1;
                    LDH1(cpu, *cpu->C);
                    cpu->cycles += 8;
                    break;

                    case 5:
                    *cpu->PC += 3;
                    LD_8(cpu, cpu->writeMemory(val16), *cpu->A);
                    cpu->cycles += 16;
                    break;

                    case 6:
                    *cpu->PC += 1;
                    LDH2(cpu, *cpu->C);
                    cpu->cycles += 8;
                    break;

                    case 7:
                    *cpu->PC += 3;
                    LD_8(cpu, cpu->A, *cpu->readMemory(val16));
                    cpu->cycles += 16;
                    break;
                }
                break;
            
                case 3:
                switch(y){
                    case 0:
                    val16 = *(uint16_t*)cpu->readMemory(*cpu->PC+1);
                    *cpu->PC += 3;
                    JP(cpu, val16);
                    cpu->cycles += 16;
                    break;

                    case 6:
                    *cpu->PC += 1;
                    DI(cpu);
                    cpu->cycles += 4;
                    break;

                    case 7:
                    *cpu->PC += 1;
                    EI(cpu);
                    cpu->EI_DELAY = true;
                    cpu->cycles += 4;
                    break;
                }
                break;

                case 4:
                val16 = *(uint16_t*)cpu->readMemory(*cpu->PC+1);
                *cpu->PC += 3;
                old_PC = *cpu->PC;
                switch(y){
                    case 0:
                    CALLNZ(cpu, val16);
                    break;
                    
                    case 1:
                    CALLZ(cpu, val16);
                    break;
                    
                    case 2:
                    CALLNC(cpu, val16);
                    break;
                    
                    case 3:
                    CALLC(cpu, val16);
                    break;
                    
                    case 4:
                    printf("empty opcode\n");
                    break;
                    
                    case 5:
                    printf("empty opcode\n");
                    break;
                    
                    case 6:
                    printf("empty opcode\n");
                    break;
                    
                    case 7:
                    printf("empty opcode\n");
                    break;
                }
                if(old_PC == *cpu->PC)
                    cpu->cycles += 12;
                else
                    cpu->cycles += 24;
                break;

                case 5:
                switch(q){
                    case 0:
                    *cpu->PC += 1;
                    PUSH(cpu, *rp2[p]);
                    cpu->cycles += 16;
                    break;

                    case 1:
                    if(p == 0){
                        val16 = *(uint16_t*)cpu->readMemory(*cpu->PC+1);
                        *cpu->PC += 3;
                        CALL(cpu, val16);
                        cpu->cycles += 24;
                    }
                    break;
                }
                break;

                case 6:
                val8 = *cpu->readMemory(*cpu->PC+1);
                aluFunc function = alu[y];
                *cpu->PC += 2;
                (*function)(cpu, cpu->A, val8);
                cpu->cycles += 8;
                break;

                case 7:
                *cpu->PC += 1;
                RST(cpu, y);
                cpu->cycles += 16;
                break;
            }
            break;
        }
        break;
    }

    // in gameboy cpu, lower bits of F register are always 0, clear them if they're set
    *cpu->F &= SET_C | SET_H | SET_N | SET_Z;

    // 'clean' IE and IF flag highest bit
    *cpu->writeMemory(IE_ADDR) &= VBLANK_IRQ | STAT_IRQ | TIMER_IRQ | SERIAL_IRQ | JOYPAD_IRQ;
    *cpu->writeMemory(IF_ADDR) &= VBLANK_IRQ | STAT_IRQ | TIMER_IRQ | SERIAL_IRQ | JOYPAD_IRQ;
}

// Z80 INSTRUCTIONS

void NOP(cpu_t* cpu){
}

void STOP(cpu_t* cpu){
    printf("entered stop opcode\n");     
}

void JR(cpu_t* cpu, int8_t d){
    *cpu->PC += d;
}

void JRNZ(cpu_t* cpu, int8_t d){
    if(!(*cpu->F & SET_Z))
        JR(cpu, d);
}

void JRZ(cpu_t* cpu, int8_t d){
    if((*cpu->F & SET_Z))
        JR(cpu, d);
}

void JRNC(cpu_t* cpu, int8_t d){
    if(!(*cpu->F & SET_C))
        JR(cpu, d);
}

void JRC(cpu_t* cpu, int8_t d){
    if(*cpu->F & SET_C)
        JR(cpu, d);
}

void LD_8(cpu_t* cpu, uint8_t* reg, uint8_t val){
    *reg = val;
}

void LD_16(cpu_t* cpu, uint16_t* reg, uint16_t val){
    *reg = val;
}

void ADD_16(cpu_t* cpu, uint16_t* regDst, uint16_t* regSrc){
    CLEAR_FLAG(N);
    bool carry = calculateCarry(16, *regDst, *regSrc, 0);
    CHANGE_FLAG(C, carry);

    bool aux_carry = calculateCarry(12, *regDst, *regSrc, 0);
    CHANGE_FLAG(H, aux_carry);

    *regDst += *regSrc;
}

void INC_16(cpu_t* cpu, uint16_t* reg){
    *reg += 1;
}

void DEC_16(cpu_t* cpu, uint16_t* reg){
    *reg -= 1;
}

void INC_8(cpu_t* cpu, uint8_t* reg){
    bool aux_carry = calculateCarry(4, *reg, 1, 0);
    CHANGE_FLAG(H, aux_carry);
    CLEAR_FLAG(N);

    *reg += 1;
    setZero(cpu, *reg);
}

void DEC_8(cpu_t* cpu, uint8_t* reg){
    bool aux_carry = calculateCarry(4, *reg, -1, 0);
    CHANGE_FLAG(H, !aux_carry);
    SET_FLAG(N);

    *reg -= 1;
    setZero(cpu, *reg);
}

void DAA(cpu_t* cpu){
    // explanation at:
    //      https://forums.nesdev.org/viewtopic.php?t=15944


    if(!(*cpu->F & SET_N)){  // after an addition, adjust if (half-)carry occurred or if result is out of bounds
        if((*cpu->F & SET_C) || *cpu->A > 0x99){
            *cpu->A += 0x60;
            SET_FLAG(C);
        }
        
        if((*cpu->F & SET_H) || (*cpu->A & 0x0f) > 0x09)
            *cpu->A += 0x6;
    } else {  // after a subtraction, only adjust if (half-)carry occurred
        if(*cpu->F & SET_C)
            *cpu->A -= 0x60;
        
        if(*cpu->F & SET_H)
            *cpu->A -= 0x6;
    }

    setZero(cpu, *cpu->A);
    CLEAR_FLAG(H);
}

void CPL(cpu_t* cpu){
    *cpu->A = ~(*cpu->A);
    SET_FLAG(N);
    SET_FLAG(H);
}

void SCF(cpu_t* cpu){
    SET_FLAG(C);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);
}

void CCF(cpu_t* cpu){
    bool carry = *cpu->F & SET_C;
    CHANGE_FLAG(C, !carry);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);
}

void HLT(cpu_t* cpu){
    cpu->HALTED = true;
}

void RETNZ(cpu_t* cpu){
    if(!(*cpu->F & SET_Z))
        POP(cpu, cpu->PC);
}

void RETZ(cpu_t* cpu){
    if(*cpu->F & SET_Z)
        POP(cpu, cpu->PC);
}

void RETNC(cpu_t* cpu){
    if(!(*cpu->F & SET_C))
        POP(cpu, cpu->PC);
}

void RETC(cpu_t* cpu){
    if(*cpu->F & SET_C)
        POP(cpu, cpu->PC);
}

void LDH1(cpu_t* cpu, uint8_t n){
    *cpu->writeMemory(0xFF00 + n) = *cpu->A;
}

void LDH2(cpu_t* cpu, uint8_t n){
    *cpu->A = *cpu->readMemory(0xFF00 + n);
}


void ADD_SP(cpu_t* cpu, int8_t n){
    uint16_t res = *cpu->SP + n;
    bool carry = calculateCarry(8, *cpu->SP, n, 0);
    CHANGE_FLAG(C, carry);

    bool aux_carry = calculateCarry(4, *cpu->SP, n, 0);
    CHANGE_FLAG(H, aux_carry);

    *cpu->SP = res;
    CLEAR_FLAG(Z);
    CLEAR_FLAG(N);
}

void LD_SP(cpu_t* cpu, int8_t n){
    uint16_t tmp = *cpu->SP;
    ADD_SP(cpu, n);
    *cpu->HL = *cpu->SP;
    *cpu->SP = tmp;
}

void POP(cpu_t* cpu, uint16_t* reg){
    *reg = *(uint16_t*)cpu->readMemory(*cpu->SP);
    *cpu->SP = *cpu->SP + 2; 
}

void RET(cpu_t* cpu){
    POP(cpu, cpu->PC);
}

void JP(cpu_t* cpu, uint16_t val){
    *cpu->PC = val;
}

void JPNZ(cpu_t* cpu, uint16_t val){
    if(!(*cpu->F & SET_Z))
        JP(cpu, val);
}

void JPZ(cpu_t* cpu, uint16_t val){
    if(*cpu->F & SET_Z)
        JP(cpu, val);
}

void JPNC(cpu_t* cpu, uint16_t val){
    if(!(*cpu->F & SET_C))
        JP(cpu, val);
}

void JPC(cpu_t* cpu, uint16_t val){
    if(*cpu->F & SET_C)
        JP(cpu, val);
}

void DI(cpu_t* cpu){
    cpu->IME = false;
}

void EI(cpu_t* cpu){
    cpu->IME = true;
}

void CALL(cpu_t* cpu, uint16_t val){
    PUSH(cpu, *cpu->PC);
    *cpu->PC = val;
}

void CALLNZ(cpu_t* cpu, uint16_t val){
    if(!(*cpu->F & SET_Z))
        CALL(cpu, val);
}

void CALLZ(cpu_t* cpu, uint16_t val){
    if(*cpu->F & SET_Z)
        CALL(cpu, val);
}

void CALLNC(cpu_t* cpu, uint16_t val){
    if(!(*cpu->F & SET_C))
        CALL(cpu, val);
}

void CALLC(cpu_t* cpu, uint16_t val){
    if(*cpu->F & SET_C)
        CALL(cpu, val);
}

void PUSH(cpu_t* cpu, uint16_t val){
    *cpu->writeMemory(*cpu->SP-1) = val >> 8;
    *cpu->writeMemory(*cpu->SP-2) = val & 0xFF;
    *cpu->SP = *cpu->SP - 2;
}

void RST(cpu_t* cpu, uint8_t addr){
    CALL(cpu, addr*8);
}

void RLCA(cpu_t* cpu){
    bool carry = *cpu->A & 0b10000000;
    CHANGE_FLAG(C, carry);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);
    CLEAR_FLAG(Z);

    *cpu->A = (*cpu->A << 1) | (carry);
}

void RRCA(cpu_t* cpu){
    bool carry = *cpu->A & 0b1;
    CHANGE_FLAG(C, carry);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);
    CLEAR_FLAG(Z);
    *cpu->A = (*cpu->A >> 1) | (carry << 7);
}

void RLA(cpu_t* cpu){
    bool carry = *cpu->F & SET_C;
    CHANGE_FLAG(C, *cpu->A & 0b10000000);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);
    CLEAR_FLAG(Z);
    *cpu->A = (*cpu->A << 1) | carry;
}

void RRA(cpu_t* cpu){
    bool carry = *cpu->F & SET_C;
    CHANGE_FLAG(C, *cpu->A & 0b1);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);
    CLEAR_FLAG(Z);
    *cpu->A = (*cpu->A >> 1) | (carry << 7);
}

void RLC(cpu_t* cpu, uint8_t* reg){
    bool msb = *reg >> 7;
    CHANGE_FLAG(C, msb);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);
    *reg = (*reg << 1) | msb;

    setZero(cpu, *reg);
}

void RRC(cpu_t* cpu, uint8_t* reg){
    bool lsb = *reg & 0b1;
    CHANGE_FLAG(C, lsb);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);
    *reg = (lsb << 7) | (*reg >> 1);

    setZero(cpu, *reg);
}

void RL(cpu_t* cpu, uint8_t* reg){
    bool carry = *cpu->F & SET_C;
    bool msb = *reg >> 7;
    *reg = (*reg << 1) | carry;
    CHANGE_FLAG(C, msb);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);

    setZero(cpu, *reg);
}

void RR(cpu_t* cpu, uint8_t* reg){
    bool carry = *cpu->F & SET_C;
    bool lsb = *reg & 0b1;
    *reg = (*reg >> 1) | (carry << 7);
    CHANGE_FLAG(C, lsb);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);

    setZero(cpu, *reg);
}

void SLA(cpu_t* cpu, uint8_t* reg){
    bool msb = *reg >> 7;
    CHANGE_FLAG(C, msb);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);

    *reg = *reg << 1;

    setZero(cpu, *reg);
}

void SRA(cpu_t* cpu, uint8_t* reg){
    bool sign = *reg >> 7;
    bool lsb = *reg & 0b1;
    CHANGE_FLAG(C, lsb);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);

    *reg = (*reg >> 1) | (sign << 7);

    setZero(cpu, *reg);
}

void SWAP(cpu_t* cpu, uint8_t* reg){
    uint8_t high_nibble = *reg >> 4;
    *reg <<= 4;
    *reg |= high_nibble;

    setZero(cpu, *reg);
    CLEAR_FLAG(N);
    CLEAR_FLAG(H);
    CLEAR_FLAG(C);
}


void SRL(cpu_t* cpu, uint8_t* reg){
    bool lsb = *reg & 0b1;
    CHANGE_FLAG(C, lsb);
    CLEAR_FLAG(H);
    CLEAR_FLAG(N);

    *reg = *reg >> 1;

    setZero(cpu, *reg);
}

void BIT(cpu_t* cpu, uint8_t bit, uint8_t* reg){
    uint8_t masked_bit = *reg & (1 << bit);
    CHANGE_FLAG(Z, masked_bit == 0);
    SET_FLAG(H);
    CLEAR_FLAG(N);
}

void RES(cpu_t* cpu, uint8_t bit, uint8_t* reg){
    *reg = *reg & (~(uint8_t)(1 << bit));
}

void SET(cpu_t* cpu, uint8_t bit, uint8_t* reg){
    *reg = * reg | (1 << bit);
}

void ADD(cpu_t* cpu, uint8_t* reg, uint8_t val){
    uint8_t res = *reg + val;
    bool carry = calculateCarry(8, *reg, val, 0);
    CHANGE_FLAG(C, carry);

    bool aux_carry = calculateCarry(4, *reg, val, 0);
    CHANGE_FLAG(H, aux_carry);

    *reg = res;
    setZero(cpu, *reg);
    CLEAR_FLAG(N);
}

void ADC(cpu_t* cpu, uint8_t* reg, uint8_t val){
    bool carry = *cpu->F & SET_C;
    uint8_t res = *reg + val + carry;

    bool new_carry = calculateCarry(8, *reg, val, carry);
    CHANGE_FLAG(C, new_carry);

    bool aux_carry = calculateCarry(4, *reg, val, carry);
    CHANGE_FLAG(H, aux_carry);

    *reg = res;
    setZero(cpu, *reg);
    CLEAR_FLAG(N);
}

void SUB(cpu_t* cpu, uint8_t* reg, uint8_t val){
    val = ~val + 1;
    bool carry = calculateCarry(8, *reg, val - 1, 1);
    CHANGE_FLAG(C, !carry);

    bool aux_carry = calculateCarry(4, *reg, val - 1, 1);
    CHANGE_FLAG(H, !aux_carry);

    uint8_t res = *reg + val;

    *reg = res;
    setZero(cpu, *reg);
    SET_FLAG(N);
}

void SBC(cpu_t* cpu, uint8_t* reg, uint8_t val){
    val = ~val + 1;
    bool carry = *cpu->F & SET_C;
    bool new_carry = calculateCarry(8, *reg, val - 1, !carry);
    bool aux_carry = calculateCarry(4, *reg, val - 1, !carry);

    CHANGE_FLAG(C, !new_carry);
    CHANGE_FLAG(H, !aux_carry);

    uint8_t res = *reg + val - carry;

    *reg = res;
    setZero(cpu, *reg);
    SET_FLAG(N);
}

void AND(cpu_t* cpu, uint8_t* reg, uint8_t val){
    *reg &= val;
    setZero(cpu, *reg);
    SET_FLAG(H);
    CLEAR_FLAG(C);
    CLEAR_FLAG(N);
}

void XOR(cpu_t* cpu, uint8_t* reg, uint8_t val){
    *reg ^= val;
    setZero(cpu, *reg);
    CLEAR_FLAG(H);
    CLEAR_FLAG(C);
    CLEAR_FLAG(N);
}

void OR(cpu_t* cpu, uint8_t* reg, uint8_t val){
    *reg |= val;
    setZero(cpu, *reg);
    CLEAR_FLAG(H);
    CLEAR_FLAG(C);
    CLEAR_FLAG(N);
}

void CP(cpu_t* cpu, uint8_t* reg, uint8_t val){
    uint8_t copy = *reg;
    SUB(cpu, reg, val);
    *reg = copy;
}

void ADC_16(cpu_t* cpu, uint16_t* reg, uint16_t val){
    bool carry = *cpu->F & SET_C;

    bool new_carry = calculateCarry(16, *reg, val, carry);
    CHANGE_FLAG(C, new_carry);
    
    bool aux_carry = calculateCarry(12, *reg, val, carry);
    CHANGE_FLAG(H, aux_carry);

    uint16_t res = *reg + val + carry;
           
    *reg = res;
    setZero(cpu, *reg);
    CLEAR_FLAG(N);
}

void SBC_16(cpu_t* cpu, uint16_t* reg, uint16_t val){
    val = ~val + 1;
    bool carry = *cpu->F & SET_C;
    bool new_carry = calculateCarry(16, *reg, val - 1, !carry);
    bool aux_carry = calculateCarry(12, *reg, val - 1, !carry);

    CHANGE_FLAG(C, !new_carry);
    CHANGE_FLAG(H, !aux_carry);

    uint16_t res = *reg + val - carry; 

    *reg = res;
    setZero(cpu, *reg);
    SET_FLAG(N);
}

void NEG(cpu_t* cpu, uint8_t* reg){
    uint8_t tmp = *reg;
    *reg = 0;
    SUB(cpu, reg, tmp);
}

void RETI(cpu_t* cpu){
    RET(cpu);
    EI(cpu);
}

void RRD(cpu_t* cpu){
    uint8_t tmpA = *cpu->A;
    uint8_t val = *cpu->readMemory(*cpu->HL);
    *cpu->A = (tmpA & 0xF0) | (val & 0xF);
    *cpu->writeMemory(*cpu->HL) = (val >> 4) | (tmpA << 4);
    setZero(cpu, *cpu->A);
    CLEAR_FLAG(N);
    CLEAR_FLAG(H);
}

void RLD(cpu_t* cpu){
    uint8_t tmpA = *cpu->A;
    uint8_t val = *cpu->readMemory(*cpu->HL);
    *cpu->A = (tmpA & 0xF0) | (val >> 4);
    *cpu->writeMemory(*cpu->HL) = (val << 4) | (tmpA & 0xF);
    setZero(cpu, *cpu->A);
    CLEAR_FLAG(N);
    CLEAR_FLAG(H);
}

// block instructions

void LDI(cpu_t* cpu, uint8_t* reg, uint8_t val){
    *reg = val;
    *cpu->HL += 1;
}

void LDD(cpu_t* cpu, uint8_t* reg, uint8_t val){
    *reg = val;
    *cpu->HL -= 1;
}

void setZero(cpu_t* cpu, uint16_t val){
    CHANGE_FLAG(Z, val == 0);
}

bool calculateCarry(int bit, uint16_t a, uint16_t b, bool cy) {
  int32_t result = a + b + cy;
  int32_t carry = result ^ a ^ b;
  return carry & (1 << bit);
}