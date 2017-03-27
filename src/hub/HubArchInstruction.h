/*
 *  Copyright (c) 2016, Stefan Johnson
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

#ifndef HackingGame_HubArchInstruction_h
#define HackingGame_HubArchInstruction_h

#include <Blob2D/Blob2D.h>
#include "HubArchAssembly.h"
#include "HubArchProcessor.h"

typedef enum {
    HKHubArchInstructionOperandNA = (1 << 0),
    HKHubArchInstructionOperandI = (1 << 1),
    HKHubArchInstructionOperandR = (1 << 2),
    HKHubArchInstructionOperandM = (1 << 3),
    HKHubArchInstructionOperandRM = HKHubArchInstructionOperandR | HKHubArchInstructionOperandM,
    HKHubArchInstructionOperandRel = (1 << 4) | HKHubArchInstructionOperandI
} HKHubArchInstructionOperand;

typedef enum {
    HKHubArchInstructionMemoryOperationMask = 3,
    HKHubArchInstructionMemoryOperationSrc = (1 << 0),
    HKHubArchInstructionMemoryOperationDst = (1 << 2),
    
    HKHubArchInstructionMemoryOperationOp1 = 0,
    HKHubArchInstructionMemoryOperationOp2 = 2,
    HKHubArchInstructionMemoryOperationOp3 = 4,
    
    HKHubArchInstructionMemoryOperationNone = 0,
    HKHubArchInstructionMemoryOperationS = (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp1),
    HKHubArchInstructionMemoryOperationD = (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp1),
    HKHubArchInstructionMemoryOperationB = ((HKHubArchInstructionMemoryOperationSrc | HKHubArchInstructionMemoryOperationDst) << HKHubArchInstructionMemoryOperationOp1),
    
    HKHubArchInstructionMemoryOperationSS = (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp1) | (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp2),
    HKHubArchInstructionMemoryOperationSD = (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp1) | (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp2),
    HKHubArchInstructionMemoryOperationDS = (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp1) | (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp2),
    HKHubArchInstructionMemoryOperationDD = (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp1) | (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp2),
    
    HKHubArchInstructionMemoryOperationSSS = (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp1) | (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp2) | (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp3),
    HKHubArchInstructionMemoryOperationSSD = (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp1) | (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp2) | (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp3),
    HKHubArchInstructionMemoryOperationSDS = (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp1) | (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp2) | (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp3),
    HKHubArchInstructionMemoryOperationSDD = (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp1) | (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp2) | (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp3),
    HKHubArchInstructionMemoryOperationDSS = (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp1) | (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp2) | (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp3),
    HKHubArchInstructionMemoryOperationDSD = (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp1) | (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp2) | (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp3),
    HKHubArchInstructionMemoryOperationDDS = (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp1) | (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp2) | (HKHubArchInstructionMemoryOperationSrc << HKHubArchInstructionMemoryOperationOp3),
    HKHubArchInstructionMemoryOperationDDD = (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp1) | (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp2) | (HKHubArchInstructionMemoryOperationDst << HKHubArchInstructionMemoryOperationOp3)
} HKHubArchInstructionMemoryOperation;

typedef enum {
    HKHubArchInstructionRegisterSpecialPurpose = (1 << 1),
    HKHubArchInstructionRegisterGeneralPurpose = (1 << 2),
    HKHubArchInstructionRegisterGeneralPurposeIndexMask = 3,
    HKHubArchInstructionRegisterSpecialPurposeIndexMask = 1,
    
    HKHubArchInstructionRegisterR0 = 0 | HKHubArchInstructionRegisterGeneralPurpose,
    HKHubArchInstructionRegisterR1 = 1 | HKHubArchInstructionRegisterGeneralPurpose,
    HKHubArchInstructionRegisterR2 = 2 | HKHubArchInstructionRegisterGeneralPurpose,
    HKHubArchInstructionRegisterR3 = 3 | HKHubArchInstructionRegisterGeneralPurpose,
    
    HKHubArchInstructionRegisterFlags = 0 | HKHubArchInstructionRegisterSpecialPurpose,
    HKHubArchInstructionRegisterPC = 1 | HKHubArchInstructionRegisterSpecialPurpose
} HKHubArchInstructionRegister;

typedef enum {
    HKHubArchInstructionMemoryOffset,           //[0]
    HKHubArchInstructionMemoryRegister,         //[r0]
    HKHubArchInstructionMemoryRelativeOffset,   //[r0+0]
    HKHubArchInstructionMemoryRelativeRegister  //[r0+r0]
} HKHubArchInstructionMemory;

typedef struct {
    HKHubArchInstructionOperand type; //HKHubArchInstructionOperandNA or HKHubArchInstructionOperandI or HKHubArchInstructionOperandR or HKHubArchInstructionOperandM
    union {
        uint8_t value; //type = HKHubArchInstructionOperandI
        HKHubArchInstructionRegister reg; //type = HKHubArchInstructionOperandR
        struct {
            HKHubArchInstructionMemory type;
            union {
                uint8_t offset; //memory.type = HKHubArchInstructionMemoryOffset
                HKHubArchInstructionRegister reg; //memory.type = HKHubArchInstructionMemoryRegister
                struct {
                    uint8_t offset;
                    HKHubArchInstructionRegister reg;
                } relativeOffset; //memory.type = HKHubArchInstructionMemoryRelativeOffset
                HKHubArchInstructionRegister relativeReg[2]; //memory.type = HKHubArchInstructionMemoryRelativeRegister
            };
        } memory; //type = HKHubArchInstructionOperandM
    };
} HKHubArchInstructionOperandValue;

typedef struct {
    int8_t opcode; //-1 = invalid
    HKHubArchInstructionOperandValue operand[3];
} HKHubArchInstructionState;

typedef enum {
    HKHubArchInstructionOperationResultFailure,
    HKHubArchInstructionOperationResultSuccess,
    HKHubArchInstructionOperationResultMask = 1,
    
    HKHubArchInstructionOperationResultFlagSkipPC = (1 << 1),
    HKHubArchInstructionOperationResultFlagPipelineStall = (1 << 2)
} HKHubArchInstructionOperationResult;

/*!
 * @brief An instruction operation executed on a given processor with the given state.
 * @description Updates the processor's cycle count and any other state. Cycles are calculated for the operation
 *              alone.
 *
 * @param Processor The processor to execute the operation in.
 * @param State The operand state for the instruction.
 * @return The result of the operation.
 */
