/*
 *  Copyright (c) 2019, Stefan Johnson
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this list
 *     of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice, this
 *     list of conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "HubArchJIT.h"
#include "HubArchInstruction.h"


typedef enum {
    HKHubArchJITRegisterAL = 0,
    HKHubArchJITRegisterCL,
    HKHubArchJITRegisterDL,
    HKHubArchJITRegisterBL,
    HKHubArchJITRegisterAH,
    HKHubArchJITRegisterCH,
    HKHubArchJITRegisterDH,
    HKHubArchJITRegisterBH,
    
    HKHubArchJITRegisterAX = 0,
    HKHubArchJITRegisterCX,
    HKHubArchJITRegisterDX,
    HKHubArchJITRegisterBX,
    HKHubArchJITRegisterSP,
    HKHubArchJITRegisterBP,
    HKHubArchJITRegisterSI,
    HKHubArchJITRegisterDI,
    
    HKHubArchJITRegisterEAX = 0,
    HKHubArchJITRegisterECX,
    HKHubArchJITRegisterEDX,
    HKHubArchJITRegisterEBX,
    HKHubArchJITRegisterESP,
    HKHubArchJITRegisterEBP,
    HKHubArchJITRegisterESI,
    HKHubArchJITRegisterEDI,
    
    HKHubArchJITRegisterRAX = 0,
    HKHubArchJITRegisterRCX,
    HKHubArchJITRegisterRDX,
    HKHubArchJITRegisterRBX,
    HKHubArchJITRegisterRSP,
    HKHubArchJITRegisterRBP,
    HKHubArchJITRegisterRSI,
    HKHubArchJITRegisterRDI,
    
    HKHubArchJITRegisterR8B = 0,
    HKHubArchJITRegisterR9B,
    HKHubArchJITRegisterR10B,
    HKHubArchJITRegisterR11B,
    HKHubArchJITRegisterR12B,
    HKHubArchJITRegisterR13B,
    HKHubArchJITRegisterR14B,
    HKHubArchJITRegisterR15B,
    
    HKHubArchJITRegisterCompatibilityR0 = HKHubArchJITRegisterBL,
    HKHubArchJITRegisterCompatibilityR1 = HKHubArchJITRegisterBH,
    HKHubArchJITRegisterCompatibilityR2 = HKHubArchJITRegisterCL,
    HKHubArchJITRegisterCompatibilityR3 = HKHubArchJITRegisterCH,
    HKHubArchJITRegisterCompatibilityFlags = HKHubArchJITRegisterDL,
    HKHubArchJITRegisterCompatibilityPC = HKHubArchJITRegisterDH,
    HKHubArchJITRegisterCompatibilityCycles = HKHubArchJITRegisterRSI,
    HKHubArchJITRegisterCompatibilityMemory = HKHubArchJITRegisterRDI
} HKHubArchJITRegister;

enum {
    HKHubArchJITRMSIB = HKHubArchJITRegisterRSP
};

typedef enum {
    HKHubArchJITModAddress,
    HKHubArchJITModAddressDisp8,
    HKHubArchJITModAddressDisp32,
    HKHubArchJITModRegister
} HKHubArchJITMod;

typedef enum {
    HKHubArchJITOpcodeAddMR8 = 0,
    HKHubArchJITOpcodeAddRM8 = 2,
    HKHubArchJITOpcodeSubMR8 = 0x28,
    HKHubArchJITOpcodeSubRM8 = 0x2a,
    HKHubArchJITOpcodeOrMR8 = 8,
    HKHubArchJITOpcodeOrRM8 = 0x0a,
    HKHubArchJITOpcodeXorMR8 = 0x30,
    HKHubArchJITOpcodeXorRM8 = 0x32,
    HKHubArchJITOpcodeAndMR8 = 0x20,
    HKHubArchJITOpcodeAndRM8 = 0x22,
    HKHubArchJITOpcodeCmpMR8 = 0x38,
    HKHubArchJITOpcodeCmpRM8 = 0x3a,
    HKHubArchJITOpcodeMovMR8 = 0x88,
    HKHubArchJITOpcodeMovRM8 = 0x8a,
    HKHubArchJITOpcodeArithmeticMI8 = 0x80,
    HKHubArchJITOpcodeArithmeticMI = 0x83,
    HKHubArchJITOpcodeLahf = 0x9f,
    HKHubArchJITOpcodeBitAdjustMI8 = 0xc0,
    HKHubArchJITOpcodeBitAdjustMI = 0xc1,
    HKHubArchJITOpcodeBitAdjust1MI8 = 0xd0,
    HKHubArchJITOpcodeBitAdjust1MI = 0xd1,
    HKHubArchJITOpcodeBitAdjustMC8 = 0xd2,
    HKHubArchJITOpcodeRetn = 0xc3,
    HKHubArchJITOpcodeMovOI8 = 0xb0,
    HKHubArchJITOpcodeMovOI = 0xb8,
    HKHubArchJITOpcodeMoveMI8 = 0xc6,
    HKHubArchJITOpcodeTestMR8 = 0x84
} HKHubArchJITOpcode;

typedef enum {
    HKHubArchJITJumpAbove = 0x77,
    HKHubArchJITJumpAboveEqual = 0x73,
    HKHubArchJITJumpBelow = 0x72,
    HKHubArchJITJumpBelowEqual = 0x76,
    HKHubArchJITJumpCarry = 0x72,
    HKHubArchJITJumpEqual = 0x74,
    HKHubArchJITJumpGreater = 0x7f,
    HKHubArchJITJumpGreaterEqual = 0x7d,
    HKHubArchJITJumpLess = 0x7c,
    HKHubArchJITJumpLessEqual = 0x7e,
    HKHubArchJITJumpNotAbove = 0x76,
    HKHubArchJITJumpNotAboveEqual = 0x72,
    HKHubArchJITJumpNotBelow = 0x73,
    HKHubArchJITJumpNotBelowEqual = 0x77,
    HKHubArchJITJumpNotCarry = 0x73,
    HKHubArchJITJumpNotEqual = 0x75,
    HKHubArchJITJumpNotGreater = 0x7e,
    HKHubArchJITJumpNotGreaterEqual = 0x7c,
    HKHubArchJITJumpNotLess = 0x7d,
    HKHubArchJITJumpNotLessEqual = 0x7f,
    HKHubArchJITJumpOverflow = 0x70,
    HKHubArchJITJumpSign = 0x78,
    HKHubArchJITJumpZero = 0x74,
    HKHubArchJITJumpNotOverflow = 0x71,
    HKHubArchJITJumpNotSign = 0x79,
    HKHubArchJITJumpNotZero = 0x75,
    HKHubArchJITJumpUnconditional = 0xeb
} HKHubArchJITJump;

typedef enum {
    HKHubArchJITArithmeticAdd,
    HKHubArchJITArithmeticOr,
    HKHubArchJITArithmeticAdc,
    HKHubArchJITArithmeticSbb,
    HKHubArchJITArithmeticAnd,
    HKHubArchJITArithmeticSub,
    HKHubArchJITArithmeticXor,
    HKHubArchJITArithmeticCmp
} HKHubArchJITArithmetic;

typedef enum {
    HKHubArchJITBitAdjustRol,
    HKHubArchJITBitAdjustRor,
    HKHubArchJITBitAdjustRcl,
    HKHubArchJITBitAdjustRcr,
    HKHubArchJITBitAdjustShl,
    HKHubArchJITBitAdjustShr,
    HKHubArchJITBitAdjustSal,
    HKHubArchJITBitAdjustSar
} HKHubArchJITBitAdjust;

typedef enum {
    HKHubArchJITMoveMov
} HKHubArchJITMove;

enum {
    HKHubArchJIT0fPrefixOpcodeSeto = 0x90,
    HKHubArchJIT0fPrefixOpcodeSetl = 0x9c,
    HKHubArchJIT0fPrefixOpcodeSetnz = 0x95,
    HKHubArchJIT0fPrefixOpcodeMovzxMR8 = 0xb6
};

enum {
    HKHubArchJITRexB = 0x41,
    HKHubArchJITRexR = 0x44,
    HKHubArchJITRexRB = HKHubArchJITRexR | HKHubArchJITRexB,
    HKHubArchJITRexW = 0x48,
    HKHubArchJIT16Bit = 0x66
};

#if CC_HARDWARE_ARCH_X86_64
static uint8_t HKHubArchJITGetRegister(HKHubArchInstructionRegister Reg)
{
    switch (Reg)
    {
        case HKHubArchInstructionRegisterR0:
            return HKHubArchJITRegisterCompatibilityR0;
            
        case HKHubArchInstructionRegisterR1:
            return HKHubArchJITRegisterCompatibilityR1;
            
        case HKHubArchInstructionRegisterR2:
            return HKHubArchJITRegisterCompatibilityR2;
            
        case HKHubArchInstructionRegisterR3:
            return HKHubArchJITRegisterCompatibilityR3;
            
        case HKHubArchInstructionRegisterFlags:
            return HKHubArchJITRegisterCompatibilityFlags;
            
        case HKHubArchInstructionRegisterPC:
            return HKHubArchJITRegisterCompatibilityPC;
            
        default:
            break;
    }
    
    CCAssertLog(0, "Unsupported register type");
    
    return 0;
}

static CC_FORCE_INLINE uint8_t HKHubArchJITModRM(HKHubArchJITMod Mod, uint8_t Reg, uint8_t Rm)
{
    return (Mod << 6) | (Reg << 3) | Rm;
}

static CC_FORCE_INLINE uint8_t HKHubArchJITSIB(uint8_t Scale, HKHubArchJITRegister Index, HKHubArchJITRegister Base)
{
    return (Scale << 6) | (Index << 3) | Base;
}

static uint8_t HKHubArchJITGetModRM(const HKHubArchExecutionGraphInstruction *Instruction)
{
    if (Instruction->state.operand[0].type == HKHubArchInstructionOperandR)
    {
        if (Instruction->state.operand[1].type == HKHubArchInstructionOperandR)
        {
            return HKHubArchJITModRM(HKHubArchJITModRegister, HKHubArchJITGetRegister(Instruction->state.operand[1].reg), HKHubArchJITGetRegister(Instruction->state.operand[0].reg));
        }
    }
    
    return 0;
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionSeto(uint8_t *Ptr, size_t *Index, HKHubArchJITRegister Reg)
{
    Ptr[(*Index)++] = 0x0f;
    Ptr[(*Index)++] = HKHubArchJIT0fPrefixOpcodeSeto;
    Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, 0, Reg);
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionSetl(uint8_t *Ptr, size_t *Index, HKHubArchJITRegister Reg)
{
    Ptr[(*Index)++] = 0x0f;
    Ptr[(*Index)++] = HKHubArchJIT0fPrefixOpcodeSetl;
    Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, 0, Reg);
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionSetnz(uint8_t *Ptr, size_t *Index, HKHubArchJITRegister Reg)
{
    Ptr[(*Index)++] = 0x0f;
    Ptr[(*Index)++] = HKHubArchJIT0fPrefixOpcodeSetnz;
    Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, 0, Reg);
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionMovzxMR8(uint8_t *Ptr, size_t *Index, HKHubArchJITRegister Dst, HKHubArchJITRegister Src)
{
    Ptr[(*Index)++] = 0x0f;
    Ptr[(*Index)++] = HKHubArchJIT0fPrefixOpcodeMovzxMR8;
    Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Dst, Src);
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionSubMR8(uint8_t *Ptr, size_t *Index, HKHubArchJITRegister Dst, HKHubArchJITRegister Src)
{
    Ptr[(*Index)++] = HKHubArchJITOpcodeSubMR8;
    Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Src, Dst);
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionOrMR8(uint8_t *Ptr, size_t *Index, HKHubArchJITRegister Dst, HKHubArchJITRegister Src)
{
    Ptr[(*Index)++] = HKHubArchJITOpcodeOrMR8;
    Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Src, Dst);
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionTestMR8(uint8_t *Ptr, size_t *Index, HKHubArchJITRegister Dst, HKHubArchJITRegister Src)
{
    Ptr[(*Index)++] = HKHubArchJITOpcodeTestMR8;
    Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Src, Dst);
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionXorMR8(uint8_t *Ptr, size_t *Index, HKHubArchJITRegister Dst, HKHubArchJITRegister Src)
{
    Ptr[(*Index)++] = HKHubArchJITOpcodeXorMR8;
    Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Src, Dst);
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionMovOI8(uint8_t *Ptr, size_t *Index, HKHubArchJITRegister Reg, uint8_t Imm8)
{
    Ptr[(*Index)++] = HKHubArchJITOpcodeMovOI8 + Reg;
    Ptr[(*Index)++] = Imm8;
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionMovMR8(uint8_t *Ptr, size_t *Index, HKHubArchJITRegister Dst, HKHubArchJITRegister Src)
{
    Ptr[(*Index)++] = HKHubArchJITOpcodeMovMR8;
    Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Src, Dst);
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionMovOI(uint8_t *Ptr, size_t *Index, HKHubArchJITRegister Reg, uint32_t Imm32)
{
    Ptr[(*Index)++] = HKHubArchJITOpcodeMovOI + Reg;
    *(uint32_t*)&Ptr[*Index] = Imm32;
    *Index += 4;
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionArithmeticMI8(uint8_t *Ptr, size_t *Index, HKHubArchJITArithmetic Type, HKHubArchJITRegister Reg, uint8_t Imm8)
{
    Ptr[(*Index)++] = HKHubArchJITOpcodeArithmeticMI8;
    Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Type, Reg);
    Ptr[(*Index)++] = Imm8;
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionArithmeticMI(uint8_t *Ptr, size_t *Index, HKHubArchJITArithmetic Type, HKHubArchJITRegister Reg, uint8_t Imm8)
{
    Ptr[(*Index)++] = HKHubArchJITOpcodeArithmeticMI;
    Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Type, Reg);
    Ptr[(*Index)++] = Imm8;
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionBitAdjustMI8(uint8_t *Ptr, size_t *Index, HKHubArchJITBitAdjust Type, HKHubArchJITRegister Reg, uint8_t Imm8)
{
    if (Imm8 == 1)
    {
        Ptr[(*Index)++] = HKHubArchJITOpcodeBitAdjust1MI8;
        Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Type, Reg);
    }
    
    else if (Imm8)
    {
        Ptr[(*Index)++] = HKHubArchJITOpcodeBitAdjustMI8;
        Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Type, Reg);
        Ptr[(*Index)++] = Imm8;
    }
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionBitAdjustMI(uint8_t *Ptr, size_t *Index, HKHubArchJITBitAdjust Type, HKHubArchJITRegister Reg, uint8_t Imm8)
{
    if (Imm8 == 1)
    {
        Ptr[(*Index)++] = HKHubArchJITOpcodeBitAdjust1MI;
        Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Type, Reg);
    }
    
    else if (Imm8)
    {
        Ptr[(*Index)++] = HKHubArchJITOpcodeBitAdjustMI;
        Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Type, Reg);
        Ptr[(*Index)++] = Imm8;
    }
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionBitAdjustMC8(uint8_t *Ptr, size_t *Index, HKHubArchJITBitAdjust Type, HKHubArchJITRegister Reg)
{
    Ptr[(*Index)++] = HKHubArchJITOpcodeBitAdjustMC8;
    Ptr[(*Index)++] = HKHubArchJITModRM(HKHubArchJITModRegister, Type, Reg);
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionJumpRel8(uint8_t *Ptr, size_t *Index, HKHubArchJITJump Type, uint8_t Rel8)
{
    Ptr[(*Index)++] = Type;
    Ptr[(*Index)++] = Rel8;
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionReturn(uint8_t *Ptr, size_t *Index)
{
    Ptr[(*Index)++] = HKHubArchJITOpcodeRetn;
}

static void HKHubArchJITCopyFlagsSCZPresetO(uint8_t *Ptr, size_t *Index)
{
    Ptr[(*Index)++] = HKHubArchJITOpcodeLahf;
    
    /*
     sz0a0p1c 0000000o
     z0a0p1c0 000000os = AX rol 1
     c0z0a0p1          = AH ror 2
     0z0a0p10 00000osc = AX rol 1
     z0a0p100          = AH << 1
     0a0p1000 0000oscz = AX rol 1
     */
    Ptr[(*Index)++] = HKHubArchJIT16Bit;
    HKHubArchJITAddInstructionBitAdjustMI(Ptr, Index, HKHubArchJITBitAdjustRol, HKHubArchJITRegisterAX, 1);
    HKHubArchJITAddInstructionBitAdjustMI8(Ptr, Index, HKHubArchJITBitAdjustRor, HKHubArchJITRegisterAH, 2);
    Ptr[(*Index)++] = HKHubArchJIT16Bit;
    HKHubArchJITAddInstructionBitAdjustMI(Ptr, Index, HKHubArchJITBitAdjustRol, HKHubArchJITRegisterAX, 1);
    HKHubArchJITAddInstructionBitAdjustMI8(Ptr, Index, HKHubArchJITBitAdjustShl, HKHubArchJITRegisterAH, 1);
    Ptr[(*Index)++] = HKHubArchJIT16Bit;
    HKHubArchJITAddInstructionBitAdjustMI(Ptr, Index, HKHubArchJITBitAdjustRol, HKHubArchJITRegisterAX, 1);
    HKHubArchJITAddInstructionArithmeticMI8(Ptr, Index, HKHubArchJITArithmeticAnd, HKHubArchJITRegisterCompatibilityFlags, 0xf0);
    HKHubArchJITAddInstructionOrMR8(Ptr, Index, HKHubArchJITRegisterCompatibilityFlags, HKHubArchJITRegisterAL);
}

