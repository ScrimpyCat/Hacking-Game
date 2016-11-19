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

#include "HubArchAssembly.h"

static void HKHubArchAssemblyASTNodeDestructor(void *Container, HKHubArchAssemblyASTNode *Node)
{
    if (Node->string) CCStringDestroy(Node->string);
    if (Node->childNodes) CCCollectionDestroy(Node->childNodes);
}

static void HKHubArchAssemblyResolveLiteralValue(HKHubArchAssemblyASTNode *Parent, const char *String, size_t Length, _Bool Hex, _Bool Sym, _Bool Dec, char Operator)
{
    HKHubArchAssemblyASTNode Node = {
        .type = HKHubArchAssemblyASTTypeUnknown,
        .line = Parent->line,
        .string = CCStringCreateWithSize(CC_STD_ALLOCATOR, CCStringHintCopy | CCStringEncodingASCII, String, Length),
        .childNodes = NULL
    };
    
    if ((Hex) || (Dec))
    {
        Node.type = HKHubArchAssemblyASTTypeInteger;
        Node.integer.value = (uint8_t)strtol(String, NULL, Dec ? 10 : 16);
    }
    
    else if (Sym)
    {
        Node.type = HKHubArchAssemblyASTTypeSymbol;
    }
    
    else
    {
        switch (Operator)
        {
            case '.':
                Node.type = HKHubArchAssemblyASTTypeOffset;
                break;
                
            case '+':
                Node.type = HKHubArchAssemblyASTTypePlus;
                break;
                
            case '-':
                Node.type = HKHubArchAssemblyASTTypeMinus;
                break;
        }
    }
    
    if (!Parent->childNodes) Parent->childNodes = CCCollectionCreate(CC_STD_ALLOCATOR, CCCollectionHintOrdered, sizeof(HKHubArchAssemblyASTNode), (CCCollectionElementDestructor)HKHubArchAssemblyASTNodeDestructor);
    
    CCOrderedCollectionAppendElement(Parent->childNodes, &Node);
}

static void HKHubArchAssemblyParseOperand(HKHubArchAssemblyASTNode *Node)
{
    CCOrderedCollection Strings = CCStringCreateBySeparatingOccurrencesOfGroupedStrings(Node->string, (CCString[2]){ CC_STRING(" "), CC_STRING("\t") }, 2);
    
    CC_COLLECTION_FOREACH(CCString, String, Strings)
    {
        if (CCStringGetLength(String))
        {
            CC_STRING_TEMP_BUFFER(Buffer, String)
            {
                const char *Start = Buffer;
                _Bool Hex = FALSE, Sym = FALSE, Dec = FALSE, CreateNode = FALSE;
                for (char c; (c = *(Buffer++)); )
                {
                    if (isalnum(c))
                    {
                        if (!CreateNode)
                        {
                            Hex = TRUE;
                            Sym = TRUE;
                            Dec = TRUE;
                        }
                        
                        CreateNode = TRUE;
                        
                        if ((!isxdigit(c)) && (c != 'x'))
                        {
                            Hex = FALSE;
                            Dec = FALSE;
                        }
                        
                        else if (!isdigit(c))
                        {
                            Dec = FALSE;
                        }
                    }
                    
                    else
                    {
                        if (CreateNode) HKHubArchAssemblyResolveLiteralValue(Node, Start, (Buffer - Start) - 1, Hex, Sym, Dec, 0);
                        
                        Start = Buffer;
                        Hex = FALSE;
                        Sym = FALSE;
                        Dec = FALSE;
                        CreateNode = FALSE;
                        
                        HKHubArchAssemblyResolveLiteralValue(Node, Buffer - 1, 1, Hex, Sym, Dec, c);
                    }
                }
                
                if (CreateNode) HKHubArchAssemblyResolveLiteralValue(Node, Start, Buffer - Start, Hex, Sym, Dec, 0);
            }
        }
    }
    
    CCCollectionDestroy(Strings);
}

