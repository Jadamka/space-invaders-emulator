#include "cpu.h"
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

CPU::CPU()
{
    memset(memory, 0, sizeof(memory));
    memset(registers, 0, sizeof(registers));
    conditionBits = 0x02; // 0000 0010
    pc = 0;
    sp = 0;
}

void CPU::cycle()
{

}

void CPU::print_ins_hex(const char* ins, uint8_t highByte, uint8_t lowByte, bool hasLowByte)
{
    if(hasLowByte)
    {
        std::cout << ins
                  << std::hex << std::uppercase
                  << std::setw(2) << std::setfill('0') << static_cast<int32_t>(highByte)
                  << std::setw(2) << std::setfill('0') << static_cast<int32_t>(lowByte)
                  << std::dec << "\n";
    }
    else
    {
        std::cout << ins
                  << std::hex << std::uppercase
                  << std::setw(2) << std::setfill('0') << static_cast<int32_t>(highByte)
                  << std::dec << "\n";
    }
}

void CPU::set_flag(uint8_t reg, enum FlagBit flagBit, enum ConditionIns op, uint8_t number, uint16_t addr)
{
    uint8_t value;

    if(reg == REF_MEMORY)
        value = memory[addr];
    else
        value = registers[reg];

    if(flagBit == S)
    {
        if(op == CMP)
            value -= number;
        conditionBits = (conditionBits & 0x7F) | (value & 0x80);
    }
    else if(flagBit ==  Z)
    {
        if(op == CMP)
            value -= number;
        if(value == 0x00)
            conditionBits |= 0x40;
        else
            conditionBits &= 0xBF;
    }
    else if(flagBit == A)
    {
        value &= 0x0F;
        switch(op)
        {
        case INR:
            if((value + 1)> 0x0F)
                conditionBits |= 0x10;
            else
                conditionBits &= 0xEF;
            break;
        case DCR:
            if((value & 0x0F) == 0)
                conditionBits |= 0x10;
            else
                conditionBits &= 0xEF;
            //value-- (value is a copy of a variable, no need to do this operation)
            break;
        case DAA:
        case ADD:
        case ADC:
            if((value + number) > 0x0F)
              conditionBits |= 0x10;
            else
              conditionBits &= 0xEF;
            break;
        case SUB:
            if(value < (number & 0x0F))
                conditionBits |= 0x10;
            else
                conditionBits &= 0xEF;
            break;
        case CMP:
            {
                if(value < (number & 0x0F))
                    conditionBits |= 0x10;
                else
                    conditionBits &= 0xEF;

                uint8_t signA = (value & 0x80);
                uint8_t signB = (value & 0x80);
                if(signA != signB)
                    conditionBits ^= 0x01;
                break;
            }
        case ANA:
            if((value & 0x08) && (number & 0x08))
                conditionBits |= 0x10;
            else
                conditionBits &= 0xEF;
        default:
            break;
        }
    }
    else if(flagBit == P)
    {
        if(op == CMP)
            value -= number;

        uint8_t bitsCount = 0;

        while(value != 0x00)
        {
            if((value & 0x01) == 1)
                bitsCount++;
            value >>= 1;
        }

        if(bitsCount % 2 == 0)
            conditionBits |= 0x04;
        else
            conditionBits &= 0xFB;
    }
    else if(flagBit == C)
    {
        uint16_t sum = 0;
        switch(op)
        {
        case DAA:
        case ADD:
        case ADC:
            sum = value + number;
            if(sum > 0xFF)
                conditionBits |= 0x01;
            else
                conditionBits &= 0xFE;
            break;
        case SUB:
        case CMP:
            sum = value - number;
            if(value < number)
                conditionBits |= 0x01;
            else
                conditionBits &= 0xFE;
            break;
        default:
            break;
        }
    }
}