static void HKHubArchJITCopyFlags(uint8_t *Ptr, size_t *Index)
{
    HKHubArchJITAddInstructionSeto(Ptr, Index, HKHubArchJITRegisterAL);
    
    HKHubArchJITCopyFlagsSCZPresetO(Ptr, Index);
}

static void HKHubArchJITCheckCycles(uint8_t *Ptr, size_t *Index, size_t Cost, const HKHubArchExecutionGraphInstruction *Instruction)
{
    size_t Cycles = Cost;
    const HKHubArchInstructionMemoryOperation MemoryOp = HKHubArchInstructionGetMemoryOperation(&Instruction->state);
    for (size_t Loop = 0; Loop < 3; Loop++)
    {
        if (Instruction->state.operand[Loop].type == HKHubArchInstructionOperandM)
        {
            _Static_assert(HKHubArchInstructionMemoryOperationOp1 == 0 &&
                           HKHubArchInstructionMemoryOperationOp2 == 2 &&
                           HKHubArchInstructionMemoryOperationOp3 == 4, "Expects the following operand mask layout");
            
            Cycles += (((MemoryOp >> (Loop * 2)) & HKHubArchInstructionMemoryOperationSrc) ? HKHubArchProcessorSpeedMemoryRead : 0) + (((MemoryOp >> (Loop * 2)) & HKHubArchInstructionMemoryOperationDst) ? HKHubArchProcessorSpeedMemoryWrite : 0);
        }
    }
    
    Ptr[(*Index)++] = HKHubArchJITRexW;
    HKHubArchJITAddInstructionArithmeticMI(Ptr, Index, HKHubArchJITArithmeticSub, HKHubArchJITRegisterCompatibilityCycles, Cycles);
    HKHubArchJITAddInstructionJumpRel8(Ptr, Index, HKHubArchJITJumpNotBelow, 5);
    Ptr[(*Index)++] = HKHubArchJITRexW;
    HKHubArchJITAddInstructionArithmeticMI(Ptr, Index, HKHubArchJITArithmeticAdd, HKHubArchJITRegisterCompatibilityCycles, Cycles);
    HKHubArchJITAddInstructionReturn(Ptr, Index);
}