typedef HKHubArchInstructionOperationResult (*HKHubArchInstructionOperation)(HKHubArchProcessor Processor, const HKHubArchInstructionState *State);


/*!
 * @brief Encode an instruction AST.
 * @param Offset The offset the instruction should be located at.
 * @param Data The data to output the encoding to.
 * @param Command The instruction command.
 * @param Errors The collection of errors.
 * @param Labels The labels.
 * @param Defines The defined symbols.
 * @return The offset after the instruction.
 */
size_t HKHubArchInstructionEncode(size_t Offset, uint8_t Data[256], HKHubArchAssemblyASTNode *Command, CCOrderedCollection Errors, CCDictionary Labels, CCDictionary Defines);

/*!
 * @brief Decode an instruction from binary.
 * @param Offset The offset the instruction is located at.
 * @param Data The data that contains the instruction.
 * @param Decoded The pointer to store the decoded state. May be null.
 * @return The offset after the instruction.
 */
uint8_t HKHubArchInstructionDecode(uint8_t Offset, uint8_t Data[256], HKHubArchInstructionState *Decoded);

/*!
 * @brief Get the size of the encoded instruction state.
 * @param State The state for the instruction.
 * @return The size of the encoding.
 */
size_t HKHubArchInstructionSizeOfEncoding(const HKHubArchInstructionState *State);

/*!
 * @brief Get the memory operation of the instruction.
 * @param State The state for the instruction.
 * @return The memory operation.
 */
HKHubArchInstructionMemoryOperation HKHubArchInstructionGetMemoryOperation(const HKHubArchInstructionState *State);

/*!
 * @brief Disassemble an instruction from state.
 * @param State The pointer to decoded instruction state.
 * @return The disassembly or 0 on failure. Must be destroyed to free memory.
 */
CC_NEW CCString HKHubArchInstructionDisassemble(const HKHubArchInstructionState *State);

/*!
 * @brief Execute an instruction on a given processor with the given state.
 * @description Updates the processor's cycle count and any other state. Cycles are calculated for the operation
 *              alone.
 *
 * @param Processor The processor to execute the operation in.
 * @param State The state for the instruction to be executed.
 * @return The result type of the executed operation.
 */
HKHubArchInstructionOperationResult HKHubArchInstructionExecute(HKHubArchProcessor Processor, const HKHubArchInstructionState *State);

#endif
