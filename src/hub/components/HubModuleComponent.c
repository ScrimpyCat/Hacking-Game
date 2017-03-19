/*
 *  Copyright (c) 2017, Stefan Johnson
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

#include "HubModuleComponent.h"
#include "HubModuleKeyboardComponent.h"

const char * const HKHubModuleComponentName = "module";

static void HKHubModuleComponentMessageHandler(CCComponent Component, CCMessage *Message);

static struct {
    CCString module;
    const CCComponentExpressionDescriptor *descriptor;
} HKHubModuleComponentDescriptors[] = {
    { .module = CC_STRING("keyboard-module"), .descriptor = &HKHubModuleKeyboardComponentDescriptor }
};

void HKHubModuleComponentRegister(void)
{
    CCComponentRegister(HK_HUB_MODULE_COMPONENT_ID, HKHubModuleComponentName, CC_STD_ALLOCATOR, sizeof(HKHubModuleComponentClass), HKHubModuleComponentInitialize, HKHubModuleComponentMessageHandler, HKHubModuleComponentDeallocate);
    
    for (size_t Loop = 0; Loop < sizeof(HKHubModuleComponentDescriptors) / sizeof(typeof(*HKHubModuleComponentDescriptors)); Loop++)
    {
        CCComponentExpressionRegister(HKHubModuleComponentDescriptors[Loop].module, HKHubModuleComponentDescriptors[Loop].descriptor, TRUE);
    }
}

void HKHubModuleComponentDeregister(void)
{
    CCComponentDeregister(HK_HUB_MODULE_COMPONENT_ID);
}

static void HKHubModuleComponentMessageHandler(CCComponent Component, CCMessage *Message)
{
    CCComponentMessageHandler MessageHandler = HKHubModuleComponentGetMessageHandler(Component);
    if (MessageHandler) MessageHandler(Component, Message);
}