static size_t HKHubArchJITGenerate2OperandMutator(uint8_t *Ptr, const HKHubArchExecutionGraphInstruction *Instruction, uint8_t Type, HKHubArchJITOpcode OpcodeMR, HKHubArchJITOpcode OpcodeRM, HKHubArchJITOpcode OpcodeMI, size_t Cost, _Bool FlagsAffected)
{
    const size_t Size = HKHubArchInstructionSizeOfEncoding(&Instruction->state);
    
    size_t Index = 0;
    HKHubArchJITCheckCycles(Ptr, &Index, Cost + (Size * HKHubArchProcessorSpeedMemoryRead), Instruction);
    
    if (Instruction->state.operand[0].type == HKHubArchInstructionOperandR)
    {
        if (Instruction->state.operand[1].type == HKHubArchInstructionOperandR)
        {
            Ptr[Index++] = OpcodeMR;
            Ptr[Index++] = HKHubArchJITGetModRM(Instruction);
        }
        
        else if (Instruction->state.operand[1].type == HKHubArchInstructionOperandI)
        {
            Ptr[Index++] = OpcodeMI;
            Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModRegister, Type, HKHubArchJITGetRegister(Instruction->state.operand[0].reg));
            Ptr[Index++] = Instruction->state.operand[1].value;
        }
        
        else if (Instruction->state.operand[1].type == HKHubArchInstructionOperandM)
        {
            switch (Instruction->state.operand[1].memory.type)
            {
                case HKHubArchInstructionMemoryOffset:
                    Ptr[Index++] = OpcodeRM;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddressDisp8, HKHubArchJITGetRegister(Instruction->state.operand[0].reg), HKHubArchJITRegisterCompatibilityMemory);
                    Ptr[Index++] = Instruction->state.operand[1].memory.offset;
                    break;
                    
                case HKHubArchInstructionMemoryRegister:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[1].memory.reg));
                    Ptr[Index++] = OpcodeRM;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITGetRegister(Instruction->state.operand[0].reg), HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    break;
                    
                case HKHubArchInstructionMemoryRelativeOffset:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[1].memory.relativeOffset.reg));
                    HKHubArchJITAddInstructionArithmeticMI8(Ptr, &Index, HKHubArchJITArithmeticAdd, HKHubArchJITRegisterAL, Instruction->state.operand[1].memory.relativeOffset.offset);
                    Ptr[Index++] = OpcodeRM;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITGetRegister(Instruction->state.operand[0].reg), HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    break;
                    
                case HKHubArchInstructionMemoryRelativeRegister:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[1].memory.relativeReg[0]));
                    Ptr[Index++] = HKHubArchJITOpcodeAddMR8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModRegister, HKHubArchJITGetRegister(Instruction->state.operand[1].memory.relativeReg[1]), HKHubArchJITRegisterAL);
                    Ptr[Index++] = OpcodeRM;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITGetRegister(Instruction->state.operand[0].reg), HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    break;
            }
        }
        
        if (FlagsAffected) HKHubArchJITCopyFlags(Ptr, &Index);
        
        if ((Instruction->state.operand[0].reg & HKHubArchInstructionRegisterSpecialPurpose) && (Instruction->state.operand[0].reg == HKHubArchInstructionRegisterPC) && ((HKHubArchInstructionGetMemoryOperation(&Instruction->state) >> HKHubArchInstructionMemoryOperationOp1) & HKHubArchInstructionMemoryOperationDst)) HKHubArchJITAddInstructionReturn(Ptr, &Index);
        else HKHubArchJITAddInstructionArithmeticMI8(Ptr, &Index, HKHubArchJITArithmeticAdd, HKHubArchJITRegisterCompatibilityPC, Size);
    }
    
    else if (Instruction->state.operand[0].type == HKHubArchInstructionOperandM)
    {
        if (Instruction->state.operand[1].type == HKHubArchInstructionOperandR)
        {
            switch (Instruction->state.operand[0].memory.type)
            {
                case HKHubArchInstructionMemoryOffset:
                    Ptr[Index++] = OpcodeMR;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddressDisp8, HKHubArchJITGetRegister(Instruction->state.operand[1].reg), HKHubArchJITRegisterCompatibilityMemory);
                    Ptr[Index++] = Instruction->state.operand[0].memory.offset;
                    break;
                    
                case HKHubArchInstructionMemoryRegister:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[0].memory.reg));
                    Ptr[Index++] = OpcodeMR;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITGetRegister(Instruction->state.operand[1].reg), HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    break;
                    
                case HKHubArchInstructionMemoryRelativeOffset:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[0].memory.relativeOffset.reg));
                    HKHubArchJITAddInstructionArithmeticMI8(Ptr, &Index, HKHubArchJITArithmeticAdd, HKHubArchJITRegisterAL, Instruction->state.operand[0].memory.relativeOffset.offset);
                    Ptr[Index++] = OpcodeMR;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITGetRegister(Instruction->state.operand[1].reg), HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    break;
                    
                case HKHubArchInstructionMemoryRelativeRegister:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[0].memory.relativeReg[0]));
                    Ptr[Index++] = HKHubArchJITOpcodeAddMR8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModRegister, HKHubArchJITGetRegister(Instruction->state.operand[0].memory.relativeReg[1]), HKHubArchJITRegisterAL);
                    Ptr[Index++] = OpcodeMR;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITGetRegister(Instruction->state.operand[1].reg), HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    break;
            }
        }
        
        else if (Instruction->state.operand[1].type == HKHubArchInstructionOperandI)
        {
            switch (Instruction->state.operand[0].memory.type)
            {
                case HKHubArchInstructionMemoryOffset:
                    Ptr[Index++] = OpcodeMI;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddressDisp8, Type, HKHubArchJITRegisterCompatibilityMemory);
                    Ptr[Index++] = Instruction->state.operand[0].memory.offset;
                    Ptr[Index++] = Instruction->state.operand[1].value;
                    break;
                    
                case HKHubArchInstructionMemoryRegister:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[0].memory.reg));
                    Ptr[Index++] = OpcodeMI;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, Type, HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    Ptr[Index++] = Instruction->state.operand[1].value;
                    break;
                    
                case HKHubArchInstructionMemoryRelativeOffset:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[0].memory.relativeOffset.reg));
                    HKHubArchJITAddInstructionArithmeticMI8(Ptr, &Index, HKHubArchJITArithmeticAdd, HKHubArchJITRegisterAL, Instruction->state.operand[0].memory.relativeOffset.offset);
                    Ptr[Index++] = OpcodeMI;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, Type, HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    Ptr[Index++] = Instruction->state.operand[1].value;
                    break;
                    
                case HKHubArchInstructionMemoryRelativeRegister:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[0].memory.relativeReg[0]));
                    Ptr[Index++] = HKHubArchJITOpcodeAddMR8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModRegister, HKHubArchJITGetRegister(Instruction->state.operand[0].memory.relativeReg[1]), HKHubArchJITRegisterAL);
                    Ptr[Index++] = OpcodeMI;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, Type, HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    Ptr[Index++] = Instruction->state.operand[1].value;
                    break;
            }
        }
        
        else if (Instruction->state.operand[1].type == HKHubArchInstructionOperandM)
        {
            switch (Instruction->state.operand[0].memory.type)
            {
                case HKHubArchInstructionMemoryOffset:
                    HKHubArchJITAddInstructionMovOI(Ptr, &Index, HKHubArchJITRegisterEBP, Instruction->state.operand[0].memory.offset);
                    break;
                    
                case HKHubArchInstructionMemoryRegister:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEBP, HKHubArchJITGetRegister(Instruction->state.operand[0].memory.reg));
                    break;
                    
                case HKHubArchInstructionMemoryRelativeOffset:
                    Ptr[Index++] = HKHubArchJITOpcodeMovMR8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModRegister, HKHubArchJITGetRegister(Instruction->state.operand[0].memory.relativeOffset.reg), HKHubArchJITRegisterAL);
                    HKHubArchJITAddInstructionArithmeticMI8(Ptr, &Index, HKHubArchJITArithmeticAdd, HKHubArchJITRegisterAL, Instruction->state.operand[0].memory.relativeOffset.offset);
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEBP, HKHubArchJITRegisterAL);
                    break;
                    
                case HKHubArchInstructionMemoryRelativeRegister:
                    Ptr[Index++] = HKHubArchJITOpcodeMovMR8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModRegister, HKHubArchJITGetRegister(Instruction->state.operand[0].memory.relativeReg[0]), HKHubArchJITRegisterAL);
                    Ptr[Index++] = HKHubArchJITOpcodeAddMR8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModRegister, HKHubArchJITGetRegister(Instruction->state.operand[0].memory.relativeReg[1]), HKHubArchJITRegisterAL);
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEBP, HKHubArchJITRegisterAL);
                    break;
            }
            
            switch (Instruction->state.operand[1].memory.type)
            {
                case HKHubArchInstructionMemoryOffset:
                    Ptr[Index++] = HKHubArchJITOpcodeMovRM8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddressDisp8, HKHubArchJITRegisterAL, HKHubArchJITRegisterCompatibilityMemory);
                    Ptr[Index++] = Instruction->state.operand[1].memory.offset;
                    break;
                    
                case HKHubArchInstructionMemoryRegister:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[1].memory.reg));
                    Ptr[Index++] = HKHubArchJITOpcodeMovRM8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITRegisterAL, HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    break;
                    
                case HKHubArchInstructionMemoryRelativeOffset:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[1].memory.relativeOffset.reg));
                    HKHubArchJITAddInstructionArithmeticMI8(Ptr, &Index, HKHubArchJITArithmeticAdd, HKHubArchJITRegisterAL, Instruction->state.operand[1].memory.relativeOffset.offset);
                    Ptr[Index++] = HKHubArchJITOpcodeMovRM8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITRegisterAL, HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    break;
                    
                case HKHubArchInstructionMemoryRelativeRegister:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[1].memory.relativeReg[0]));
                    Ptr[Index++] = HKHubArchJITOpcodeAddMR8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModRegister, HKHubArchJITGetRegister(Instruction->state.operand[1].memory.relativeReg[1]), HKHubArchJITRegisterAL);
                    Ptr[Index++] = HKHubArchJITOpcodeMovRM8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITRegisterAL, HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    break;
            }
            
            Ptr[Index++] = OpcodeMR;
            Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITRegisterAL, HKHubArchJITRMSIB);
            Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRBP, HKHubArchJITRegisterCompatibilityMemory);
        }
        
        if (FlagsAffected) HKHubArchJITCopyFlags(Ptr, &Index);
        
        HKHubArchJITAddInstructionArithmeticMI8(Ptr, &Index, HKHubArchJITArithmeticAdd, HKHubArchJITRegisterCompatibilityPC, Size);
    }
    
    return Index;
}