static void HKHubArchAssemblyParseCommand(const char **Source, size_t *Line, HKHubArchAssemblyASTType ParentType, CCOrderedCollection AST)
{
    HKHubArchAssemblyASTType Type = HKHubArchAssemblyASTTypeInstruction;
    const char *Symbol = NULL;
    _Bool IsStr = FALSE, IsComment = FALSE, IsEscape = FALSE, IsCommand = ParentType == HKHubArchAssemblyASTTypeSource;
    for (char c = 0; (c = **Source); (*Source)++)
    {
        if (!Symbol) Symbol = *Source;
        
        if ((!IsComment) && (!IsStr) && (IsCommand) && (c == '.'))
        {
            Type = HKHubArchAssemblyASTTypeDirective;
        }
        
        else if ((!IsComment) && (!IsStr) && (IsCommand) && (c == ':'))
        {
            Type = HKHubArchAssemblyASTTypeLabel;
        }
        
        else if ((!IsComment) && (!IsStr) && (!IsCommand) && (c == '['))
        {
            HKHubArchAssemblyASTNode Node = {
                .type = HKHubArchAssemblyASTTypeMemory,
                .string = 0,
                .line = *Line,
                .childNodes = CCCollectionCreate(CC_STD_ALLOCATOR, CCCollectionHintOrdered, sizeof(HKHubArchAssemblyASTNode), (CCCollectionElementDestructor)HKHubArchAssemblyASTNodeDestructor)
            };
            
            (*Source)++;
            HKHubArchAssemblyParseCommand(Source, Line, Type, Node.childNodes);
            CCOrderedCollectionAppendElement(AST, &Node);
            
            Symbol = NULL;
            Type = HKHubArchAssemblyASTTypeInstruction;
            
            if (**Source == '\n') return;
        }
        
        else if ((!IsComment) && (!IsStr) && (((IsCommand) && (isspace(c))) || (c == '\n') || (c == ',') || ((c == '#') && (IsComment = TRUE)) || (c == ']')))
        {
            if (*Source != Symbol)
            {
                const size_t Length = (*Source - Symbol) - (Type != HKHubArchAssemblyASTTypeLabel ? 0 : 1);
                if (ParentType != HKHubArchAssemblyASTTypeSource) Type = HKHubArchAssemblyASTTypeOperand;
                
                HKHubArchAssemblyASTNode Node = {
                    .type = Type,
                    .string = CCStringCreateWithSize(CC_STD_ALLOCATOR, CCStringHintCopy | CCStringEncodingASCII, Symbol, Length),
                    .line = *Line,
                    .childNodes = NULL
                };
                
                if ((!IsComment) && (c != '\n') && ((Type == HKHubArchAssemblyASTTypeInstruction) || (Type == HKHubArchAssemblyASTTypeDirective)))
                {
                    Node.childNodes = CCCollectionCreate(CC_STD_ALLOCATOR, CCCollectionHintOrdered, sizeof(HKHubArchAssemblyASTNode), (CCCollectionElementDestructor)HKHubArchAssemblyASTNodeDestructor);
                    HKHubArchAssemblyParseCommand(Source, Line, Type, Node.childNodes);
                }
                
                else if (Type == HKHubArchAssemblyASTTypeOperand)
                {
                    HKHubArchAssemblyParseOperand(&Node);
                }
                
                CCOrderedCollectionAppendElement(AST, &Node);
                
                if (c == ']') return;
            }
            
            Symbol = NULL;
            Type = HKHubArchAssemblyASTTypeInstruction;
        }
        
        else if ((!IsComment) && (!IsEscape) && (c == '"'))
        {
            IsStr = !IsStr;
        }
        
        else if ((!IsStr) && (c == '#'))
        {
            IsComment = TRUE;
        }
        
        else if (IsEscape)
        {
            IsEscape = FALSE;
        }
        
        else if (c == '\\')
        {
            IsEscape = TRUE;
        }
        
        if (c == '\n')
        {
            Symbol = NULL;
            IsComment = FALSE;
            (*Line)++;
            
            if (ParentType != HKHubArchAssemblyASTTypeSource) return;
        }
    }
    
    if ((Symbol) && (*Symbol) && (!**Source))
    {
        const size_t Length = *Source - Symbol;
        if (ParentType != HKHubArchAssemblyASTTypeSource) Type = HKHubArchAssemblyASTTypeOperand;
        
        HKHubArchAssemblyASTNode Node = {
            .type = Type,
            .string = CCStringCreateWithSize(CC_STD_ALLOCATOR, CCStringHintCopy | CCStringEncodingASCII, Symbol, Length),
            .line = *Line,
            .childNodes = NULL
        };
        
        if (Type == HKHubArchAssemblyASTTypeOperand)
        {
            HKHubArchAssemblyParseOperand(&Node);
        }
        
        CCOrderedCollectionAppendElement(AST, &Node);
    }
}

