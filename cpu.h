#pragma once

#include<stdbool.h>
#include<stdint.h>

uint8_t MEMORY[0x10000]; //  64KiB Memory
uint8_t STATUS = 0;
uint16_t PC = 0x0000; // TODO start from reset vector
uint8_t ACCUM = 0;
uint8_t XREG = 0;
uint8_t YREG = 0;
uint16_t STACKPTR = 0x1FF;

// 6502 Function Macros
// Carry Flag (Bit 0)
#define isCarry() ((STATUS & 0x01) != 0)
#define setCarryFlag(value) (STATUS = (STATUS & 0xFE) | ((value) & 0x01))

// Zero Flag (Bit 1)
#define isZero() ((STATUS & 0x02) != 0)
#define setZeroFlag(value) (STATUS = (STATUS & 0xFD) | (((value) & 0x01) << 1))

// Interrupt Flag (Bit 2)
#define isInterrupt() ((STATUS & 0x04) != 0)
#define setInterruptFlag(value) (STATUS = (STATUS & 0xFB) | (((value) & 0x01) << 2))

// Decimal Flag (Bit 3)
#define isDecimal() ((STATUS & 0x08) != 0)
#define setDecimalFlag(value) (STATUS = (STATUS & 0xF7) | (((value) & 0x01) << 3))

// Break Flag (Bit 4)
#define isBreak() ((STATUS & 0x10) != 0)
#define setBreakFlag(value) (STATUS = (STATUS & 0xEF) | (((value) & 0x01) << 4))

// Overflow Flag (Bit 6)
#define isOverflow() ((STATUS & 0x40) != 0)
#define setOverflowFlag(value) (STATUS = (STATUS & 0xBF) | (((value) & 0x01) << 6))

// Negative Flag (Bit 7)
#define isNegative() ((STATUS & 0x80) != 0)
#define setNegativeFlag(value) (STATUS = (STATUS & 0x7F) | (((value) & 0x01) << 7))

// Program Counter
#define incPC(value) (PC = PC + (value))
#define setPC(address) (PC = address)

// Registers
#define incX() (XREG++)
#define decX() (XREG--)
#define incY() (YREG++)
#define decY() (YREG--)

typedef enum _AddressingMode {
    AddressingImplied, // no operand
    AddressingAccumulator, // operand is accumulator (implied)
    AddressingImmediate, // operand is single byte actual value
    AddressingZeroPage, // operand is a zero page address (high byte of addr is 0x00)
    AddressingZeroPageX, // operand is a zpg address, effective address is addr incremented by X (no carry)
    AddressingZeroPageY, // operand is a zpg address, effective address is addr incremented by Y (no carry)
    AddressingAbsolute, // operand is a complete (16-bit) address
    AddressingAbsoluteX, // operand is address, effective addr is addr incremented by X (with carry)
    AddressingAbsoluteY, // operand is address, effective addr is addr incremented by Y (with carry)
    AddressingIndirect, // operand is address, effective addr is the *contents* of addr. (ie, operand is a pointer)
    AddressingXIndirect, // operand is zpg address, effective addr is the 16bit word in (addr+X, addr+X+1) (no carry)
    AddressingIndirectY, // operand is zpg address, effective addr is the 16bit word in (addr, addr+1) + Y (w carry)
    AddressingRelative // operand (branch target) is PC + signed offset
} AddressingMode;

typedef struct _Instruction {
    uint8_t opcode;
    char mnemonic[4];
    AddressingMode mode;
    int timing;
    bool timingAddOne;
} Instruction;

/**
 * Array of instructions (struct)
 */