static CC_FORCE_INLINE void HKHubArchJITAddInstructionBitAdjustMR8(uint8_t *Ptr, size_t *Index, HKHubArchJITBitAdjust Type, HKHubArchJITRegister Dst, HKHubArchJITRegister Src)
{
    HKHubArchJITRegister Target = Dst;
    
    if (Src != HKHubArchJITRegisterCL)
    {
        HKHubArchJITAddInstructionMovMR8(Ptr, Index, HKHubArchJITRegisterAL, HKHubArchJITRegisterCL);
        HKHubArchJITAddInstructionMovMR8(Ptr, Index, HKHubArchJITRegisterCL, Src);
        
        if (Dst == HKHubArchJITRegisterCL) Target = HKHubArchJITRegisterAL;
    }
    
    HKHubArchJITAddInstructionBitAdjustMC8(Ptr, Index, Type, Target);
    
    if (Src != HKHubArchJITRegisterCL) HKHubArchJITAddInstructionMovMR8(Ptr, Index, HKHubArchJITRegisterCL, HKHubArchJITRegisterAL);
}

static size_t HKHubArchJITGenerate2OperandBitMutator(uint8_t *Ptr, const HKHubArchExecutionGraphInstruction *Instruction, uint8_t Type, size_t Cost, _Bool FlagsAffected)
{
    const size_t Size = HKHubArchInstructionSizeOfEncoding(&Instruction->state);
    
    size_t Index = 0;
    HKHubArchJITCheckCycles(Ptr, &Index, Cost + (Size * HKHubArchProcessorSpeedMemoryRead), Instruction);
    
    if (Instruction->state.operand[0].type == HKHubArchInstructionOperandR)
    {
        if (Instruction->state.operand[1].type == HKHubArchInstructionOperandR)
        {
            const HKHubArchJITRegister Dst = HKHubArchJITGetRegister(Instruction->state.operand[0].reg), Src = HKHubArchJITGetRegister(Instruction->state.operand[1].reg);
            
            HKHubArchJITAddInstructionMovMR8(Ptr, &Index, HKHubArchJITRegisterAL, HKHubArchJITRegisterCL);
            HKHubArchJITAddInstructionMovMR8(Ptr, &Index, HKHubArchJITRegisterAH, Dst);
            
            HKHubArchJITAddInstructionMovOI8(Ptr, &Index, HKHubArchJITRegisterCL, 8);
            HKHubArchJITAddInstructionSubMR8(Ptr, &Index, HKHubArchJITRegisterCL, (Src == HKHubArchJITRegisterCL ? HKHubArchJITRegisterAL : Src));
            HKHubArchJITAddInstructionJumpRel8(Ptr, &Index, HKHubArchJITJumpCarry, 2);
            HKHubArchJITAddInstructionBitAdjustMC8(Ptr, &Index, (Type == HKHubArchJITBitAdjustShl ? HKHubArchJITBitAdjustShr : HKHubArchJITBitAdjustShl), HKHubArchJITRegisterAH);
            HKHubArchJITAddInstructionTestMR8(Ptr, &Index, HKHubArchJITRegisterAH, HKHubArchJITRegisterAH);
            Ptr[Index++] = HKHubArchJITRexB;
            HKHubArchJITAddInstructionSetnz(Ptr, &Index, HKHubArchJITRegisterR8B);
            HKHubArchJITAddInstructionMovMR8(Ptr, &Index, HKHubArchJITRegisterCL, HKHubArchJITRegisterAL);
            
            HKHubArchJITAddInstructionTestMR8(Ptr, &Index, Dst, Dst);
            HKHubArchJITAddInstructionBitAdjustMR8(Ptr, &Index, Type, Dst, Src);
        }
        
        else if (Instruction->state.operand[1].type == HKHubArchInstructionOperandI)
        {
            const HKHubArchJITRegister Dst = HKHubArchJITGetRegister(Instruction->state.operand[0].reg);
            
            if (Instruction->state.operand[1].value)
            {
                HKHubArchJITAddInstructionMovMR8(Ptr, &Index, HKHubArchJITRegisterAL, HKHubArchJITRegisterCL);
                HKHubArchJITAddInstructionMovMR8(Ptr, &Index, HKHubArchJITRegisterAH, Dst);
                
                HKHubArchJITAddInstructionMovOI8(Ptr, &Index, HKHubArchJITRegisterCL, 8);
                HKHubArchJITAddInstructionArithmeticMI8(Ptr, &Index, HKHubArchJITArithmeticSub, HKHubArchJITRegisterCL, Instruction->state.operand[1].value);
                HKHubArchJITAddInstructionJumpRel8(Ptr, &Index, HKHubArchJITJumpCarry, 2);
                HKHubArchJITAddInstructionBitAdjustMC8(Ptr, &Index, (Type == HKHubArchJITBitAdjustShl ? HKHubArchJITBitAdjustShr : HKHubArchJITBitAdjustShl), HKHubArchJITRegisterAH);
                HKHubArchJITAddInstructionTestMR8(Ptr, &Index, HKHubArchJITRegisterAH, HKHubArchJITRegisterAH);
                Ptr[Index++] = HKHubArchJITRexB;
                HKHubArchJITAddInstructionSetnz(Ptr, &Index, HKHubArchJITRegisterR8B);
                HKHubArchJITAddInstructionMovMR8(Ptr, &Index, HKHubArchJITRegisterCL, HKHubArchJITRegisterAL);
                
                HKHubArchJITAddInstructionBitAdjustMI8(Ptr, &Index, Type, Dst, Instruction->state.operand[1].value);
            }
            
            else
            {
                // optimize, can do this afterwards and set AL directly
                Ptr[Index++] = HKHubArchJITRexRB;
                HKHubArchJITAddInstructionXorMR8(Ptr, &Index, HKHubArchJITRegisterR8B, HKHubArchJITRegisterR8B);
                
                HKHubArchJITAddInstructionTestMR8(Ptr, &Index, Dst, Dst);
            }            
        }
        
        else if (Instruction->state.operand[1].type == HKHubArchInstructionOperandM)
        {
            switch (Instruction->state.operand[1].memory.type)
            {
                case HKHubArchInstructionMemoryOffset:
                    Ptr[Index++] = HKHubArchJITOpcodeMovRM8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddressDisp8, HKHubArchJITRegisterAH, HKHubArchJITRegisterCompatibilityMemory);
                    Ptr[Index++] = Instruction->state.operand[1].memory.offset;
                    break;
                    
                case HKHubArchInstructionMemoryRegister:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[1].memory.reg));
                    Ptr[Index++] = HKHubArchJITOpcodeMovRM8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITRegisterAH, HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    break;
                    
                case HKHubArchInstructionMemoryRelativeOffset:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[1].memory.relativeOffset.reg));
                    HKHubArchJITAddInstructionArithmeticMI8(Ptr, &Index, HKHubArchJITArithmeticAdd, HKHubArchJITRegisterAL, Instruction->state.operand[1].memory.relativeOffset.offset);
                    Ptr[Index++] = HKHubArchJITOpcodeMovRM8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITRegisterAH, HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    break;
                    
                case HKHubArchInstructionMemoryRelativeRegister:
                    HKHubArchJITAddInstructionMovzxMR8(Ptr, &Index, HKHubArchJITRegisterEAX, HKHubArchJITGetRegister(Instruction->state.operand[1].memory.relativeReg[0]));
                    Ptr[Index++] = HKHubArchJITOpcodeAddMR8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModRegister, HKHubArchJITGetRegister(Instruction->state.operand[1].memory.relativeReg[1]), HKHubArchJITRegisterAL);
                    Ptr[Index++] = HKHubArchJITOpcodeMovRM8;
                    Ptr[Index++] = HKHubArchJITModRM(HKHubArchJITModAddress, HKHubArchJITRegisterAH, HKHubArchJITRMSIB);
                    Ptr[Index++] = HKHubArchJITSIB(0, HKHubArchJITRegisterRAX, HKHubArchJITRegisterCompatibilityMemory);
                    break;
            }
            
            const HKHubArchJITRegister Dst = HKHubArchJITGetRegister(Instruction->state.operand[0].reg);
            
            Ptr[Index++] = HKHubArchJITRexB;
            HKHubArchJITAddInstructionMovMR8(Ptr, &Index, HKHubArchJITRegisterR8B, HKHubArchJITRegisterCL);
            HKHubArchJITAddInstructionMovMR8(Ptr, &Index, HKHubArchJITRegisterAL, Dst);
            
            HKHubArchJITAddInstructionMovOI8(Ptr, &Index, HKHubArchJITRegisterCL, 8);
            HKHubArchJITAddInstructionSubMR8(Ptr, &Index, HKHubArchJITRegisterCL, HKHubArchJITRegisterAH);
            HKHubArchJITAddInstructionJumpRel8(Ptr, &Index, HKHubArchJITJumpCarry, 2);
            HKHubArchJITAddInstructionBitAdjustMC8(Ptr, &Index, (Type == HKHubArchJITBitAdjustShl ? HKHubArchJITBitAdjustShr : HKHubArchJITBitAdjustShl), HKHubArchJITRegisterAL);
            HKHubArchJITAddInstructionTestMR8(Ptr, &Index, HKHubArchJITRegisterAL, HKHubArchJITRegisterAL);
            Ptr[Index++] = HKHubArchJITRexR;
            HKHubArchJITAddInstructionMovMR8(Ptr, &Index, HKHubArchJITRegisterCL, HKHubArchJITRegisterR8B);
            Ptr[Index++] = HKHubArchJITRexB;
            HKHubArchJITAddInstructionSetnz(Ptr, &Index, HKHubArchJITRegisterR8B);
            
            HKHubArchJITAddInstructionTestMR8(Ptr, &Index, Dst, Dst);
            HKHubArchJITAddInstructionBitAdjustMR8(Ptr, &Index, Type, Dst, HKHubArchJITRegisterAH);
        }
        
        if (FlagsAffected)
        {
            Ptr[Index++] = HKHubArchJITRexR;
            HKHubArchJITAddInstructionMovMR8(Ptr, &Index, HKHubArchJITRegisterAL, HKHubArchJITRegisterR8B);
            HKHubArchJITCopyFlagsSCZPresetO(Ptr, &Index);
        }
        
        if ((Instruction->state.operand[0].reg & HKHubArchInstructionRegisterSpecialPurpose) && (Instruction->state.operand[0].reg == HKHubArchInstructionRegisterPC) && ((HKHubArchInstructionGetMemoryOperation(&Instruction->state) >> HKHubArchInstructionMemoryOperationOp1) & HKHubArchInstructionMemoryOperationDst)) HKHubArchJITAddInstructionReturn(Ptr, &Index);
        else HKHubArchJITAddInstructionArithmeticMI8(Ptr, &Index, HKHubArchJITArithmeticAdd, HKHubArchJITRegisterCompatibilityPC, Size);
    }
    
    return Index;
}

