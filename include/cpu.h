#ifndef CPU_H
#define CPU_H

#include <iosfwd>
#include <stdint.h>

// There are pair registers like BC, DE and HL
// The first one hold 8 most significant bits and the other holds 8 least significant bits
// So 1F2A in registers HL would be H = 0x1F and L = 0x2A

// 8-bit registers
#define REG_A 0
#define REG_B 1
#define REG_C 2
#define REG_D 3
#define REG_E 4
#define REG_H 5
#define REG_L 6
#define REF_MEMORY 7    // "register" M

#define MEMORY_SIZE 65536

enum FlagBit
{
    S,
    Z,
    A,
    P,
    C
};

// These are instructions that affect condition bits
enum ConditionIns
{
    NONE,
    INR,
    DCR,
    DAA,
    ADD,
    ADC,
    SUB,
};

// Intel 8080
class CPU
{
private:
    uint8_t memory[MEMORY_SIZE];
    uint8_t registers[7];

    /*
     * 00000010(byte)    => DEFAULT BITS (0x02)
     * sz-a-p-c
     * s = Sign flag
     * z = Zero flag. Set if result was zero, reset otherwise
     * - => Not used, always zero
     * a => Auxiliary Carry flag. Set to carry out of bit 3 in result
     * - => Not used, always zero
     * p => Parity flag. Set if the number of bits in result is even and reset if it's odd
     * - => Not used, always one
     * c => Carry flag. Set to carry out of bit 7 in result
    */
    uint8_t conditionBits;

    uint16_t pc;
    uint16_t sp;

    // If instruction has length of 2 bytes only highByte will be used
    void print_ins_hex(const char* ins, uint8_t highByte, uint8_t lowByte, bool hasLowByte);
    void set_flag(uint8_t reg, enum FlagBit flagBit, enum ConditionIns op = NONE, uint8_t number = 0, uint16_t addr = 0);

public:
    CPU();
    void cycle();
    bool load_rom(const char* path);
    void disassembler();
    ~CPU();
};

#endif // CPU_H
