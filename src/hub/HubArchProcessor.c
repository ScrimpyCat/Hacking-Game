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

#include "HubArchProcessor.h"
#include "HubArchInstruction.h"

const double HKHubArchProcessorHertz = 400.0;
const size_t HKHubArchProcessorSpeedMemoryRead = 1;
const size_t HKHubArchProcessorSpeedMemoryWrite = 1;
const size_t HKHubArchProcessorSpeedPortTransmission = 4;

static uintmax_t HKHubArchProcessorPortHasher(HKHubArchPortID *Key)
{
    return *Key;
}

static void HKHubArchProcessorDestructor(HKHubArchProcessor Processor)
{
    CC_DICTIONARY_FOREACH_VALUE(HKHubArchPortConnection, Connection, Processor->ports)
    {
        for (int Loop = 0; Loop < 2; Loop++)
        {
            if (Connection->port[Loop].device == Processor) Connection->port[Loop].disconnect = NULL;
        }
        
        HKHubArchPortConnectionDisconnect(Connection);
    }
    
    CCDictionaryDestroy(Processor->ports);
}

HKHubArchProcessor HKHubArchProcessorCreate(CCAllocatorType Allocator, HKHubArchBinary Binary)
{
    CCAssertLog(Binary, "Binary must not be null");
    
    HKHubArchProcessor Processor = CCMalloc(Allocator, sizeof(HKHubArchProcessorInfo), NULL, CC_DEFAULT_ERROR_CALLBACK);
    
    if (Processor)
    {
        Processor->ports = CCDictionaryCreate(Allocator, CCDictionaryHintHeavyFinding, sizeof(HKHubArchPortID), sizeof(HKHubArchPortConnection), &(CCDictionaryCallbacks){
            .getHash = (CCDictionaryKeyHasher)HKHubArchProcessorPortHasher,
            .valueDestructor = HKHubArchPortConnectionDestructorForDictionary
        });
        
        Processor->message.type = HKHubArchProcessorMessageClear;
        Processor->state.r[0] = 0;
        Processor->state.r[1] = 0;
        Processor->state.r[2] = 0;
        Processor->state.r[3] = 0;
        Processor->state.pc = Binary->entrypoint;
        Processor->state.flags = 0;
        Processor->cycles = 0;
        Processor->complete = FALSE;
        
        memcpy(Processor->memory, Binary->data, sizeof(Processor->memory));
        
        CCMemorySetDestructor(Processor, (CCMemoryDestructorCallback)HKHubArchProcessorDestructor);
    }
    
    else CC_LOG_ERROR("Failed to create processor, due to allocation failure (%zu)", sizeof(HKHubArchProcessorInfo));
    
    return Processor;
}

void HKHubArchProcessorDestroy(HKHubArchProcessor Processor)
{
    CCAssertLog(Processor, "Processor must not be null");
    
    CCFree(Processor);
}

void HKHubArchProcessorReset(HKHubArchProcessor Processor, HKHubArchBinary Binary)
{
    CCAssertLog(Processor, "Processor must not be null");
    CCAssertLog(Binary, "Binary must not be null");
    
    memcpy(Processor->memory, Binary->data, sizeof(Processor->memory));
    
    Processor->message.type = HKHubArchProcessorMessageClear;
    Processor->state.r[0] = 0;
    Processor->state.r[1] = 0;
    Processor->state.r[2] = 0;
    Processor->state.r[3] = 0;
    Processor->state.pc = Binary->entrypoint;
    Processor->state.flags = 0;
    Processor->complete = FALSE;
}

void HKHubArchProcessorSetCycles(HKHubArchProcessor Processor, int64_t Cycles)
{
    CCAssertLog(Processor, "Processor must not be null");
    
    Processor->cycles = Cycles;
    Processor->complete = FALSE;
}

void HKHubArchProcessorAddProcessingTime(HKHubArchProcessor Processor, double Seconds)
{
    CCAssertLog(Processor, "Processor must not be null");
    
    Processor->cycles += Seconds * HKHubArchProcessorHertz;
    Processor->complete = FALSE;
}

static void HKHubArchProcessorDisconnectPort(HKHubArchProcessor Processor, HKHubArchPortID Port)
{
    CCDictionaryRemoveValue(Processor->ports, &Port);
}

static HKHubArchPortResponse HKHubArchProcessorPortSend(HKHubArchPortConnection Connection, HKHubArchProcessor Device, HKHubArchPortID Port, HKHubArchPortMessage *Message, HKHubArchPortDevice ConnectedDevice, int64_t Timestamp)
{
    if (Device->cycles < Timestamp) return HKHubArchPortResponseTimeout;
    
    if (Device->message.type == HKHubArchProcessorMessageSend)
    {
        if (Device->message.port != Port) return HKHubArchPortResponseRetry;
        
        *Message = Device->message.data;
        
        Device->message.type = HKHubArchProcessorMessageComplete;
        
        return HKHubArchPortResponseSuccess;
    }
    
    return HKHubArchPortResponseRetry;
}