static size_t HKHubArchJITGenerateAdd(uint8_t *Ptr, const HKHubArchExecutionGraphInstruction *Instruction)
{
    return HKHubArchJITGenerate2OperandMutator(Ptr, Instruction, HKHubArchJITArithmeticAdd, HKHubArchJITOpcodeAddMR8, HKHubArchJITOpcodeAddRM8, HKHubArchJITOpcodeArithmeticMI8, 2, TRUE);
}

static size_t HKHubArchJITGenerateSub(uint8_t *Ptr, const HKHubArchExecutionGraphInstruction *Instruction)
{
    return HKHubArchJITGenerate2OperandMutator(Ptr, Instruction, HKHubArchJITArithmeticSub, HKHubArchJITOpcodeSubMR8, HKHubArchJITOpcodeSubRM8, HKHubArchJITOpcodeArithmeticMI8, 2, TRUE);
}

static size_t HKHubArchJITGenerateOr(uint8_t *Ptr, const HKHubArchExecutionGraphInstruction *Instruction)
{
    return HKHubArchJITGenerate2OperandMutator(Ptr, Instruction, HKHubArchJITArithmeticOr, HKHubArchJITOpcodeOrMR8, HKHubArchJITOpcodeOrRM8, HKHubArchJITOpcodeArithmeticMI8, 1, TRUE);
}

