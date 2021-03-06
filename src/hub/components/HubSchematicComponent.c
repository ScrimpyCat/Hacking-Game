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

#include "HubSchematicComponent.h"

const CCString HKHubSchematicComponentName = CC_STRING("schematic");

static const CCComponentExpressionDescriptor HKHubSchematicComponentDescriptor = {
    .id = HK_HUB_SCHEMATIC_COMPONENT_ID,
    .initialize = NULL,
    .deserialize = HKHubSchematicComponentDeserializer,
    .serialize = NULL
};

void HKHubSchematicComponentRegister(void)
{
    CCComponentRegister(HK_HUB_SCHEMATIC_COMPONENT_ID, HKHubSchematicComponentName, CC_STD_ALLOCATOR, sizeof(HKHubSchematicComponentClass), HKHubSchematicComponentInitialize, NULL, HKHubSchematicComponentDeallocate);
    
    CCComponentExpressionRegister(CC_STRING("schematic"), &HKHubSchematicComponentDescriptor, TRUE);
}

void HKHubSchematicComponentDeregister(void)
{
    CCComponentDeregister(HK_HUB_SCHEMATIC_COMPONENT_ID);
}

static CCComponentExpressionArgumentDeserializer Arguments[] = {
    { .name = CC_STRING("editable:"), .serializedType = CCExpressionValueTypeUnspecified, .setterType = CCComponentExpressionArgumentTypeBool, .setter = (CCComponentExpressionSetter)HKHubSchematicComponentSetIsEditable },
    { .name = CC_STRING("debuggable:"), .serializedType = CCExpressionValueTypeUnspecified, .setterType = CCComponentExpressionArgumentTypeBool, .setter = (CCComponentExpressionSetter)HKHubSchematicComponentSetIsDebuggable }
};

void HKHubSchematicComponentDeserializer(CCComponent Component, CCExpression Arg, _Bool Deferred)
{
    CCComponentExpressionDeserializeArgument(Component, Arg, Arguments, sizeof(Arguments) / sizeof(typeof(*Arguments)), Deferred);
}
