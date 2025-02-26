#pragma once

#include "cpu.h"
#include<string.h>

// Reference:   http://www.6502.org/tutorials/6502opcodes.html

void initCpu(ExecutionContext *context) {
    // populate opcode table
    // but first, clear it out
    memset(&OpcodeTable[0], 0, sizeof(OpcodeTable));
    const int count = sizeof(Instructions) / sizeof(Instructions[0]);
    for (int i = 0; i < count; i++) {
        Instruction *instruction = &Instructions[i];
        OpcodeTable[instruction->opcode] = instruction;
    }

    // clear memory bank to default value
    for (int i = 0; i < 0xFFFF; i++) {
        MEMORY[i] = 0xEA; // NOP
        // MEMORY[i] = 0xE8; // INX
    }
}

void cpu_poke(uint16_t address, uint8_t value) {
    MEMORY[address] = value;
}

// 6502 Functions
void checkStatus(const uint8_t value) {
    setZeroFlag(value == 0);
    setNegativeFlag(value >= 128);
}

void reset(ExecutionContext *context) {
    XREG = 0;
    YREG = 0;
    ACCUM = 0;
    context->A = ACCUM;
    context->X = XREG;
    context->Y = YREG;
}

uint16_t getInstructionLength(const Instruction *instruction) {
    switch (instruction->mode) {
        case AddressingImplied:
        case AddressingAccumulator:
            return 1;
        case AddressingImmediate:
        case AddressingZeroPage:
        case AddressingZeroPageX:
        case AddressingZeroPageY:
        case AddressingXIndirect:
        case AddressingIndirectY:
        case AddressingRelative:
            return 2;
        case AddressingAbsolute:
        case AddressingAbsoluteX:
        case AddressingAbsoluteY:
        case AddressingIndirect:
            return 3;
        default: return 1;
    }
}

uint16_t cpu_getJumpAddress(AddressingMode mode) {
    if (mode == AddressingAbsolute) {
        // Get next two bytes in little endian format.
        // We need to look ahead one extra byte because the actual instruction hasn't updated PC yet.
        uint8_t low = MEMORY[PC + 1 + 0];
        uint8_t high = MEMORY[PC + 1 + 1];
        uint16_t newAddress = high * 0x100 + low;
        return newAddress;
    }

    return PC;
}

ExecutionResult tick(ExecutionContext *context) {
    // Are there ticks running down?
    if (context->pendingTiming > 0) {
        context->pendingTiming--;
        return RUNNING;
    }

    // No more ticks.  Execute instruction.
    const ExecutionResult result = executeNext(context);

    return result;
}

ExecutionResult executeNext(ExecutionContext *context) {
    // fetch opcode
    const uint8_t opcode = MEMORY[PC];
    Instruction *instruction = OpcodeTable[opcode];
    if (!instruction) {
        // We could be processing DATA instead of an OPCODE.
        // For example, when the PC gets put into the text page area.
        // For now, just quietly ignore any opcode that isn't found.
        return INVALID;
    }

    // assign the instruction to the execution context
    context->instruction = instruction;

    // reset the execution context timing from instruction
    context->pendingTiming = instruction->timing;

    // All instructions will update the PC based on the length of the instruction,
    // except instructions that modify the PC directly. Flag those situations so
    // we can suppress the default behavior later.
    bool updatePC = true;

    // decode opcode and execute
    switch (opcode) {
        // ADC
        case 0x69:
        case 0x65:
        case 0x75:
        case 0x6D:
        case 0x7D:
        case 0x79:
        case 0x61:
        case 0x71: {

            break;
        }

        // JMP
        case 0x4C:
        case 0x6C: {
            updatePC = false;
            uint16_t newAddress = cpu_getJumpAddress(instruction->mode);
            // printf("%04X X=%02X      JMP %04X \n", PC, XREG, newAddress);
            setPC(newAddress);
            context->PC = newAddress;
            break;
        }

        // INX
        case 0xE8: {
            incX();
            context->X = XREG;
            // printf("%04X X=%02X      INX \n", PC, XREG);
            break;
        }

        // INY
        case 0xC8: {
            incY();
            context->Y = YREG;
            break;
        }

        // NOP
        case 0xEA: {
            // do nothing
            break;
        }
        default: {
            // opcode not found
            break;
        }
    }

    // printInstruction(instruction->opcode);

    // Update PC and timing
    if (updatePC) {
        incPC(getInstructionLength(instruction));
        context->PC = PC;
        // printStatus();
    }

    return RUNNING;
}