static size_t HKHubArchJITGenerateXor(uint8_t *Ptr, const HKHubArchExecutionGraphInstruction *Instruction)
{
    return HKHubArchJITGenerate2OperandMutator(Ptr, Instruction, HKHubArchJITArithmeticXor, HKHubArchJITOpcodeXorMR8, HKHubArchJITOpcodeXorRM8, HKHubArchJITOpcodeArithmeticMI8, 1, TRUE);
}

static size_t HKHubArchJITGenerateAnd(uint8_t *Ptr, const HKHubArchExecutionGraphInstruction *Instruction)
{
    return HKHubArchJITGenerate2OperandMutator(Ptr, Instruction, HKHubArchJITArithmeticAnd, HKHubArchJITOpcodeAndMR8, HKHubArchJITOpcodeAndRM8, HKHubArchJITOpcodeArithmeticMI8, 1, TRUE);
}

static size_t HKHubArchJITGenerateCmp(uint8_t *Ptr, const HKHubArchExecutionGraphInstruction *Instruction)
{
    return HKHubArchJITGenerate2OperandMutator(Ptr, Instruction, HKHubArchJITArithmeticCmp, HKHubArchJITOpcodeCmpMR8, HKHubArchJITOpcodeCmpRM8, HKHubArchJITOpcodeArithmeticMI8, 2, TRUE);
}