void CPU::disassembler()
{
    uint8_t opcode = memory[pc];

    switch(opcode)
    {
    case 0x00:
    case 0x08:
    case 0x10:
    case 0x18:
    case 0x20:
    case 0x28:
    case 0x30:
    case 0x38:
        std::cout << "NOP\n";
        pc += 1;
        break;
    case 0x01:
        print_ins_hex("LXI B, #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0x02:
        std::cout << "STAX B\n";
        memory[(registers[REG_B] << 8) | registers[REG_C]] = registers[REG_A];
        pc += 1;
        break;
    case 0x03:
        std::cout << "INX B\n";
        pc += 1;
        break;
    case 0x04:
        std::cout << "INR B\n";
        set_flag(REG_B, A, INR);
        registers[REG_B]++;
        set_flag(REG_B, S, INR);
        set_flag(REG_B, Z, INR);
        set_flag(REG_B, P, INR);
        pc += 1;
        break;
    case 0x05:
        std::cout << "DCR B\n";
        set_flag(REG_B, A, DCR);
        registers[REG_B]--;
        set_flag(REG_B, S, DCR);
        set_flag(REG_B, Z, DCR);
        set_flag(REG_B, P, DCR);
        pc += 1;
        break;
    case 0x06:
        print_ins_hex("MVI B, #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0x07:
        std::cout << "RLC\n";
        pc += 1;
        break;
    case 0x09:
        std::cout << "DAD B\n";
        pc += 1;
        break;
    case 0x0A:
        std::cout << "LDAX B\n";
        registers[REG_A] = memory[(registers[REG_B] << 8) | registers[REG_C]];
        pc += 1;
        break;
    case 0x0B:
        std::cout << "DCX B\n";
        pc += 1;
        break;
    case 0x0C:
        std::cout << "INR C\n";
        set_flag(REG_C, A, INR);
        registers[REG_C]++;
        set_flag(REG_C, S);
        set_flag(REG_C, Z);
        set_flag(REG_C, P);
        pc += 1;
        break;
    case 0x0D:
        std::cout << "DCR C\n";
        set_flag(REG_C, A, DCR);
        registers[REG_C]--;
        set_flag(REG_C, S);
        set_flag(REG_C, Z);
        set_flag(REG_C, P);
        pc += 1;
        break;
    case 0x0E:
        print_ins_hex("MVI C, #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0x0F:
        std::cout << "RRC\n";
        pc += 1;
        break;
    case 0x11:
        print_ins_hex("LXI D, #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0x12:
        std::cout << "STAX D\n";
        memory[(registers[REG_D] << 8) | registers[REG_E]] = registers[REG_A];
        pc += 1;
        break;
    case 0x13:
        std::cout << "INX D\n";
        pc += 1;
        break;
    case 0x14:
        std::cout << "INR D\n";
        set_flag(REG_D, A, INR);
        registers[REG_D]++;
        set_flag(REG_D, S);
        set_flag(REG_D, Z);
        set_flag(REG_D, P);
        pc += 1;
        break;
    case 0x15:
        std::cout << "DCR D\n";
        set_flag(REG_D, A, DCR);
        registers[REG_D]--;
        set_flag(REG_D, S);
        set_flag(REG_D, Z);
        set_flag(REG_D, P);
        pc += 1;
        break;
    case 0x16:
        print_ins_hex("MVI D, #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0x17:
        std::cout << "RAL\n";
        pc += 1;
        break;
    case 0x19:
        std::cout << "DAD D\n";
        pc += 1;
        break;
    case 0x1A:
        std::cout << "LDAX D\n";
        registers[REG_A] = memory[(registers[REG_D] << 8) | registers[REG_E]];
        pc += 1;
        break;
    case 0x1B:
        std::cout << "DCX D\n";
        pc += 1;
        break;
    case 0x1C:
        std::cout << "INR E\n";
        set_flag(REG_E, A, INR);
        registers[REG_E]++;
        set_flag(REG_E, S);
        set_flag(REG_E, Z);
        set_flag(REG_E, P);
        pc += 1;
        break;
    case 0x1D:
        std::cout << "DCR E\n";
        set_flag(REG_E, A, DCR);
        registers[REG_E]--;
        set_flag(REG_E, S);
        set_flag(REG_E, Z);
        set_flag(REG_E, P);
        pc += 1;
        break;
    case 0x1E:
        print_ins_hex("MVI E, #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0x1F:
        std::cout << "RAR\n";
        pc += 1;
        break;
    case 0x21:
        print_ins_hex("LXI H, #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0x22:
        print_ins_hex("SHLD #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0x23:
        std::cout << "INX H\n";
        pc += 1;
        break;
    case 0x24:
        std::cout << "INR H\n";
        set_flag(REG_H, A, INR);
        registers[REG_H]++;
        set_flag(REG_H, S);
        set_flag(REG_H, Z);
        set_flag(REG_H, P);
        pc += 1;
        break;
    case 0x25:
        std::cout << "DCR H\n";
        set_flag(REG_H, A, DCR);
        registers[REG_H]--;
        set_flag(REG_H, S);
        set_flag(REG_H, Z);
        set_flag(REG_H, P);
        pc += 1;
        break;
    case 0x26:
        print_ins_hex("MVI H, #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0x27:
        std::cout << "DAA\n";
        if((registers[REG_A] & 0x0F) > 9 || ((conditionBits >> 4) & 0x01))
        {
          set_flag(REG_A, A, DAA, 0x06);
          registers[REG_A] += 0x06;
        }
        if((registers[REG_A] >> 4) > 9 || (conditionBits & 0x01))
        {
          set_flag(REG_A, C, DAA, 0x60);
          registers[REG_A] += 0x60;
        }
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x29:
        std::cout << "DAD H\n";
        pc += 1;
        break;
    case 0x2A:
        print_ins_hex("LHLD #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0x2B:
        std::cout << "DCX H\n";
        pc += 1;
        break;
    case 0x2C:
        std::cout << "INR L\n";
        set_flag(REG_L, A, INR);
        registers[REG_L]++;
        set_flag(REG_L, S);
        set_flag(REG_L, Z);
        set_flag(REG_L, P);
        pc += 1;
        break;
    case 0x2D:
        std::cout << "DCR L\n";
        set_flag(REG_L, A, DCR);
        registers[REG_L]--;
        set_flag(REG_L, S);
        set_flag(REG_L, Z);
        set_flag(REG_L, P);
        pc += 1;
        break;
    case 0x2E:
        print_ins_hex("MVI L, #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0x2F:
        std::cout << "CMA\n";
        registers[REG_A] = ~registers[REG_A];
        pc += 1;
        break;
    case 0x31:
        print_ins_hex("LXI SP, #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0x32:
        print_ins_hex("STA #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0x33:
        std::cout << "INX SP";
        pc += 1;
        break;
    case 0x34:
    {
        std::cout << "INR M\n";
        uint16_t addr = (registers[REG_H] << 8) | (registers[REG_L]);
        set_flag(REF_MEMORY, A, INR, 0, addr);
        memory[addr]++;
        set_flag(REF_MEMORY, S, INR, 0, addr);
        set_flag(REF_MEMORY, Z, INR, 0, addr);
        set_flag(REF_MEMORY, P, INR, 0, addr);
        pc += 1;
        break;
    }
    case 0x35:
    {
        std::cout << "DCR M\n";
        uint16_t addr = (registers[REG_H] << 8) | (registers[REG_L]);
        set_flag(REF_MEMORY, A, DCR, 0, addr);
        memory[addr]++;
        set_flag(REF_MEMORY, S, DCR, 0, addr);
        set_flag(REF_MEMORY, Z, DCR, 0, addr);
        set_flag(REF_MEMORY, P, DCR, 0, addr);
        pc += 1;
        break;
    }
    case 0x36:
        print_ins_hex("MVI M, #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0x37:
        std::cout << "STC\n";
        conditionBits |= 0x01;
        pc += 1;
        break;
    case 0x39:
        std::cout << "DAD SP\n";
        pc += 1;
        break;
    case 0x3A:
        print_ins_hex("LDA #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0x3B:
        std::cout << "DCX SP\n";
        pc += 1;
        break;
    case 0x3C:
        std::cout << "INR A\n";
        set_flag(REG_A, A, INR);
        registers[REG_A]++;
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x3D:
        std::cout << "DCR A\n";
        pc += 1;
        break;
    case 0x3E:
        print_ins_hex("MVI A, #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0x3F:
        std::cout << "CMC\n";
        conditionBits ^= 0x01;
        pc += 1;
        break;
    case 0x40:
        std::cout << "MOV B, B\n"; // NOP
        pc += 1;
        break;
    case 0x41:
        std::cout << "MOV B, C\n";
        registers[REG_B] = registers[REG_C];
        pc += 1;
        break;
    case 0x42:
        std::cout << "MOV B, D\n";
        registers[REG_B] = registers[REG_D];
        pc += 1;
        break;
    case 0x43:
        std::cout << "MOV B, E\n";
        registers[REG_B] = registers[REG_E];
        pc += 1;
        break;
    case 0x44:
        std::cout << "MOV B, H\n";
        registers[REG_B] = registers[REG_H];
        pc += 1;
        break;
    case 0x45:
        std::cout << "MOV B, L\n";
        registers[REG_B] = registers[REG_L];
        pc += 1;
        break;
    case 0x46:
        std::cout << "MOV B, M\n";
        registers[REG_B] = memory[(registers[REG_H] << 8) | registers[REG_L]];
        pc += 1;
        break;
    case 0x47:
        std::cout << "MOV B, A\n";
        registers[REG_B] = registers[REG_A];
        pc += 1;
        break;
    case 0x48:
        std::cout << "MOV C, B\n";
        registers[REG_C] = registers[REG_B];
        pc += 1;
        break;
    case 0x49:
        std::cout << "MOV C, C\n"; // NOP
        pc += 1;
        break;
    case 0x4A:
        std::cout << "MOV C, D\n";
        registers[REG_C] = registers[REG_D];
        pc += 1;
        break;
    case 0x4B:
        std::cout << "MOV C, E\n";
        registers[REG_C] = registers[REG_E];
        pc += 1;
        break;
    case 0x4C:
        std::cout << "MOV C, H\n";
        registers[REG_C] = registers[REG_H];
        pc += 1;
        break;
    case 0x4D:
        std::cout << "MOV C, L\n";
        registers[REG_C] = registers[REG_L];
        pc += 1;
        break;
    case 0x4E:
        std::cout << "MOV C, M\n";
        registers[REG_C] = memory[(registers[REG_H] << 8) | registers[REG_L]];
        pc += 1;
        break;
    case 0x4F:
        std::cout << "MOV C, A\n";
        registers[REG_C] = registers[REG_A];
        pc += 1;
        break;
    case 0x50:
        std::cout << "MOV D, B\n";
        registers[REG_D] = registers[REG_B];
        pc += 1;
        break;
    case 0x51:
        std::cout << "MOV D, C\n";
        registers[REG_D] = registers[REG_C];
        pc += 1;
        break;
    case 0x52:
        std::cout << "MOV D, D\n"; // NOP
        pc += 1;
        break;
    case 0x53:
        std::cout << "MOV D, E\n";
        registers[REG_D] = registers[REG_E];
        pc += 1;
        break;
    case 0x54:
        std::cout << "MOV D, H\n";
        registers[REG_D] = registers[REG_H];
        pc += 1;
        break;
    case 0x55:
        std::cout << "MOV D, L\n";
        registers[REG_D] = registers[REG_L];
        pc += 1;
        break;
    case 0x56:
        std::cout << "MOV D, M\n";
        registers[REG_D] = memory[(registers[REG_H] << 8) | registers[REG_L]];
        pc += 1;
        break;
    case 0x57:
        std::cout << "MOV D, A\n";
        registers[REG_D] = registers[REG_A];
        pc += 1;
        break;
    case 0x58:
        std::cout << "MOV E, B\n";
        registers[REG_E] = registers[REG_B];
        pc += 1;
        break;
    case 0x59:
        std::cout << "MOV E, C\n";
        registers[REG_E] = registers[REG_C];
        pc += 1;
        break;
    case 0x5A:
        std::cout << "MOV E, D\n";
        registers[REG_E] = registers[REG_D];
        pc += 1;
        break;
    case 0x5B:
        std::cout << "MOV E, E\n"; // NOP
        pc += 1;
        break;
    case 0x5C:
        std::cout << "MOV E, H\n";
        registers[REG_E] = registers[REG_H];
        pc += 1;
        break;
    case 0x5D:
        std::cout << "MOV E, L\n";
        registers[REG_E] = registers[REG_L];
        pc += 1;
        break;
    case 0x5E:
        std::cout << "MOV E, M\n";
        registers[REG_E] = memory[(registers[REG_H] << 8) | registers[REG_L]];
        pc += 1;
        break;
    case 0x5F:
        std::cout << "MOV E, A\n";
        registers[REG_E] = registers[REG_A];
        pc += 1;
        break;
    case 0x60:
        std::cout << "MOV H, B\n";
        registers[REG_H] = registers[REG_B];
        pc += 1;
        break;
    case 0x61:
        std::cout << "MOV H, C\n";
        registers[REG_H] = registers[REG_C];
        pc += 1;
        break;
    case 0x62:
        std::cout << "MOV H, D\n";
        registers[REG_H] = registers[REG_D];
        pc += 1;
        break;
    case 0x63:
        std::cout << "MOV H, E\n";
        registers[REG_H] = registers[REG_E];
        pc += 1;
        break;
    case 0x64:
        std::cout << "MOV H, H\n"; // NOP
        pc += 1;
        break;
    case 0x65:
        std::cout << "MOV H, L\n";
        registers[REG_H] = registers[REG_L];
        pc += 1;
        break;
    case 0x66:
        std::cout << "MOV H, M\n";
        registers[REG_H] = memory[(registers[REG_H] << 8) | registers[REG_L]];
        pc += 1;
        break;
    case 0x67:
        std::cout << "MOV H, A\n";
        registers[REG_H] = registers[REG_A];
        pc += 1;
        break;
    case 0x68:
        std::cout << "MOV L, B\n";
        registers[REG_L] = registers[REG_B];
        pc += 1;
        break;
    case 0x69:
        std::cout << "MOV L, C\n";
        registers[REG_L] = registers[REG_C];
        pc += 1;
        break;
    case 0x6A:
        std::cout << "MOV L, D\n";
        registers[REG_L] = registers[REG_D];
        pc += 1;
        break;
    case 0x6B:
        std::cout << "MOV L, E\n";
        registers[REG_L] = registers[REG_E];
        pc += 1;
        break;
    case 0x6C:
        std::cout << "MOV L, H\n";
        registers[REG_L] = registers[REG_H];
        pc += 1;
        break;
    case 0x6D:
        std::cout << "MOV L, L\n"; // NOP
        pc += 1;
        break;
    case 0x6E:
        std::cout << "MOV L, M\n";
        registers[REG_L] = memory[(registers[REG_H] << 8) | registers[REG_L]];
        pc += 1;
        break;
    case 0x6F:
        std::cout << "MOV L, A\n";
        registers[REG_L] = registers[REG_A];
        pc += 1;
        break;
    case 0x70:
        std::cout << "MOV M, B\n";
        memory[(registers[REG_H] << 8) | registers[REG_L]] = registers[REG_B];
        pc += 1;
        break;
    case 0x71:
        std::cout << "MOV M, C\n";
        memory[(registers[REG_H] << 8) | registers[REG_L]] = registers[REG_C];
        pc += 1;
        break;
    case 0x72:
        std::cout << "MOV M, D\n";
        memory[(registers[REG_H] << 8) | registers[REG_L]] = registers[REG_D];
        pc += 1;
        break;
    case 0x73:
        std::cout << "MOV M, E\n";
        memory[(registers[REG_H] << 8) | registers[REG_L]] = registers[REG_E];
        pc += 1;
        break;
    case 0x74:
        std::cout << "MOV M, H\n";
        memory[(registers[REG_H] << 8) | registers[REG_L]] = registers[REG_H];
        pc += 1;
        break;
    case 0x75:
        std::cout << "MOV M, L\n";
        memory[(registers[REG_H] << 8) | registers[REG_L]] = registers[REG_L];
        pc += 1;
        break;
    case 0x76:
        std::cout << "HLT\n";
        pc += 1;
        break;
    case 0x77:
        std::cout << "MOV M, A\n";
        memory[(registers[REG_H] << 8) | registers[REG_L]] = registers[REG_A];
        pc += 1;
        break;
    case 0x78:
        std::cout << "MOV A, B\n";
        registers[REG_A] = registers[REG_B];
        pc += 1;
        break;
    case 0x79:
        std::cout << "MOV A, C\n";
        registers[REG_A] = registers[REG_C];
        pc += 1;
        break;
    case 0x7A:
        std::cout << "MOV A, D\n";
        registers[REG_A] = registers[REG_D];
        pc += 1;
        break;
    case 0x7B:
        std::cout << "MOV A, E\n";
        registers[REG_A] = registers[REG_E];
        pc += 1;
        break;
    case 0x7C:
        std::cout << "MOV A, H\n";
        registers[REG_A] = registers[REG_H];
        pc += 1;
        break;
    case 0x7D:
        std::cout << "MOV A, L\n";
        registers[REG_A] = registers[REG_L];
        pc += 1;
        break;
    case 0x7E:
        std::cout << "MOV A, M\n";
        registers[REG_A] = memory[(registers[REG_H] << 8) | registers[REG_L]];
        pc += 1;
        break;
    case 0x7F:
        std::cout << "MOV A, A\n"; // NOP
        pc += 1;
        break;
    case 0x80:
        std::cout << "ADD B\n";
        set_flag(REG_A, A, ADD, registers[REG_B]);
        set_flag(REG_A, C, ADD, registers[REG_B]);
        registers[REG_A] += registers[REG_B];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x81:
        std::cout << "ADD C\n";
        set_flag(REG_A, A, ADD, registers[REG_C]);
        set_flag(REG_A, C, ADD, registers[REG_C]);
        registers[REG_A] += registers[REG_C];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x82:
        std::cout << "ADD D\n";
        set_flag(REG_A, A, ADD, registers[REG_D]);
        set_flag(REG_A, C, ADD, registers[REG_D]);
        registers[REG_A] += registers[REG_D];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x83:
        std::cout << "ADD E\n";
        set_flag(REG_A, A, ADD, registers[REG_E]);
        set_flag(REG_A, C, ADD, registers[REG_E]);
        registers[REG_A] += registers[REG_E];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x84:
        std::cout << "ADD H\n";
        set_flag(REG_A, A, ADD, registers[REG_H]);
        set_flag(REG_A, C, ADD, registers[REG_H]);
        registers[REG_A] += registers[REG_H];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x85:
        std::cout << "ADD L\n";
        set_flag(REG_A, A, ADD, registers[REG_L]);
        set_flag(REG_A, C, ADD, registers[REG_L]);
        registers[REG_A] += registers[REG_L];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x86:
        std::cout << "ADD M\n";
        set_flag(REG_A, A, ADD, memory[(registers[REG_H] << 8) | registers[REG_L]]);
        set_flag(REG_A, C, ADD, memory[(registers[REG_H] << 8) | registers[REG_L]]);
        registers[REG_A] += memory[(registers[REG_H] << 8) | registers[REG_L]];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x87:
        std::cout << "ADD A\n";
        set_flag(REG_A, A, ADD, registers[REG_A]);
        set_flag(REG_A, C, ADD, registers[REG_A]);
        registers[REG_A] += registers[REG_A];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x88:
    {
        std::cout << "ADC B\n";
        uint8_t carryValue = (conditionBits & 0x01);
        set_flag(REG_A, A, ADC, (registers[REG_B] + carryValue));
        set_flag(REG_A, C, ADC, (registers[REG_B] + carryValue));
        registers[REG_A] += (registers[REG_B] + carryValue);
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    }
    case 0x89:
    {
        std::cout << "ADC C\n";
        uint8_t carryValue = (conditionBits & 0x01);
        set_flag(REG_A, A, ADC, (registers[REG_C] + carryValue));
        set_flag(REG_A, C, ADC, (registers[REG_C] + carryValue));
        registers[REG_A] += (registers[REG_C] + carryValue);
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    }
    case 0x8A:
    {
        std::cout << "ADC D\n";
        uint8_t carryValue = (conditionBits & 0x01);
        set_flag(REG_A, A, ADC, (registers[REG_D] + carryValue));
        set_flag(REG_A, C, ADC, (registers[REG_D] + carryValue));
        registers[REG_A] += (registers[REG_D] + carryValue);
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    }
    case 0x8B:
    {
        std::cout << "ADC E\n";
        uint8_t carryValue = (conditionBits & 0x01);
        set_flag(REG_A, A, ADC, (registers[REG_E] + carryValue));
        set_flag(REG_A, C, ADC, (registers[REG_E] + carryValue));
        registers[REG_A] += (registers[REG_E] + carryValue);
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    }
    case 0x8C:
    {
        std::cout << "ADC H\n";
        uint8_t carryValue = (conditionBits & 0x01);
        set_flag(REG_A, A, ADC, (registers[REG_H] + carryValue));
        set_flag(REG_A, C, ADC, (registers[REG_H] + carryValue));
        registers[REG_A] += (registers[REG_H] + carryValue);
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    }
    case 0x8D:
    {
        std::cout << "ADC L\n";
        uint8_t carryValue = (conditionBits & 0x01);
        set_flag(REG_A, A, ADC, (registers[REG_L] + carryValue));
        set_flag(REG_A, C, ADC, (registers[REG_L] + carryValue));
        registers[REG_A] += (registers[REG_L] + carryValue);
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    }
    case 0x8E:
    {
        std::cout << "ADC M\n";
        uint8_t carryValue = (conditionBits & 0x01);
        set_flag(REG_A, A, ADC, (memory[(registers[REG_H] << 8) | registers[REG_L]] + carryValue));
        set_flag(REG_A, C, ADC, (memory[(registers[REG_H] << 8) | registers[REG_L]] + carryValue));
        registers[REG_A] += (memory[(registers[REG_H] << 8) | registers[REG_L]] + carryValue);
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    }
    case 0x8F:
    {
        std::cout << "ADC A\n";
        uint8_t carryValue = (conditionBits & 0x01);
        set_flag(REG_A, A, ADC, (registers[REG_A] + carryValue));
        set_flag(REG_A, C, ADC, (registers[REG_A] + carryValue));
        registers[REG_A] += (registers[REG_A] + carryValue);
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    }
    case 0x90:
        std::cout << "SUB B\n";
        set_flag(REG_A, A, SUB, registers[REG_B]);
        set_flag(REG_A, C, SUB, registers[REG_B]);
        registers[REG_A] -= registers[REG_B];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x91:
        std::cout << "SUB C\n";
        set_flag(REG_A, A, SUB, registers[REG_C]);
        set_flag(REG_A, C, SUB, registers[REG_C]);
        registers[REG_A] -= registers[REG_C];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x92:
        std::cout << "SUB D\n";
        set_flag(REG_A, A, SUB, registers[REG_D]);
        set_flag(REG_A, C, SUB, registers[REG_D]);
        registers[REG_A] -= registers[REG_D];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x93:
        std::cout << "SUB E\n";
        set_flag(REG_A, A, SUB, registers[REG_E]);
        set_flag(REG_A, C, SUB, registers[REG_E]);
        registers[REG_A] -= registers[REG_E];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x94:
        std::cout << "SUB H\n";
        set_flag(REG_A, A, SUB, registers[REG_H]);
        set_flag(REG_A, C, SUB, registers[REG_H]);
        registers[REG_A] -= registers[REG_H];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x95:
        std::cout << "SUB L\n";
        set_flag(REG_A, A, SUB, registers[REG_L]);
        set_flag(REG_A, C, SUB, registers[REG_L]);
        registers[REG_A] -= registers[REG_L];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x96:
        std::cout << "SUB M\n";
        set_flag(REG_A, A, SUB, memory[(registers[REG_H] << 8) | registers[REG_L]]);
        set_flag(REG_A, C, SUB, memory[(registers[REG_H] << 8) | registers[REG_L]]);
        registers[REG_A] -= memory[(registers[REG_H] << 8) | registers[REG_L]];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x97:
        std::cout << "SUB A\n";
        set_flag(REG_A, A, SUB, registers[REG_A]);
        set_flag(REG_A, C, SUB, registers[REG_A]);
        registers[REG_A] -= registers[REG_A];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0x98:
        {
            std::cout << "SBB B\n";
            uint8_t carryValue = (conditionBits & 0x01);
            set_flag(REG_A, A, SUB, (registers[REG_B] + carryValue));
            set_flag(REG_A, C, SUB, (registers[REG_B] + carryValue));
            registers[REG_A] -= (registers[REG_B] + carryValue);
            set_flag(REG_A, S);
            set_flag(REG_A, Z);
            set_flag(REG_A, P);
            pc += 1;
            break;
        }
    case 0x99:
        {
            std::cout << "SBB C\n";
            uint8_t carryValue = (conditionBits & 0x01);
            set_flag(REG_A, A, SUB, (registers[REG_C] + carryValue));
            set_flag(REG_A, C, SUB, (registers[REG_C] + carryValue));
            registers[REG_A] -= (registers[REG_C] + carryValue);
            set_flag(REG_A, S);
            set_flag(REG_A, Z);
            set_flag(REG_A, P);
            pc += 1;
            break;
        }
    case 0x9A:
        {
            std::cout << "SBB D\n";
            uint8_t carryValue = (conditionBits & 0x01);
            set_flag(REG_A, A, SUB, (registers[REG_D] + carryValue));
            set_flag(REG_A, C, SUB, (registers[REG_D] + carryValue));
            registers[REG_A] -= (registers[REG_D] + carryValue);
            set_flag(REG_A, S);
            set_flag(REG_A, Z);
            set_flag(REG_A, P);
            pc += 1;
            break;
        }
    case 0x9B:
        {
            std::cout << "SBB E\n";
            uint8_t carryValue = (conditionBits & 0x01);
            set_flag(REG_A, A, SUB, (registers[REG_E] + carryValue));
            set_flag(REG_A, C, SUB, (registers[REG_E] + carryValue));
            registers[REG_A] -= (registers[REG_E] + carryValue);
            set_flag(REG_A, S);
            set_flag(REG_A, Z);
            set_flag(REG_A, P);
            pc += 1;
            break;
        }
    case 0x9C:
        {
            std::cout << "SBB H\n";
            uint8_t carryValue = (conditionBits & 0x01);
            set_flag(REG_A, A, SUB, (registers[REG_H] + carryValue));
            set_flag(REG_A, C, SUB, (registers[REG_H] + carryValue));
            registers[REG_A] -= (registers[REG_H] + carryValue);
            set_flag(REG_A, S);
            set_flag(REG_A, Z);
            set_flag(REG_A, P);
            pc += 1;
            break;
        }
    case 0x9D:
        {
            std::cout << "SBB L\n";
            uint8_t carryValue = (conditionBits & 0x01);
            set_flag(REG_A, A, SUB, (registers[REG_L] + carryValue));
            set_flag(REG_A, C, SUB, (registers[REG_L] + carryValue));
            registers[REG_A] -= (registers[REG_L] + carryValue);
            set_flag(REG_A, S);
            set_flag(REG_A, Z);
            set_flag(REG_A, P);
            pc += 1;
            break;
        }
    case 0x9E:
        {
            std::cout << "SBB M\n";
            uint8_t carryValue = (conditionBits & 0x01);
            set_flag(REG_A, A, SUB, (memory[(registers[REG_H] << 8) | registers[REG_L]] + carryValue));
            set_flag(REG_A, C, SUB, (memory[(registers[REG_H] << 8) | registers[REG_L]] + carryValue));
            registers[REG_A] -= (memory[(registers[REG_H] << 8) | registers[REG_L]] + carryValue);
            set_flag(REG_A, S);
            set_flag(REG_A, Z);
            set_flag(REG_A, P);
            pc += 1;
            break;
        }
    case 0x9F:
        {
            std::cout << "SBB A\n";
            uint8_t carryValue = (conditionBits & 0x01);
            set_flag(REG_A, A, SUB, (registers[REG_A] + carryValue));
            set_flag(REG_A, C, SUB, (registers[REG_A] + carryValue));
            registers[REG_A] -= (registers[REG_A] + carryValue);
            set_flag(REG_A, S);
            set_flag(REG_A, Z);
            set_flag(REG_A, P);
            pc += 1;
            break;
        }
    case 0xA0:
        std::cout << "ANA B\n";
        conditionBits &= 0xFE;
        set_flag(REG_A, A, ANA, registers[REG_B]);
        registers[REG_A] &= registers[REG_B];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xA1:
        std::cout << "ANA C\n";
        conditionBits &= 0xFE;
        set_flag(REG_A, A, ANA, registers[REG_C]);
        registers[REG_A] &= registers[REG_C];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xA2:
        std::cout << "ANA D\n";
        conditionBits &= 0xFE;
        set_flag(REG_A, A, ANA, registers[REG_D]);
        registers[REG_A] &= registers[REG_D];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xA3:
        std::cout << "ANA E\n";
        conditionBits &= 0xFE;
        set_flag(REG_A, A, ANA, registers[REG_E]);
        registers[REG_A] &= registers[REG_E];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xA4:
        std::cout << "ANA H\n";
        conditionBits &= 0xFE;
        set_flag(REG_A, A, ANA, registers[REG_H]);
        registers[REG_A] &= registers[REG_H];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xA5:
        std::cout << "ANA L\n";
        conditionBits &= 0xFE;
        set_flag(REG_A, A, ANA, registers[REG_L]);
        registers[REG_A] &= registers[REG_L];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xA6:
        std::cout << "ANA M\n";
        conditionBits &= 0xFE;
        set_flag(REG_A, A, ANA, (memory[(registers[REG_H] << 8) | registers[REG_L]]));
        registers[REG_A] &= memory[(registers[REG_H] << 8) | registers[REG_L]];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xA7:
        std::cout << "ANA A\n";
        conditionBits &= 0xFE;
        set_flag(REG_A, A, ANA, registers[REG_A]);
        registers[REG_A] &= registers[REG_A];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xA8:
        std::cout << "XRA B\n";
        conditionBits &= 0xEE;
        registers[REG_A] ^= registers[REG_B];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xA9:
        std::cout << "XRA C\n";
        conditionBits &= 0xEE;
        registers[REG_A] ^= registers[REG_C];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xAA:
        std::cout << "XRA D\n";
        conditionBits &= 0xEE;
        registers[REG_A] ^= registers[REG_D];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xAB:
        std::cout << "XRA E\n";
        conditionBits &= 0xEE;
        registers[REG_A] ^= registers[REG_E];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xAC:
        std::cout << "XRA H\n";
        conditionBits &= 0xEE;
        registers[REG_A] ^= registers[REG_H];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xAD:
        std::cout << "XRA L\n";
        conditionBits &= 0xEE;
        registers[REG_A] ^= registers[REG_L];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xAE:
        std::cout << "XRA M\n";
        conditionBits &= 0xEE;
        registers[REG_A] ^= memory[(registers[REG_H] << 8) | registers[REG_L]];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xAF:
        std::cout << "XRA A\n";
        conditionBits &= 0xEE;
        registers[REG_A] ^= registers[REG_A];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xB0:
        std::cout << "ORA B\n";
        conditionBits &= 0xEE;
        registers[REG_A] |= registers[REG_B];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xB1:
        std::cout << "ORA C\n";
        conditionBits &= 0xEE;
        registers[REG_A] |= registers[REG_C];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xB2:
        std::cout << "ORA D\n";
        conditionBits &= 0xEE;
        registers[REG_A] |= registers[REG_D];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xB3:
        std::cout << "ORA E\n";
        conditionBits &= 0xEE;
        registers[REG_A] |= registers[REG_E];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xB4:
        std::cout << "ORA H\n";
        conditionBits &= 0xEE;
        registers[REG_A] |= registers[REG_H];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xB5:
        std::cout << "ORA L\n";
        conditionBits &= 0xEE;
        registers[REG_A] |= registers[REG_L];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xB6:
        std::cout << "ORA M\n";
        conditionBits &= 0xEE;
        registers[REG_A] |= memory[(registers[REG_H] << 8) | registers[REG_L]];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xB7:
        std::cout << "ORA A\n";
        conditionBits &= 0xEE;
        registers[REG_A] |= registers[REG_A];
        set_flag(REG_A, S);
        set_flag(REG_A, Z);
        set_flag(REG_A, P);
        pc += 1;
        break;
    case 0xB8:
        std::cout << "CMP B\n";
        set_flag(REG_A, A, CMP, registers[REG_B]);
        set_flag(REG_A, C, CMP, registers[REG_B]);
        set_flag(REG_A, S, CMP, registers[REG_B]);
        set_flag(REG_A, Z, CMP, registers[REG_B]);
        set_flag(REG_A, P, CMP, registers[REG_B]);
        pc += 1;
        break;
    case 0xB9:
        std::cout << "CMP C\n";
        pc += 1;
        break;
    case 0xBA:
        std::cout << "CMP D\n";
        pc += 1;
        break;
    case 0xBB:
        std::cout << "CMP E\n";
        pc += 1;
        break;
    case 0xBC:
        std::cout << "CMP H\n";
        pc += 1;
        break;
    case 0xBD:
        std::cout << "CMP L\n";
        pc += 1;
        break;
    case 0xBE:
        std::cout << "CMP M\n";
        pc += 1;
        break;
    case 0xBF:
        std::cout << "CMP A\n";
        pc += 1;
        break;
    case 0xC0:
        std::cout << "RNZ\n";
        pc += 1;
        break;
    case 0xC1:
        std::cout << "POP B\n";
        pc += 1;
        break;
    case 0xC2:
        print_ins_hex("JNZ #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xC3:
    case 0xCB:
        print_ins_hex("JMP #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xC4:
        print_ins_hex("CNZ #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xC5:
        std::cout << "PUSH B\n";
        pc += 1;
        break;
    case 0xC6:
        print_ins_hex("ADI #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0xC7:
        std::cout << "RST 0\n";
        pc += 1;
        break;
    case 0xC8:
        std::cout << "RZ\n";
        pc += 1;
        break;
    case 0xC9:
    case 0xD9:
        std::cout << "RET\n";
        pc += 1;
        break;
    case 0xCA:
        print_ins_hex("JZ #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xCC:
        print_ins_hex("CZ #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xCD:
    case 0xDD:
    case 0xED:
    case 0xFD:
        print_ins_hex("CALL #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xCE:
        print_ins_hex("ACI #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0xCF:
        std::cout << "RST 1\n";
        pc += 1;
        break;
    case 0xD0:
        std::cout << "RNC\n";
        pc += 1;
        break;
    case 0xD1:
        std::cout << "POP D\n";
        pc += 1;
        break;
    case 0xD2:
        print_ins_hex("JNC #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xD3:
        print_ins_hex("OUT #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0xD4:
        print_ins_hex("CNC #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xD5:
        std::cout << "PUSH D\n";
        pc += 1;
        break;
    case 0xD6:
        print_ins_hex("SUI #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0xD7:
        std::cout << "RST 2\n";
        pc += 1;
        break;
    case 0xD8:
        std::cout << "RC\n";
        pc += 1;
        break;
    case 0xDA:
        print_ins_hex("JC #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xDB:
        print_ins_hex("IN #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0xDC:
        print_ins_hex("CC #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xDE:
        print_ins_hex("SBI #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0xDF:
        std::cout << "RST 3\n";
        pc += 1;
        break;
    case 0xE0:
        std::cout << "RPO\n";
        pc += 1;
        break;
    case 0xE1:
        std::cout << "POP H\n";
        pc += 1;
        break;
    case 0xE2:
        print_ins_hex("JPO #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xE3:
        std::cout << "XTHL\n";
        pc += 1;
        break;
    case 0xE4:
        print_ins_hex("CPO #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xE5:
        std::cout << "PUSH H\n";
        pc += 1;
        break;
    case 0xE6:
        print_ins_hex("ANI #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0xE7:
        std::cout << "RST 4\n";
        pc += 1;
        break;
    case 0xE8:
        std::cout << "RPE\n";
        pc += 1;
        break;
    case 0xE9:
        std::cout << "PCHL\n";
        pc += 1;
        break;
    case 0xEA:
        print_ins_hex("JPE #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xEB:
        std::cout << "XCHG\n";
        pc += 1;
        break;
    case 0xEC:
        print_ins_hex("CPE #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xEE:
        print_ins_hex("XRI #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0xEF:
        std::cout << "RST 5\n";
        pc += 1;
        break;
    case 0xF0:
        std::cout << "RP\n";
        pc += 1;
        break;
    case 0xF1:
        std::cout << "POP PSW\n";
        pc += 1;
        break;
    case 0xF2:
        print_ins_hex("JP #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xF3:
        std::cout << "DI\n";
        pc += 1;
        break;
    case 0xF4:
        print_ins_hex("CP #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xF5:
        std::cout << "PUSH PSW\n";
        pc += 1;
        break;
    case 0xF6:
        print_ins_hex("ORI #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0xF7:
        std::cout << "RST 6\n";
        pc += 1;
        break;
    case 0xF8:
        std::cout << "RM\n";
        pc += 1;
        break;
    case 0xF9:
        std::cout << "SPHL\n";
        pc += 1;
        break;
    case 0xFA:
        print_ins_hex("JM #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xFB:
        std::cout << "EI\n";
        pc += 1;
        break;
    case 0xFC:
        print_ins_hex("CM #$", memory[pc+2], memory[pc+1], true);
        pc += 3;
        break;
    case 0xFE:
        print_ins_hex("CPI #$", memory[pc+1], 0, false);
        pc += 2;
        break;
    case 0xFF:
        std::cout << "RST 7\n";
        break;
    default:
        std::cout << "UNKNOWN OPCODE: " << static_cast<int32_t>(opcode) << std::endl;
        break;
    }
}

bool CPU::load_rom(const char* path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if(!file.is_open())
    {
        std::cout << "Failed to open ROM file" << std::endl;
        return false;
    }
    std::streampos size = file.tellg();
    if(size > (MEMORY_SIZE-1))
    {
        std::cout << "ROM file is too large!" << std::endl;
    }

    file.seekg(0, std::ios::beg);
    if(!file.read(reinterpret_cast<char*>(&memory[0]), size))
    {
        std::cout << "Failed to read ROM file" << std::endl;
        return false;
    }

    return true;
}

CPU::~CPU()
{

}