Instruction Instructions[] = {
    {0x69, "ADC", AddressingImmediate, 2, false},
    {0x65, "ADC", AddressingZeroPage, 3, false},
    {0x75, "ADC", AddressingZeroPageX, 4, false},
    {0x6D, "ADC", AddressingAbsolute, 4, false},
    {0x7D, "ADC", AddressingAbsoluteX, 4, true},
    {0x79, "ADC", AddressingAbsoluteY, 4, true},
    {0x61, "ADC", AddressingXIndirect, 6, false},
    {0x71, "ADC", AddressingIndirectY, 5, true},
    {0x29, "AND", AddressingImmediate, 2, false},
    {0x25, "AND", AddressingZeroPage, 3, false},
    {0x35, "AND", AddressingZeroPageX, 4, false},
    {0x2D, "AND", AddressingAbsolute, 4, false},
    {0x3D, "AND", AddressingAbsoluteX, 4, true},
    {0x39, "AND", AddressingAbsoluteY, 4, true},
    {0x21, "AND", AddressingXIndirect, 6, false},
    {0x31, "AND", AddressingIndirectY, 5, true},
    {0x0A, "ASL", AddressingAccumulator, 2, false},
    {0x06, "ASL", AddressingZeroPage, 5, false},
    {0x16, "ASL", AddressingZeroPageX, 6, false},
    {0x0E, "ASL", AddressingAbsolute, 6, false},
    {0x1E, "ASL", AddressingAbsoluteX, 7, false},
    {0x90, "BCC", AddressingRelative, 0, true},
    {0xB0, "BCS", AddressingRelative, 0, true},
    {0xF0, "BEQ", AddressingRelative, 0, true},
    {0x24, "BIT", AddressingZeroPage, 3, false},
    {0x2C, "BIT", AddressingAbsolute, 4, false},
    {0x30, "BMI", AddressingRelative, 0, true},
    {0xD0, "BNE", AddressingRelative, 0, true},
    {0x10, "BPL", AddressingRelative, 0, true},
    {0x00, "BRK", AddressingImplied, 7, false},
    {0x50, "BVC", AddressingRelative, 0, true},
    {0x70, "BVS", AddressingRelative, 0, true},
    {0x18, "CLC", AddressingImplied, 2, false},
    {0xD8, "CLD", AddressingImplied, 2, false},
    {0x58, "CLI", AddressingImplied, 2, false},
    {0xB8, "CLV", AddressingImplied, 2, false},
    {0xC9, "CMP", AddressingImmediate, 2, false},
    {0xC5, "CMP", AddressingZeroPage, 3, false},
    {0xD5, "CMP", AddressingZeroPageX, 4, false},
    {0xCD, "CMP", AddressingAbsolute, 4, false},
    {0xDD, "CMP", AddressingAbsoluteX, 4, true},
    {0xD9, "CMP", AddressingAbsoluteY, 4, true},
    {0xC1, "CMP", AddressingXIndirect, 6, false},
    {0xD1, "CMP", AddressingIndirectY, 5, true},
    {0xE0, "CPX", AddressingImmediate, 2, false},
    {0xE4, "CPX", AddressingZeroPage, 3, false},
    {0xEC, "CPX", AddressingAbsolute, 4, false},
    {0xC0, "CPY", AddressingImmediate, 3, false},
    {0xC4, "CPY", AddressingZeroPage, 3, false},
    {0xCC, "CPY", AddressingAbsolute, 4, false},
    {0xC6, "DEC", AddressingZeroPage, 5, false},
    {0xD6, "DEC", AddressingZeroPageX, 6, false},
    {0xCE, "DEC", AddressingAbsolute, 6, false},
    {0xDE, "DEC", AddressingAbsoluteX, 7, false},
    {0xCA, "DEX", AddressingImplied, 2, false},
    {0x88, "DEY", AddressingImplied, 2, false},
    {0x49, "EOR", AddressingImmediate, 2, false},
    {0x45, "EOR", AddressingZeroPage, 3, false},
    {0x55, "EOR", AddressingZeroPageX, 4, false},
    {0x4D, "EOR", AddressingAbsolute, 4, false},
    {0x5D, "EOR", AddressingAbsoluteX, 4, true},
    {0x59, "EOR", AddressingAbsoluteY, 4, true},
    {0x41, "EOR", AddressingXIndirect, 6, false},
    {0x51, "EOR", AddressingIndirectY, 5, true},
    {0xE6, "INC", AddressingZeroPage, 5, false},
    {0xF6, "INC", AddressingZeroPageX, 6, false},
    {0xEE, "INC", AddressingAbsolute, 6, false},
    {0xFE, "INC", AddressingAbsoluteX, 7, false},
    {0xE8, "INX", AddressingImplied, 2, false}, // Y
    {0xC8, "INY", AddressingImplied, 2, false}, // Y
    {0x4C, "JMP", AddressingAbsolute, 3, false}, // Y
    {0x6C, "JMP", AddressingIndirect, 5, false},
    {0x20, "JSR", AddressingAbsolute, 6, false},
    {0xA9, "LDA", AddressingImmediate, 2, false},
    {0xA5, "LDA", AddressingZeroPage, 3, false},
    {0xB5, "LDA", AddressingZeroPageX, 4, false},
    {0xAD, "LDA", AddressingAbsolute, 4, false},
    {0xBD, "LDA", AddressingAbsoluteX, 4, true},
    {0xB9, "LDA", AddressingAbsoluteY, 4, true},
    {0xA1, "LDA", AddressingXIndirect, 6, false},
    {0xB1, "LDA", AddressingIndirectY, 5, true},
    {0xA2, "LDX", AddressingImmediate, 2, false},
    {0xA6, "LDX", AddressingZeroPage, 3, false},
    {0xB6, "LDX", AddressingZeroPageY, 4, false},
    {0xAE, "LDX", AddressingAbsolute, 4, false},
    {0xBE, "LDX", AddressingAbsoluteY, 4, true},
    {0xA0, "LDY", AddressingImmediate, 2, false},
    {0xA4, "LDY", AddressingZeroPage, 3, false},
    {0xB4, "LDY", AddressingZeroPageX, 4, false},
    {0xAC, "LDY", AddressingAbsolute, 4, false},
    {0xBC, "LDY", AddressingAbsoluteX, 4, true},
    {0x4A, "LSR", AddressingAccumulator, 2, false},
    {0x46, "LSR", AddressingZeroPage, 5, false},
    {0x56, "LSR", AddressingZeroPageX, 6, false},
    {0x4E, "LSR", AddressingAbsolute, 6, false},
    {0x5E, "LSR", AddressingAbsoluteX, 7, false},
    {0xEA, "NOP", AddressingImplied, 2, false},
    {0x09, "ORA", AddressingImmediate, 2, false},
    {0x05, "ORA", AddressingZeroPage, 3, false},
    {0x15, "ORA", AddressingZeroPageX, 4, false},
    {0x0D, "ORA", AddressingAbsolute, 4, false},
    {0x1D, "ORA", AddressingAbsoluteX, 4, true},
    {0x19, "ORA", AddressingAbsoluteY, 4, true},
    {0x01, "ORA", AddressingXIndirect, 6, false},
    {0x11, "ORA", AddressingIndirectY, 5, true},
    {0x48, "PHA", AddressingImplied, 3, false},
    {0x08, "PHP", AddressingImplied, 3, false},
    {0x68, "PLA", AddressingImplied, 4, false},
    {0x28, "PLP", AddressingImplied, 4, false},
    {0x2A, "ROL", AddressingAccumulator, 2, false},
    {0x26, "ROL", AddressingZeroPage, 5, false},
    {0x36, "ROL", AddressingZeroPageX, 6, false},
    {0x2E, "ROL", AddressingAbsolute, 6, false},
    {0x3E, "ROL", AddressingAbsoluteX, 7, false},
    {0x6A, "ROR", AddressingAccumulator, 2, false},
    {0x66, "ROR", AddressingZeroPage, 5, false},
    {0x76, "ROR", AddressingZeroPageX, 6, false},
    {0x6E, "ROR", AddressingAbsolute, 6, false},
    {0x7E, "ROR", AddressingAbsoluteX, 7, false},
    {0x40, "RTI", AddressingImplied, 6, false},
    {0x60, "RTS", AddressingImplied, 6, false},
    {0xE9, "SBC", AddressingImmediate, 2, false},
    {0xE5, "SBC", AddressingZeroPage, 3, false},
    {0xF5, "SBC", AddressingZeroPageX, 4, false},
    {0xED, "SBC", AddressingAbsolute, 4, false},
    {0xFD, "SBC", AddressingAbsoluteX, 4, true},
    {0xF9, "SBC", AddressingAbsoluteY, 4, true},
    {0xE1, "SBC", AddressingXIndirect, 6, false},
    {0xF1, "SBC", AddressingIndirectY, 5, true},
    {0x38, "SEC", AddressingImplied, 2, false},
    {0xF8, "SED", AddressingImplied, 2, false},
    {0x78, "SEI", AddressingImplied, 2, false},
    {0x85, "STA", AddressingZeroPage, 3, false},
    {0x95, "STA", AddressingZeroPageX, 4, false},
    {0x8D, "STA", AddressingAbsolute, 4, false},
    {0x9D, "STA", AddressingAbsoluteX, 5, false},
    {0x99, "STA", AddressingAbsoluteY, 5, false},
    {0x81, "STA", AddressingXIndirect, 6, false},
    {0x91, "STA", AddressingIndirectY, 6, false},
    {0x86, "STX", AddressingZeroPage, 3, false},
    {0x96, "STX", AddressingZeroPageY, 4, false},
    {0x8E, "STX", AddressingAbsolute, 4, false},
    {0x84, "STY", AddressingZeroPage, 3, false},
    {0x94, "STY", AddressingZeroPageX, 4, false},
    {0x8C, "STY", AddressingAbsolute, 4, false},
    {0xAA, "TAX", AddressingImplied, 2, false},
    {0xA8, "TAY", AddressingImplied, 2, false},
    {0xBA, "TSX", AddressingImplied, 2, false},
    {0x8A, "TXA", AddressingImplied, 2, false},
    {0x9A, "TXS", AddressingImplied, 2, false},
    {0x98, "TYA", AddressingImplied, 2, false}
};
static Instruction *OpcodeTable[256];