static size_t HKHubArchJITGenerateMov(uint8_t *Ptr, const HKHubArchExecutionGraphInstruction *Instruction)
{
    return HKHubArchJITGenerate2OperandMutator(Ptr, Instruction, HKHubArchJITMoveMov, HKHubArchJITOpcodeMovMR8, HKHubArchJITOpcodeMovRM8, HKHubArchJITOpcodeMoveMI8, 1, FALSE);
}

static size_t HKHubArchJITGenerateShl(uint8_t *Ptr, const HKHubArchExecutionGraphInstruction *Instruction)
{
    return HKHubArchJITGenerate2OperandBitMutator(Ptr, Instruction, HKHubArchJITBitAdjustShl, 1, TRUE);
}

static size_t HKHubArchJITGenerateShr(uint8_t *Ptr, const HKHubArchExecutionGraphInstruction *Instruction)
{
    return HKHubArchJITGenerate2OperandBitMutator(Ptr, Instruction, HKHubArchJITBitAdjustShr, 1, TRUE);
}

_Bool HKHubArchJITGenerateBlock(HKHubArchJIT JIT, HKHubArchJITBlock *JITBlock, void *Ptr, CCLinkedList(HKHubArchExecutionGraphInstruction) Block)
{
    /*
     rax : reserved
     
     bl : r0
     bh : r1
     cl : r2
     ch : r3
     
     dl : flags
     dh : pc
     
     rsi : cycles
     rdi : memory
     
     rbp : reserved
     rsp : reserved
     */
    CCEnumerable Enumerable;
    CCLinkedListGetEnumerable(Block, &Enumerable);
    
    size_t Index = 0, InstructionIndex = 0, ReturnIndex = 0;
    for (const HKHubArchExecutionGraphInstruction *Instruction = CCEnumerableGetCurrent(&Enumerable); Instruction; Instruction = CCEnumerableNext(&Enumerable), InstructionIndex++)
    {
        switch (Instruction->state.opcode)
        {
            case 0:
            case 1:
                CCArrayAppendElement(JITBlock->map, &(HKHubArchJITBlockRelativeEntry){ .entry = (uintptr_t)(Ptr + Index), .index = InstructionIndex });
                Index += HKHubArchJITGenerateAdd(&Ptr[Index], Instruction);
                break;
                
            case 2:
            case 6:
                CCArrayAppendElement(JITBlock->map, &(HKHubArchJITBlockRelativeEntry){ .entry = (uintptr_t)(Ptr + Index), .index = InstructionIndex });
                Index += HKHubArchJITGenerateMov(&Ptr[Index], Instruction);
                break;
                
            case 4:
            case 5:
                CCArrayAppendElement(JITBlock->map, &(HKHubArchJITBlockRelativeEntry){ .entry = (uintptr_t)(Ptr + Index), .index = InstructionIndex });
                Index += HKHubArchJITGenerateSub(&Ptr[Index], Instruction);
                break;
                
            case 28:
            case 29:
                CCArrayAppendElement(JITBlock->map, &(HKHubArchJITBlockRelativeEntry){ .entry = (uintptr_t)(Ptr + Index), .index = InstructionIndex });
                Index += HKHubArchJITGenerateCmp(&Ptr[Index], Instruction);
                break;
                
            case 32:
            case 33:
                if (Instruction->state.operand[0].type == HKHubArchInstructionOperandM)
                {
                    // TODO: Support memory destination
                    if (Index != ReturnIndex)
                    {
                        HKHubArchJITAddInstructionReturn(Ptr, &Index);
                        ReturnIndex = Index;
                    }
                }
                
                else
                {
                    CCArrayAppendElement(JITBlock->map, &(HKHubArchJITBlockRelativeEntry){ .entry = (uintptr_t)(Ptr + Index), .index = InstructionIndex });
                    Index += HKHubArchJITGenerateShl(&Ptr[Index], Instruction);
                }
                break;
                
            case 36:
            case 37:
                if (Instruction->state.operand[0].type == HKHubArchInstructionOperandM)
                {
                    // TODO: Support memory destination
                    if (Index != ReturnIndex)
                    {
                        HKHubArchJITAddInstructionReturn(Ptr, &Index);
                        ReturnIndex = Index;
                    }
                }
                
                else
                {
                    CCArrayAppendElement(JITBlock->map, &(HKHubArchJITBlockRelativeEntry){ .entry = (uintptr_t)(Ptr + Index), .index = InstructionIndex });
                    Index += HKHubArchJITGenerateShr(&Ptr[Index], Instruction);
                }
                break;
                
            case 40:
            case 41:
                CCArrayAppendElement(JITBlock->map, &(HKHubArchJITBlockRelativeEntry){ .entry = (uintptr_t)(Ptr + Index), .index = InstructionIndex });
                Index += HKHubArchJITGenerateXor(&Ptr[Index], Instruction);
                break;
                
            case 44:
            case 45:
                CCArrayAppendElement(JITBlock->map, &(HKHubArchJITBlockRelativeEntry){ .entry = (uintptr_t)(Ptr + Index), .index = InstructionIndex });
                Index += HKHubArchJITGenerateOr(&Ptr[Index], Instruction);
                break;
                
            case 48:
            case 49:
                CCArrayAppendElement(JITBlock->map, &(HKHubArchJITBlockRelativeEntry){ .entry = (uintptr_t)(Ptr + Index), .index = InstructionIndex });
                Index += HKHubArchJITGenerateAnd(&Ptr[Index], Instruction);
                break;
                
            default:
                if (Index != ReturnIndex)
                {
                    HKHubArchJITAddInstructionReturn(Ptr, &Index);
                    ReturnIndex = Index;
                }
                break;
        }
    }
    
    if (Index != ReturnIndex)
    {
        HKHubArchJITAddInstructionReturn(Ptr, &Index);
        
        return TRUE;
    }
    
    return Index;
}

