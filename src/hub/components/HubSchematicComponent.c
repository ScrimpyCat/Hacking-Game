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
    if (CCExpressionGetType(Arg) == CCExpressionValueTypeList)
    {
        if (CCCollectionGetCount(CCExpressionGetList(Arg)) >= 2)
        {
            CCExpression NameExpr = *(CCExpression*)CCOrderedCollectionGetElementAtIndex(CCExpressionGetList(Arg), 0);
            if (CCExpressionGetType(NameExpr) == CCExpressionValueTypeAtom)
            {
                CCString Name = CCExpressionGetAtom(NameExpr);
                if (CCStringEqual(Name, CC_STRING("slots:")))
                {
                    CCEnumerator Enumerator;
                    CCCollectionGetEnumerator(CCExpressionGetList(Arg), &Enumerator);
                    
                    for (CCExpression *DeviceExpr; (DeviceExpr = CCCollectionEnumeratorNext(&Enumerator)); )
                    {
                        if ((CCExpressionGetType(*DeviceExpr) == CCExpressionValueTypeList) && (CCCollectionGetCount(CCExpressionGetList(*DeviceExpr)) == 2))
                        {
                            CCExpression SlotIndex = *(CCExpression*)CCOrderedCollectionGetElementAtIndex(CCExpressionGetList(*DeviceExpr), 0);
                            CCExpression Device = *(CCExpression*)CCOrderedCollectionGetElementAtIndex(CCExpressionGetList(*DeviceExpr), 1);
                            
                            if ((CCExpressionGetType(SlotIndex) == CCExpressionValueTypeInteger) && (CCExpressionGetType(Device) == CCEntityExpressionValueTypeEntity))
                            {
                                const uint32_t Index = CCExpressionGetInteger(SlotIndex);
                                
                                if (Index < HK_HUB_SCHEMATIC_SLOT_MAX) HKHubSchematicComponentSetSlot(Component, CCRetain(CCExpressionGetData(Device)), Index);
                                else
                                {
                                    CC_LOG_ERROR("Expect value for argument (slots:) to be a any number of 2 element lists of type (index:integer(0, %u), entity:custom)", HK_HUB_SCHEMATIC_SLOT_MAX - 1);
                                    break;
                                }
                            }
                            
                            else
                            {
                                CC_LOG_ERROR("Expect value for argument (slots:) to be a any number of 2 element lists of type (index:integer(0, %u), entity:custom)", HK_HUB_SCHEMATIC_SLOT_MAX - 1);
                                break;
                            }
                        }
                        
                        else
                        {
                            CC_LOG_ERROR("Expect value for argument (slots:) to be a any number of 2 element lists of type (index:integer(0, %u), entity:custom)", HK_HUB_SCHEMATIC_SLOT_MAX - 1);
                            break;
                        }
                    }
                    
                    return;
                }
                
                else if (CCStringEqual(Name, CC_STRING("wirings:")))
                {
                    CCEnumerator Enumerator;
                    CCCollectionGetEnumerator(CCExpressionGetList(Arg), &Enumerator);
                    
                    for (CCExpression *WiringExpr; (WiringExpr = CCCollectionEnumeratorNext(&Enumerator)); )
                    {
                        if ((CCExpressionGetType(*WiringExpr) == CCExpressionValueTypeList) && (CCCollectionGetCount(CCExpressionGetList(*WiringExpr)) == 3))
                        {
                            CCExpression SlotA = *(CCExpression*)CCOrderedCollectionGetElementAtIndex(CCExpressionGetList(*WiringExpr), 0);
                            CCExpression Path = *(CCExpression*)CCOrderedCollectionGetElementAtIndex(CCExpressionGetList(*WiringExpr), 1);
                            CCExpression SlotB = *(CCExpression*)CCOrderedCollectionGetElementAtIndex(CCExpressionGetList(*WiringExpr), 2);
                            
                            if ((CCExpressionGetType(SlotA) == CCExpressionValueTypeList) && (CCCollectionGetCount(CCExpressionGetList(SlotA)) == 2) &&
                                (CCExpressionGetType(SlotB) == CCExpressionValueTypeList) && (CCCollectionGetCount(CCExpressionGetList(SlotB)) == 2) &&
                                (CCExpressionGetType(Path) == CCExpressionValueTypeList) && (CCCollectionGetCount(CCExpressionGetList(Path)) < 16))
                            {
                                CCExpression SlotIndexAExpr = *(CCExpression*)CCOrderedCollectionGetElementAtIndex(CCExpressionGetList(SlotA), 0);
                                CCExpression SlotPinAExpr = *(CCExpression*)CCOrderedCollectionGetElementAtIndex(CCExpressionGetList(SlotA), 1);
                                
                                CCExpression SlotIndexBExpr = *(CCExpression*)CCOrderedCollectionGetElementAtIndex(CCExpressionGetList(SlotB), 0);
                                CCExpression SlotPinBExpr = *(CCExpression*)CCOrderedCollectionGetElementAtIndex(CCExpressionGetList(SlotB), 1);
                                
                                if ((CCExpressionGetType(SlotIndexAExpr) == CCExpressionValueTypeInteger) && (CCExpressionGetType(SlotPinAExpr) == CCExpressionValueTypeInteger) &&
                                    (CCExpressionGetType(SlotIndexBExpr) == CCExpressionValueTypeInteger) && (CCExpressionGetType(SlotPinBExpr) == CCExpressionValueTypeInteger))
                                {
                                    const uint32_t SlotIndexA = CCExpressionGetInteger(SlotIndexAExpr);
                                    const uint32_t SlotPinA = CCExpressionGetInteger(SlotPinAExpr);
                                    
                                    const uint32_t SlotIndexB = CCExpressionGetInteger(SlotIndexBExpr);
                                    const uint32_t SlotPinB = CCExpressionGetInteger(SlotPinBExpr);
                                    
                                    if ((SlotIndexA < HK_HUB_SCHEMATIC_SLOT_MAX) && (SlotPinA < HK_HUB_SCHEMATIC_SLOT_PIN_MAX) &&
                                        (SlotIndexB < HK_HUB_SCHEMATIC_SLOT_MAX) && (SlotPinB < HK_HUB_SCHEMATIC_SLOT_PIN_MAX))
                                    {
                                        HKHubSchematicWiring Wiring = {
                                            .slot = { SlotIndexA, SlotIndexB },
                                            .pin = { SlotPinA, SlotPinB }
                                        };
                                        
                                        memset(&Wiring.path, 0, sizeof(Wiring.path));
                                        
                                        _Bool ValidPath = TRUE;
                                        size_t Index = 0;
                                        CC_COLLECTION_FOREACH(CCExpression, Element, CCExpressionGetList(Path))
                                        {
                                            if (Index & 1)
                                            {
                                                if (CCExpressionGetType(Element) == CCExpressionValueTypeAtom)
                                                {
                                                    CCString Turn = CCExpressionGetAtom(Element);
                                                    if (CCStringEqual(Turn, CC_STRING(":left"))) Wiring.path.turns |= 0 << (8 - ((Index / 2) + 1));
                                                    else if (CCStringEqual(Turn, CC_STRING(":right"))) Wiring.path.turns |= 1 << (8 - ((Index / 2) + 1));
                                                    else
                                                    {
                                                        CC_LOG_ERROR("Expect value for argument (wirings:) to be a list of type ((slot:integer(0, %u), pin:integer(0, %u)), (move:integer, :left | :right, ...), (slot:integer(0, %u), pin:integer(0, %u)))", HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1, HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1);
                                                        break;
                                                    }
                                                }
                                                
                                                else
                                                {
                                                    CC_LOG_ERROR("Expect value for argument (wirings:) to be a list of type ((slot:integer(0, %u), pin:integer(0, %u)), (move:integer, :left | :right, ...), (slot:integer(0, %u), pin:integer(0, %u)))", HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1, HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1);
                                                    break;
                                                }
                                            }
                                            
                                            else
                                            {
                                                if (CCExpressionGetType(Element) == CCExpressionValueTypeInteger) Wiring.path.moves[Index / 2] = CCExpressionGetInteger(Element);
                                                else
                                                {
                                                    CC_LOG_ERROR("Expect value for argument (wirings:) to be a list of type ((slot:integer(0, %u), pin:integer(0, %u)), (move:integer, :left | :right, ...), (slot:integer(0, %u), pin:integer(0, %u)))", HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1, HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1);
                                                    break;
                                                }
                                            }
                                        }
                                        
                                        if (ValidPath) HKHubSchematicComponentSetWiring(Component, Wiring);
                                    }
                                    
                                    else
                                    {
                                        CC_LOG_ERROR("Expect value for argument (wirings:) to be a list of type ((slot:integer(0, %u), pin:integer(0, %u)), (move:integer, :left | :right, ...), (slot:integer(0, %u), pin:integer(0, %u)))", HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1, HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1);
                                        break;
                                    }
                                }
                                
                                else
                                {
                                    CC_LOG_ERROR("Expect value for argument (wirings:) to be a list of type ((slot:integer(0, %u), pin:integer(0, %u)), (move:integer, :left | :right, ...), (slot:integer(0, %u), pin:integer(0, %u)))", HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1, HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1);
                                    break;
                                }
                            }
                            
                            else
                            {
                                CC_LOG_ERROR("Expect value for argument (wirings:) to be a list of type ((slot:integer(0, %u), pin:integer(0, %u)), (move:integer, :left | :right, ...), (slot:integer(0, %u), pin:integer(0, %u)))", HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1, HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1);
                                break;
                            }
                        }
                        
                        else
                        {
                            CC_LOG_ERROR("Expect value for argument (wirings:) to be a list of type ((slot:integer(0, %u), pin:integer(0, %u)), (move:integer, :left | :right, ...), (slot:integer(0, %u), pin:integer(0, %u)))", HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1, HK_HUB_SCHEMATIC_SLOT_MAX - 1, HK_HUB_SCHEMATIC_SLOT_PIN_MAX - 1);
                            break;
                        }
                    }
                    
                    return;
                }
            }
        }
    }
    
    CCComponentExpressionDeserializeArgument(Component, Arg, Arguments, sizeof(Arguments) / sizeof(typeof(*Arguments)), Deferred);
}