struct _ExecutionContext;

typedef struct _ExecutionContext {
    // CPU state
    uint8_t A; // Accumulator
    uint8_t X; // X register
    uint8_t Y; // Y register
    uint8_t P; // Processor status
    uint16_t SP; // Stack pointer
    uint16_t PC; // Program counter
    uint16_t pendingTiming; // ticks to run down between each instruction
    Instruction *instruction;
} ExecutionContext;

typedef enum _ExecutionResult {
    RUNNING,
    HALTING,
    INVALID
} ExecutionResult;

// Function prototypes
// TODO change method names to cpu_
// TODO change method names to snake_case
void checkStatus(uint8_t value);
ExecutionResult executeNext(ExecutionContext *context);
Instruction *fetchInstruction(const uint8_t opcode);
uint8_t fetchOpcode(const uint16_t address);
uint16_t getInstructionLength(const Instruction *instruction);
void printInstruction(const uint8_t opcode);
void printStatus();
void reset(ExecutionContext *context);
ExecutionResult tick(ExecutionContext *context);

uint16_t cpu_getJumpAddress(AddressingMode mode);
void cpu_init(ExecutionContext *context);
void cpu_poke(uint16_t address, uint8_t value);
void cpu_exec_adc(ExecutionContext *context, uint8_t operand);
void cpu_exec_adc_bcd(ExecutionContext *context, uint8_t operand);

// Inline functions
inline uint8_t fetchOpcode(const uint16_t address) {
    return MEMORY[address];
};

inline Instruction *fetchInstruction(const uint8_t opcode) {
    return &Instructions[opcode];
}

// debugging
inline void printInstruction(const uint8_t opcode) {
    Instruction *instruction = fetchInstruction(opcode);
    if (!instruction) return;
    printf("$%04X - A=%02X X=%02X Y=%02X PS=%02X SP=%02X %02X %s $XXXX\n",
           PC, ACCUM, XREG, YREG, STATUS, STACKPTR,
           instruction->opcode, instruction->mnemonic);
}

inline void printStatus() {
    printf("PC=%04X A=%02X X=%02X Y=%02X C=%d Z=%d I=%d D=%d O=%d N=%d\n",
           PC, ACCUM, XREG, YREG, isCarry(), isZero(), isInterrupt(), isDecimal(), isOverflow(), isNegative());
}