static HKHubArchPortResponse HKHubArchProcessorPortReceive(HKHubArchPortConnection Connection, HKHubArchProcessor Device, HKHubArchPortID Port, HKHubArchPortMessage *Message, HKHubArchPortDevice ConnectedDevice, int64_t Timestamp)
{
    if (Device->cycles < Timestamp) return HKHubArchPortResponseTimeout;
    
    if (Device->message.type == HKHubArchProcessorMessageReceive)
    {
        if (Device->message.port != Port) return HKHubArchPortResponseRetry;
        
        uint8_t Offset = Device->message.data.offset;
        for (uint8_t Size = Message->size; Size--; )
        {
            Device->memory[Offset + Size] = Message->memory[Message->offset + Size];
        }
        
        Device->message.type = HKHubArchProcessorMessageComplete;
        
        return HKHubArchPortResponseSuccess;
    }
    
    return HKHubArchPortResponseRetry;
}

HKHubArchPort HKHubArchProcessorGetPort(HKHubArchProcessor Processor, HKHubArchPortID Port)
{
    CCAssertLog(Processor, "Processor must not be null");
    
    return (HKHubArchPort){
        .device = Processor,
        .id = Port,
        .disconnect = NULL,
        .sender = (HKHubArchPortTransmit)HKHubArchProcessorPortSend,
        .receiver = (HKHubArchPortTransmit)HKHubArchProcessorPortReceive
    };
}

static inline void HKHubArchProcessorSetConnectionDisconnectCallback(HKHubArchProcessor Processor, HKHubArchPortID Port, HKHubArchPortConnection Connection, HKHubArchPortDisconnect Disconnect)
{
    for (int Loop = 0; Loop < 2; Loop++)
    {
        if ((Connection->port[Loop].device == Processor) && (Connection->port[Loop].id == Port))
        {
            Connection->port[Loop].disconnect = Disconnect;
        }
    }
}

void HKHubArchProcessorConnect(HKHubArchProcessor Processor, HKHubArchPortID Port, HKHubArchPortConnection Connection)
{
    CCAssertLog(Processor, "Processor must not be null");
    CCAssertLog(Connection, "Connection must not be null");
    
    CCDictionaryEntry Entry = CCDictionaryEntryForKey(Processor->ports, &Port);
    
    if (CCDictionaryEntryIsInitialized(Processor->ports, Entry))
    {
        HKHubArchPortConnection OldConnection = *(HKHubArchPortConnection*)CCDictionaryGetEntry(Processor->ports, Entry);
        HKHubArchProcessorSetConnectionDisconnectCallback(Processor, Port, OldConnection, NULL);
        HKHubArchPortConnectionDisconnect(OldConnection);
    }
    
    HKHubArchProcessorSetConnectionDisconnectCallback(Processor, Port, Connection, (HKHubArchPortDisconnect)HKHubArchProcessorDisconnectPort);
    
    CCDictionarySetEntry(Processor->ports, Entry, &(HKHubArchPortConnection){ CCRetain(Connection) });
}

void HKHubArchProcessorDisconnect(HKHubArchProcessor Processor, HKHubArchPortID Port)
{
    CCAssertLog(Processor, "Processor must not be null");
    
    HKHubArchPortConnection *Connection = CCDictionaryGetValue(Processor->ports, &Port);
    
    if (Connection) HKHubArchPortConnectionDisconnect(*Connection);
}

void HKHubArchProcessorRun(HKHubArchProcessor Processor)
{
    CCAssertLog(Processor, "Processor must not be null");
    
    while ((!Processor->complete) && !(Processor->complete = !(Processor->cycles > 0)))
    {
        HKHubArchInstructionState Instruction;
        uint8_t NextPC = HKHubArchInstructionDecode(Processor->state.pc, Processor->memory, &Instruction);
        
        if (Instruction.opcode != -1)
        {
            int64_t Cycles = (NextPC - Processor->state.pc) * HKHubArchProcessorSpeedMemoryRead;
            if (Cycles < Processor->cycles)
            {
                Processor->cycles -= Cycles;
                
                HKHubArchInstructionOperationResult Result;
                if (((Result = HKHubArchInstructionExecute(Processor, &Instruction)) & HKHubArchInstructionOperationResultMask) == HKHubArchInstructionOperationResultFailure)
                {
                    Processor->cycles += Cycles;
                    
                    if (Result & HKHubArchInstructionOperationResultFlagPipelineStall) break;
                    
                    Processor->complete = TRUE;
                }
                
                else if (!(Result & HKHubArchInstructionOperationResultFlagSkipPC)) Processor->state.pc = NextPC;
            }
            
            else Processor->complete = TRUE;
        }
        
        else Processor->complete = TRUE;
    }
}