CCOrderedCollection HKHubArchAssemblyParse(const char *Source)
{
    CCAssertLog(Source, "Source must not be null");
    
    CCOrderedCollection AST = CCCollectionCreate(CC_STD_ALLOCATOR, CCCollectionHintOrdered, sizeof(HKHubArchAssemblyASTNode), (CCCollectionElementDestructor)HKHubArchAssemblyASTNodeDestructor);
    
    HKHubArchAssemblyParseCommand(&Source, &(size_t){ 0 }, HKHubArchAssemblyASTTypeSource, AST);
    
    return AST;
}

#pragma mark - Directives
static size_t HKHubArchAssemblyCompileDirectiveDefine(size_t Offset, HKHubArchBinary Binary, HKHubArchAssemblyASTNode *Command, CCOrderedCollection Errors, CCDictionary Labels, CCDictionary Defines)
{
    if ((Command->childNodes) && (CCCollectionGetCount(Command->childNodes) == 2))
    {
        HKHubArchAssemblyASTNode *Name = CCOrderedCollectionGetElementAtIndex(Command->childNodes, 0);
        
        if (Name->type == HKHubArchAssemblyASTTypeSymbol)
        {
            HKHubArchAssemblyASTNode *Alias = CCOrderedCollectionGetElementAtIndex(Command->childNodes, 1);
            
            if (Alias->type == HKHubArchAssemblyASTTypeSymbol)
            {
                CCDictionarySetValue(Defines, &Name->string, &Alias->string);
            }
            
            else
            {
                CCOrderedCollectionAppendElement(Errors, &(HKHubArchAssemblyASTError){
                    .message = CC_STRING("operand 2 should be a symbol"),
                    .command = Command,
                    .operand = Alias,
                    .value = NULL
                });
            }
        }
        
        else
        {
            CCOrderedCollectionAppendElement(Errors, &(HKHubArchAssemblyASTError){
                .message = CC_STRING("operand 1 should be a symbol"),
                .command = Command,
                .operand = Name,
                .value = NULL
            });
        }
    }
    
    else
    {
        CCOrderedCollectionAppendElement(Errors, &(HKHubArchAssemblyASTError){
            .message = CC_STRING("expects 2 operands, the name of the macro and the symbol it evaluates to"),
            .command = Command,
            .operand = NULL,
            .value = NULL
        });
    }
    
    return Offset;
}

#pragma mark -

static struct {
    CCString mnemonic;
    size_t (*compile)(size_t, HKHubArchBinary, HKHubArchAssemblyASTNode *, CCOrderedCollection, CCDictionary, CCDictionary);
} Directives[] = {
    { CC_STRING(".define"), HKHubArchAssemblyCompileDirectiveDefine }
};

static void HKHubArchAssemblyASTErrorDestructor(void *Container, HKHubArchAssemblyASTError *Error)
{
    if (Error->message) CCStringDestroy(Error->message);
}