void HKHubArchJITCall(HKHubArchJIT JIT, HKHubArchProcessor Processor)
{
    const uintptr_t *Entry = CCDictionaryGetValue(JIT->map, &Processor->state.pc);
    if (!Entry) return;
    
#define HK_HUB_ARCH_JIT_Processor_r0 56
#define HK_HUB_ARCH_JIT_Processor_r1 57
#define HK_HUB_ARCH_JIT_Processor_r2 58
#define HK_HUB_ARCH_JIT_Processor_r3 59
#define HK_HUB_ARCH_JIT_Processor_pc 60
#define HK_HUB_ARCH_JIT_Processor_flags 61
#define HK_HUB_ARCH_JIT_Processor_cycles 144
#define HK_HUB_ARCH_JIT_Processor_memory 164
    
    _Static_assert(HK_HUB_ARCH_JIT_Processor_r0 == offsetof(typeof(*Processor), state.r[0]) &&
                   HK_HUB_ARCH_JIT_Processor_r1 == offsetof(typeof(*Processor), state.r[1]) &&
                   HK_HUB_ARCH_JIT_Processor_r2 == offsetof(typeof(*Processor), state.r[2]) &&
                   HK_HUB_ARCH_JIT_Processor_r3 == offsetof(typeof(*Processor), state.r[3]) &&
                   HK_HUB_ARCH_JIT_Processor_pc == offsetof(typeof(*Processor), state.pc) &&
                   HK_HUB_ARCH_JIT_Processor_flags == offsetof(typeof(*Processor), state.flags) &&
                   HK_HUB_ARCH_JIT_Processor_cycles == offsetof(typeof(*Processor), cycles) &&
                   HK_HUB_ARCH_JIT_Processor_memory == offsetof(typeof(*Processor), memory), "Need to update the offsets");
    
#define HK_HUB_ARCH_JIT_LABEL(label) HK_HUB_ARCH_JIT_LABEL_(label, HK_HUB_ARCH_JIT_##label)
#define HK_HUB_ARCH_JIT_LABEL_(label, value) HK_HUB_ARCH_JIT_LABEL__(label, value)
#define HK_HUB_ARCH_JIT_LABEL__(label, value) ".set " #label ", " #value "\n"
    
    asm(HK_HUB_ARCH_JIT_LABEL(Processor_r0)
        HK_HUB_ARCH_JIT_LABEL(Processor_r1)
        HK_HUB_ARCH_JIT_LABEL(Processor_r2)
        HK_HUB_ARCH_JIT_LABEL(Processor_r3)
        HK_HUB_ARCH_JIT_LABEL(Processor_pc)
        HK_HUB_ARCH_JIT_LABEL(Processor_flags)
        HK_HUB_ARCH_JIT_LABEL(Processor_cycles)
        HK_HUB_ARCH_JIT_LABEL(Processor_memory)
        "movq %0, %%rax\n"
        "movb Processor_r0(%%rax), %%bl\n"
        "movb Processor_r1(%%rax), %%bh\n"
        "movb Processor_r2(%%rax), %%cl\n"
        "movb Processor_r3(%%rax), %%ch\n"
        "movb Processor_flags(%%rax), %%dl\n"
        "movb Processor_pc(%%rax), %%dh\n"
        "movq Processor_cycles(%%rax), %%rsi\n"
        "leaq Processor_memory(%%rax), %%rdi\n"
        "pushq %%rbp\n"
        "pushq %%rax\n"
        "xorq %%rbp, %%rbp\n"
        "xorq %%rax, %%rax\n"
        "callq *%1\n"
        "popq %%rax\n"
        "popq %%rbp\n"
        "movb %%bl, Processor_r0(%%rax)\n"
        "movb %%bh, Processor_r1(%%rax)\n"
        "movb %%cl, Processor_r2(%%rax)\n"
        "movb %%ch, Processor_r3(%%rax)\n"
        "movb %%dl, Processor_flags(%%rax)\n"
        "movb %%dh, Processor_pc(%%rax)\n"
        "movq %%rsi, Processor_cycles(%%rax)\n"
        :
        : "m" (Processor), "m" (*Entry)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "memory");
}
#endif