HKHubArchBinary HKHubArchAssemblyCreateBinary(CCAllocatorType Allocator, CCOrderedCollection AST, CCOrderedCollection *Errors)
{
    CCAssertLog(AST, "AST must not be null");
    
    HKHubArchBinary Binary = HKHubArchBinaryCreate(Allocator);
    
    CCDictionary Labels = CCDictionaryCreate(CC_STD_ALLOCATOR, CCDictionaryHintSizeSmall, sizeof(CCString), sizeof(uint8_t), &(CCDictionaryCallbacks){
        .getHash = CCStringHasherForDictionary,
        .compareKeys = CCStringComparatorForDictionary
    });
    
    CCDictionary Defines = CCDictionaryCreate(CC_STD_ALLOCATOR, CCDictionaryHintSizeSmall, sizeof(CCString), sizeof(HKHubArchAssemblyASTNode), &(CCDictionaryCallbacks){
        .getHash = CCStringHasherForDictionary,
        .compareKeys = CCStringComparatorForDictionary,
        .valueDestructor = (CCDictionaryElementDestructor)HKHubArchAssemblyASTNodeDestructor
    });
    
    CCOrderedCollection Err = CCCollectionCreate(CC_STD_ALLOCATOR, CCCollectionHintOrdered, sizeof(HKHubArchAssemblyASTError), (CCCollectionElementDestructor)HKHubArchAssemblyASTErrorDestructor);
    
    size_t Offset = 0;
    CC_COLLECTION_FOREACH_PTR(HKHubArchAssemblyASTNode, Command, AST)
    {
        switch (Command->type)
        {
            case HKHubArchAssemblyASTTypeLabel:
                CCDictionarySetValue(Labels, &Command->string, &Offset);
                break;
                
            case HKHubArchAssemblyASTTypeInstruction:
                break;
                
            case HKHubArchAssemblyASTTypeDirective:
                for (size_t Loop = 0; Loop < sizeof(Directives) / sizeof(typeof(*Directives)); Loop++) //TODO: make dictionary
                {
                    if (CCStringEqual(Directives[Loop].mnemonic, Command->string))
                    {
                        Offset = Directives[Loop].compile(Offset, Binary, Command, Err, Labels, Defines);
                        break;
                    }
                }
                break;
                
            default:
                //error
                break;
        }
    }
    
    CCDictionaryDestroy(Defines);
    CCDictionaryDestroy(Labels);
    
    if (CCCollectionGetCount(Err))
    {
        HKHubArchBinaryDestroy(Binary);
        Binary = NULL;
        
        if (Errors) *Errors = Err;
        else CCCollectionDestroy(Err);
    }
    
    else CCCollectionDestroy(Err);
    
    return Binary;
}

static void HKHubArchAssemblyPrintASTNodes(CCOrderedCollection AST)
{
    if (!AST) return;
    
    size_t Line = 0;
    _Bool First = TRUE;
    CC_COLLECTION_FOREACH_PTR(HKHubArchAssemblyASTNode, Node, AST)
    {
        if (Line != Node->line)
        {
            if (!First) printf("\n");
            
            Line = Node->line;
            First = TRUE;
        }
        
        static const char *Types[] = {
            "source",
            "unknown",
            "instruction",
            "directive",
            "label",
            "integer",
            "register",
            "memory",
            "operand",
            "symbol",
            "plus",
            "minus",
            "offset"
        };
        
        if (!First) printf(", ");
        
        printf("%s", Types[Node->type]);
        
        if (Node->string)
        {
            CC_STRING_TEMP_BUFFER(Buffer, Node->string)
            {
                printf("<%s>", Buffer);
            }
        }
        
        printf("(");
        
        HKHubArchAssemblyPrintASTNodes(Node->childNodes);
        
        printf(")");
        
        First = FALSE;
    }
}

void HKHubArchAssemblyPrintAST(CCOrderedCollection AST)
{
    HKHubArchAssemblyPrintASTNodes(AST);
    printf("\n");
}