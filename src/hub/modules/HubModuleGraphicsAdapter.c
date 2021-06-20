/*
 *  Copyright (c) 2021, Stefan Johnson
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

#include "HubModuleGraphicsAdapter.h"

#define CC_TYPE_HKHubModuleGraphicsAdapterCell(...) HKHubModuleGraphicsAdapterCell
#define CC_TYPE_1_HKHubModuleGraphicsAdapterCell CC_TYPE_HKHubModuleGraphicsAdapterCell,

#define CC_PRESERVE_CC_TYPE_HKHubModuleGraphicsAdapterCell CC_TYPE_HKHubModuleGraphicsAdapterCell

#define CC_TYPE_DECL_HKHubModuleGraphicsAdapterCell(...) HKHubModuleGraphicsAdapterCell, __VA_ARGS__
#define CC_TYPE_DECL_1_HKHubModuleGraphicsAdapterCell CC_TYPE_DECL_HKHubModuleGraphicsAdapterCell,

#define CC_MANGLE_TYPE_1_HKHubModuleGraphicsAdapterCell HKHubModuleGraphicsAdapterCell

typedef struct {
    uint8_t x, y, width, height;
} HKHubModuleGraphicsAdapterViewport;

typedef CC_FLAG_ENUM(HKHubModuleGraphicsAdapterCell, uint32_t) {
    HKHubModuleGraphicsAdapterCellModeMask = 0x80000000,
    HKHubModuleGraphicsAdapterCellModeBitmap = 0,
    HKHubModuleGraphicsAdapterCellModeReference = 0x80000000,
    
    //p = palette page
    HKHubModuleGraphicsAdapterCellPalettePageIndex = 28,
    HKHubModuleGraphicsAdapterCellPalettePageMask = (7 << HKHubModuleGraphicsAdapterCellPalettePageIndex),
    
    //b = bold
    HKHubModuleGraphicsAdapterCellBoldFlag = (1 << 27),
    
    //a = animation offset (current frame + anim offset = anim index)
    HKHubModuleGraphicsAdapterCellAnimationOffsetIndex = 24,
    HKHubModuleGraphicsAdapterCellAnimationOffsetMask = (7 << HKHubModuleGraphicsAdapterCellAnimationOffsetIndex),
    
    //i = glyph index
    HKHubModuleGraphicsAdapterCellGlyphIndexMask = 0x1fffff,
    
    //r = reference layer
    HKHubModuleGraphicsAdapterCellReferenceLayerIndex = 16,
    HKHubModuleGraphicsAdapterCellReferenceLayerMask = (7 << HKHubModuleGraphicsAdapterCellReferenceLayerIndex),
    
    //x = x position
    HKHubModuleGraphicsAdapterCellPositionXIndex = 0,
    HKHubModuleGraphicsAdapterCellPositionXMask = (0xff << HKHubModuleGraphicsAdapterCellPositionXIndex),
    
    //y = y position
    HKHubModuleGraphicsAdapterCellPositionYIndex = 8,
    HKHubModuleGraphicsAdapterCellPositionYMask = (0xff << HKHubModuleGraphicsAdapterCellPositionYIndex),
    
    CC_RESERVED_BITS(HKHubModuleGraphicsAdapterCell, 0, 32)
};

typedef struct {
    HKHubModuleGraphicsAdapterViewport viewports[256];
    uint8_t glyphs[HK_HUB_MODULE_GRAPHICS_ADAPTER_GLYPH_BUFFER];
    uint8_t palettes[HK_HUB_MODULE_GRAPHICS_ADAPTER_PALETTE_PAGE_COUNT][256];
    uint8_t layers[HK_HUB_MODULE_GRAPHICS_ADAPTER_LAYER_COUNT][HK_HUB_MODULE_GRAPHICS_ADAPTER_LAYER_WIDTH * HK_HUB_MODULE_GRAPHICS_ADAPTER_LAYER_HEIGHT * sizeof(HKHubModuleGraphicsAdapterCell)];
} HKHubModuleGraphicsAdapterMemory;

typedef struct {
    uint8_t frame;
    HKHubModuleGraphicsAdapterMemory memory;
} HKHubModuleGraphicsAdapterState;

#define Tbuffer PTYPE(HKHubModuleGraphicsAdapterCell *)
#include <CommonC/Memory.h>

static CC_FORCE_INLINE HKHubModuleGraphicsAdapterCell HKHubModuleGraphicsAdapterCellMode(HKHubModuleGraphicsAdapterCell Cell)
{
    return Cell & HKHubModuleGraphicsAdapterCellModeMask;
}

static CC_FORCE_INLINE uint8_t HKHubModuleGraphicsAdapterCellGetPalettePage(HKHubModuleGraphicsAdapterCell Cell)
{
    return (Cell & HKHubModuleGraphicsAdapterCellPalettePageMask) >> HKHubModuleGraphicsAdapterCellPalettePageIndex;
}

static CC_FORCE_INLINE _Bool HKHubModuleGraphicsAdapterCellIsBold(HKHubModuleGraphicsAdapterCell Cell)
{
    return Cell & HKHubModuleGraphicsAdapterCellBoldFlag;
}

static CC_FORCE_INLINE uint8_t HKHubModuleGraphicsAdapterCellGetAnimationOffset(HKHubModuleGraphicsAdapterCell Cell)
{
    return (Cell & HKHubModuleGraphicsAdapterCellAnimationOffsetMask) >> HKHubModuleGraphicsAdapterCellAnimationOffsetIndex;
}

static CC_FORCE_INLINE uint32_t HKHubModuleGraphicsAdapterCellGetGlyphIndex(HKHubModuleGraphicsAdapterCell Cell)
{
    return Cell & HKHubModuleGraphicsAdapterCellGlyphIndexMask;
}

static CC_FORCE_INLINE uint8_t HKHubModuleGraphicsAdapterCellGetReferenceLayer(HKHubModuleGraphicsAdapterCell Cell)
{
    return (Cell & HKHubModuleGraphicsAdapterCellReferenceLayerMask) >> HKHubModuleGraphicsAdapterCellReferenceLayerIndex;
}

static CC_FORCE_INLINE uint8_t HKHubModuleGraphicsAdapterCellGetX(HKHubModuleGraphicsAdapterCell Cell)
{
    return (Cell & HKHubModuleGraphicsAdapterCellPositionXMask) >> HKHubModuleGraphicsAdapterCellPositionXIndex;
}

static CC_FORCE_INLINE uint8_t HKHubModuleGraphicsAdapterCellGetY(HKHubModuleGraphicsAdapterCell Cell)
{
    return (Cell & HKHubModuleGraphicsAdapterCellPositionYMask) >> HKHubModuleGraphicsAdapterCellPositionYIndex;
}

static int32_t HKHubModuleGraphicsAdapterCellIndex(HKHubModuleGraphicsAdapterMemory *Memory, size_t Layer, size_t X, size_t Y, HKHubModuleGraphicsAdapterCell *Attributes)
{
    //TODO: Handle infinite recursion case
    
    CCAssertLog(Layer < HK_HUB_MODULE_GRAPHICS_ADAPTER_LAYER_COUNT, "Layer must not exceed layer count");
    CCAssertLog(X < HK_HUB_MODULE_GRAPHICS_ADAPTER_LAYER_WIDTH, "X must not exceed layer width");
    CCAssertLog(Y < HK_HUB_MODULE_GRAPHICS_ADAPTER_LAYER_HEIGHT, "Y must not exceed layer height");
    
    HKHubModuleGraphicsAdapterCell Glyph;
    
    CCMemoryReadBig(&Memory->layers[Layer][(X + (Y * HK_HUB_MODULE_GRAPHICS_ADAPTER_LAYER_WIDTH)) * sizeof(Glyph)], sizeof(Glyph), 0, sizeof(Glyph), &Glyph);
    
    if (Attributes) *Attributes = Glyph;
    
    switch (HKHubModuleGraphicsAdapterCellMode(Glyph))
    {
        case HKHubModuleGraphicsAdapterCellModeBitmap:
            return HKHubModuleGraphicsAdapterCellGetGlyphIndex(Glyph);
            
        case HKHubModuleGraphicsAdapterCellModeReference:
            return HKHubModuleGraphicsAdapterCellIndex(Memory, HKHubModuleGraphicsAdapterCellGetReferenceLayer(Glyph), HKHubModuleGraphicsAdapterCellGetX(Glyph), HKHubModuleGraphicsAdapterCellGetY(Glyph), NULL);
            
        default:
            break;
    }
    
    return -1;
}

const uint8_t *HKHubModuleGraphicsAdapterGetGlyphBitmap(HKHubModule Adapter, CCChar Character, uint8_t *Width, uint8_t *Height, uint8_t *PaletteSize)
{
    HKHubModuleGraphicsAdapterState *State = Adapter->internal;
    HKHubModuleGraphicsAdapterMemory *Memory = &State->memory;
    const uint8_t Frame = State->frame;
    //TODO: Maintain a lookup for dynamic glyph bitmaps
    for (size_t Offset = 0; (Offset + 3) < HK_HUB_MODULE_GRAPHICS_ADAPTER_GLYPH_BUFFER; )
    {
        uint8_t BitmapWidth = (Memory->glyphs[Offset] >> 4);
        uint8_t BitmapHeight = (Memory->glyphs[Offset] & 0xf);
        uint8_t BitmapPalette = (Memory->glyphs[Offset + 1] >> 5);
        uint32_t Index = ((Memory->glyphs[Offset + 1] << 16) | (Memory->glyphs[Offset + 2] << 8) | Memory->glyphs[Offset + 3]) & HKHubModuleGraphicsAdapterCellGlyphIndexMask;
        const size_t BitmapSize = HK_HUB_MODULE_GRAPHICS_ADAPTER_GLYPH_BITMAP_SIZE(BitmapWidth + 1, BitmapHeight + 1, BitmapPalette + 1);
        
        Offset += 4;
        
        if (Index == Character)
        {
            *Width = BitmapWidth;
            *Height = BitmapHeight;
            *PaletteSize = BitmapPalette;
            
            for (uint8_t Frames = 0, Animation = 0; (Animation != 0xff) && ((Offset + BitmapSize) < HK_HUB_MODULE_GRAPHICS_ADAPTER_GLYPH_BUFFER); Animation |= Frames)
            {
                Frames = Memory->glyphs[Offset++];
                
                if (Frames & Frame) return &Memory->glyphs[Offset];
                
                Offset += BitmapSize;
            }
            
            return NULL;
        }
        
        for (uint8_t Animation = 0; (Animation != 0xff) && ((Offset + BitmapSize) < HK_HUB_MODULE_GRAPHICS_ADAPTER_GLYPH_BUFFER); )
        {
            Animation |= Memory->glyphs[Offset++];
            Offset += BitmapSize;
        }
    }
    
    return HKHubModuleGraphicsAdapterStaticGlyphGet(Character, Width, Height, PaletteSize);
}

CC_ARRAY_DECLARE(uint32_t);

static CCArray(uint32_t) HKHubModuleGraphicsAdapterGlyphIndexes = CC_STATIC_ARRAY(sizeof(uint32_t), HKHubModuleGraphicsAdapterCellGlyphIndexMask + 1, 0, CC_STATIC_ALLOC_BSS(uint32_t[HKHubModuleGraphicsAdapterCellGlyphIndexMask + 1]));

#define HK_HUB_MODULE_GRAPHICS_ADAPTER_HALF_WIDTH_GLYPH_BITMAPS ((0 << 7) | (1 << 3) | 0)
#define HK_HUB_MODULE_GRAPHICS_ADAPTER_FULL_WIDTH_GLYPH_BITMAPS ((1 << 7) | (1 << 3) | 0)

#define HK_HUB_MODULE_GRAPHICS_ADAPTER_HALF_WIDTH_GLYPH_BITMAP_SIZE HK_HUB_MODULE_GRAPHICS_ADAPTER_GLYPH_BITMAP_SIZE(1, 2, 1)
#define HK_HUB_MODULE_GRAPHICS_ADAPTER_FULL_WIDTH_GLYPH_BITMAP_SIZE HK_HUB_MODULE_GRAPHICS_ADAPTER_GLYPH_BITMAP_SIZE(2, 2, 1)

static CCArray HKHubModuleGraphicsAdapterGlyphBitmaps[16 * 16 * 8][2] = {
    [0] = { CC_STATIC_ARRAY(HK_HUB_MODULE_GRAPHICS_ADAPTER_GLYPH_BITMAP_SIZE(1, 1, 1), 1, 1, CC_STATIC_ALLOC_BSS(uint8_t[HK_HUB_MODULE_GRAPHICS_ADAPTER_GLYPH_BITMAP_SIZE(1, 1, 1)])), NULL },
    [HK_HUB_MODULE_GRAPHICS_ADAPTER_HALF_WIDTH_GLYPH_BITMAPS] = { CC_STATIC_ARRAY(HK_HUB_MODULE_GRAPHICS_ADAPTER_HALF_WIDTH_GLYPH_BITMAP_SIZE, 2040, 0, CC_STATIC_ALLOC_BSS(uint8_t[HK_HUB_MODULE_GRAPHICS_ADAPTER_HALF_WIDTH_GLYPH_BITMAP_SIZE])), NULL },
    [HK_HUB_MODULE_GRAPHICS_ADAPTER_FULL_WIDTH_GLYPH_BITMAPS] = { CC_STATIC_ARRAY(HK_HUB_MODULE_GRAPHICS_ADAPTER_FULL_WIDTH_GLYPH_BITMAP_SIZE, 151660, 0, CC_STATIC_ALLOC_BSS(uint8_t[HK_HUB_MODULE_GRAPHICS_ADAPTER_FULL_WIDTH_GLYPH_BITMAP_SIZE])), NULL }
};

void HKHubModuleGraphicsAdapterStaticGlyphInit(void)
{
    FSPath Path = FSPathCopy(HKAssetPath);
    FSPathAppendComponent(Path, FSPathComponentCreate(FSPathComponentTypeDirectory, "graphics-adapter"));
    
    CCCollection Paths = CCCollectionCreate(CC_STD_ALLOCATOR, CCCollectionHintSizeSmall, sizeof(FSPath), FSPathDestructorForCollection);
    CCCollectionInsertElement(Paths, &(FSPath){ FSPathCreate(".glyphset") });
    CCOrderedCollection GlyphSets = FSManagerGetContentsAtPath(Path, Paths, FSMatchDefault);
    
    if (GlyphSets)
    {
        CC_COLLECTION_FOREACH(FSPath, GlyphSet, GlyphSets)
        {
            const size_t ComponentCount = FSPathGetComponentCount(GlyphSet);
            if (ComponentCount >= 4)
            {
                FSPathComponent PaletteComponent = FSPathGetComponentAtIndex(GlyphSet, ComponentCount - 2);
                FSPathComponent HeightComponent = FSPathGetComponentAtIndex(GlyphSet, ComponentCount - 3);
                FSPathComponent WidthComponent = FSPathGetComponentAtIndex(GlyphSet, ComponentCount - 4);
                
                if ((FSPathComponentGetType(WidthComponent) == FSPathComponentTypeExtension) &&
                    (FSPathComponentGetType(HeightComponent) == FSPathComponentTypeExtension) &&
                    (FSPathComponentGetType(PaletteComponent) == FSPathComponentTypeExtension))
                {
                    const char *WidthStr = FSPathComponentGetString(WidthComponent);
                    const char *HeightStr = FSPathComponentGetString(HeightComponent);
                    const char *PaletteStr = FSPathComponentGetString(PaletteComponent);
                    
                    size_t Width = WidthStr[0] - '0';
                    size_t Height = HeightStr[0] - '0';
                    size_t Palette = PaletteStr[0] - '0';
                    
                    if ((Width < 10) && (Height < 10) && (Palette < 10))
                    {
                        if (WidthStr[1]) Width = (Width * 10) + (WidthStr[1] - '0');
                        if (HeightStr[1]) Height = (Height * 10) + (HeightStr[1] - '0');
                        if (PaletteStr[1]) Palette = (Palette * 10) + (PaletteStr[1] - '0');
                        
                        const size_t BitmapSize = HK_HUB_MODULE_GRAPHICS_ADAPTER_GLYPH_BITMAP_SIZE(Width, Height, Palette);
                        
                        if ((--Width < 16) && (--Height < 16) && (--Palette < 8))
                        {
                            const size_t Index = (Width << 7) | (Height << 3) | Palette;
                            
                            if (!HKHubModuleGraphicsAdapterGlyphBitmaps[Index][0])
                            {
                                HKHubModuleGraphicsAdapterGlyphBitmaps[Index][0] = CCArrayCreate(CC_STD_ALLOCATOR, BitmapSize, 16);
                            }
                            
                            FSHandle Handle;
                            if (FSHandleOpen(GlyphSet, FSHandleTypeRead, &Handle) == FSOperationSuccess)
                            {
#define MAX_BATCH_SIZE 16384
                                uint8_t Bitmaps[MAX_BATCH_SIZE];
                                size_t MaxCount = MAX_BATCH_SIZE / BitmapSize;
                                
                                for ( ; ; )
                                {
                                    size_t Size = BitmapSize * MaxCount;
                                    FSHandleRead(Handle, &Size, Bitmaps, FSBehaviourUpdateOffset);
                                    
                                    const size_t Count = Size / BitmapSize;
                                    if (!Count) break;
                                    
                                    if (CCArrayAppendElements(HKHubModuleGraphicsAdapterGlyphBitmaps[Index][0], Bitmaps, Count) == SIZE_MAX)
                                    {
                                        if (!HKHubModuleGraphicsAdapterGlyphBitmaps[Index][1])
                                        {
                                            HKHubModuleGraphicsAdapterGlyphBitmaps[Index][1] = CCArrayCreate(CC_STD_ALLOCATOR, BitmapSize, 16);
                                        }
                                        
                                        CCArrayAppendElements(HKHubModuleGraphicsAdapterGlyphBitmaps[Index][1], Bitmaps, Count);
                                    }
                                }
                                
                                FSHandleClose(Handle);
                            }
                            
                            else CC_LOG_ERROR("Failed to open glyphset file: %s", FSPathGetFullPathString(Path));
                        }
                    }
                }
            }
        }
        
        CCCollectionDestroy(GlyphSets);
    }
    
    FSPathAppendComponent(Path, FSPathComponentCreate(FSPathComponentTypeFile, "glyphset"));
    
#if CC_HARDWARE_ENDIAN_LITTLE
    FSPathAppendComponent(Path, FSPathComponentCreate(FSPathComponentTypeExtension, "little"));
#elif CC_HARDWARE_ENDIAN_BIG
    FSPathAppendComponent(Path, FSPathComponentCreate(FSPathComponentTypeExtension, "big"));
#else
#error Unknown endianness
#endif
    
    FSPathAppendComponent(Path, FSPathComponentCreate(FSPathComponentTypeExtension, "index"));
    
    if (FSManagerExists(Path))
    {
        FSHandle Handle;
        if (FSHandleOpen(Path, FSHandleTypeRead, &Handle) == FSOperationSuccess)
        {
            const size_t IndexSize = CCArrayGetElementSize(HKHubModuleGraphicsAdapterGlyphIndexes);
            size_t Size = IndexSize * CCArrayGetChunkSize(HKHubModuleGraphicsAdapterGlyphIndexes);
            
            FSHandleRead(Handle, &Size, CCArrayGetData(HKHubModuleGraphicsAdapterGlyphIndexes), FSBehaviourDefault);
            FSHandleClose(Handle);
            
            HKHubModuleGraphicsAdapterGlyphIndexes->count = Size / IndexSize;
        }
        
        else CC_LOG_ERROR("Failed to open glyphset index file: %s", FSPathGetFullPathString(Path));
    }
    
    CCCollectionDestroy(GlyphSets);
    FSPathDestroy(Path);
}

#define HK_HUB_MODULE_GRAPHICS_ADAPTER_NULL_GLYPH_INDEX 0

void HKHubModuleGraphicsAdapterStaticGlyphSet(CCChar Character, uint8_t Width, uint8_t Height, uint8_t PaletteSize, const uint8_t *Bitmap, size_t Count)
{
    CCAssertLog(Width <= 0xf, "Width must not exceed maximum width");
    CCAssertLog(Height <= 0xf, "Height must not exceed maximum height");
    CCAssertLog(PaletteSize <= 0x7, "PaletteSize must not exceed maximum palette size");
    CCAssertLog(Count <= HKHubModuleGraphicsAdapterCellGlyphIndexMask, "Count must not exceed maximum character count");
    
    if (Bitmap)
    {
        uint32_t Index = (Width << 7) | (Height << 3)| PaletteSize;
        
        if (!HKHubModuleGraphicsAdapterGlyphBitmaps[Index][0])
        {
            HKHubModuleGraphicsAdapterGlyphBitmaps[Index][0] = CCArrayCreate(CC_STD_ALLOCATOR, HK_HUB_MODULE_GRAPHICS_ADAPTER_GLYPH_BITMAP_SIZE(Width + 1, Height + 1, PaletteSize + 1), 16);
        }
        
        size_t First = CCArrayAppendElements(HKHubModuleGraphicsAdapterGlyphBitmaps[Index][0], Bitmap, Count);
        
        if (First == SIZE_MAX)
        {
            if (!HKHubModuleGraphicsAdapterGlyphBitmaps[Index][1])
            {
                HKHubModuleGraphicsAdapterGlyphBitmaps[Index][1] = CCArrayCreate(CC_STD_ALLOCATOR, HK_HUB_MODULE_GRAPHICS_ADAPTER_GLYPH_BITMAP_SIZE(Width + 1, Height + 1, PaletteSize + 1), 16);
            }
            
            First = CCArrayAppendElements(HKHubModuleGraphicsAdapterGlyphBitmaps[Index][1], Bitmap, Count);
            if (First == SIZE_MAX) return;
            
            First += CCArrayGetChunkSize(HKHubModuleGraphicsAdapterGlyphBitmaps[Index][0]);
        }
        
        if ((First & ~HKHubModuleGraphicsAdapterCellGlyphIndexMask) || ((First + Count) & ~HKHubModuleGraphicsAdapterCellGlyphIndexMask))
        {
            CC_LOG_ERROR("Exceeded static glyph count: (%d, %d, %d) %zu %zu", Width, Height, PaletteSize, CCArrayGetCount(HKHubModuleGraphicsAdapterGlyphBitmaps[Index][0]), HKHubModuleGraphicsAdapterGlyphBitmaps[Index][1] ? CCArrayGetCount(HKHubModuleGraphicsAdapterGlyphBitmaps[Index][1]) : 0);
            return;
        }
        
        Index = (Index << 21) | (uint32_t)First;
        
        for (size_t Loop = 0; Loop < Count; Loop++)
        {
            *(uint32_t*)CCArrayGetElementAtIndex(HKHubModuleGraphicsAdapterGlyphIndexes, Character + (uint32_t)Loop) = Index + (uint32_t)Loop;
        }
    }
    
    else
    {
        for (size_t Loop = 0; Loop < Count; Loop++)
        {
            *(uint32_t*)CCArrayGetElementAtIndex(HKHubModuleGraphicsAdapterGlyphIndexes, Character + (uint32_t)Loop) = HK_HUB_MODULE_GRAPHICS_ADAPTER_NULL_GLYPH_INDEX;
        }
    }
}

const uint8_t *HKHubModuleGraphicsAdapterStaticGlyphGet(CCChar Character, uint8_t *Width, uint8_t *Height, uint8_t *PaletteSize)
{
    const uint32_t Index = Character < CCArrayGetCount(HKHubModuleGraphicsAdapterGlyphIndexes) ? *(uint32_t*)CCArrayGetElementAtIndex(HKHubModuleGraphicsAdapterGlyphIndexes, Character) : HK_HUB_MODULE_GRAPHICS_ADAPTER_NULL_GLYPH_INDEX;
    
    *Width = Index >> 28;
    *Height = (Index >> 24) & 0xf;
    *PaletteSize = (Index >> 21) & 0x7;
    
    CCArray *Bitmaps = HKHubModuleGraphicsAdapterGlyphBitmaps[Index >> 21];
    uint32_t BitmapIndex = Index & HKHubModuleGraphicsAdapterCellGlyphIndexMask;
    
    if (!Bitmaps[0]) goto NotFound;
    
    if (BitmapIndex < CCArrayGetCount(Bitmaps[0]))
    {
        return CCArrayGetElementAtIndex(Bitmaps[0], BitmapIndex);
    }
    
    const size_t ChunkSize = CCArrayGetChunkSize(Bitmaps[0]);
    
    if (BitmapIndex >= ChunkSize) goto NotFound;
    
    BitmapIndex -= CCArrayGetChunkSize(Bitmaps[0]);
    
    if (!Bitmaps[1]) goto NotFound;
    
    if (BitmapIndex < CCArrayGetCount(Bitmaps[1]))
    {
        return CCArrayGetElementAtIndex(Bitmaps[1], BitmapIndex);
    }
    
NotFound:
    CC_LOG_ERROR("Bitmap does not exist: no bitmap at index (%u) in bitmaps (%p:%zu, %p:%zu)", Index, Bitmaps[0], CCArrayGetCount(Bitmaps[0]), Bitmaps[1], CCArrayGetCount(Bitmaps[1]));
    
    return HK_HUB_MODULE_GRAPHICS_ADAPTER_NULL_GLYPH_INDEX;
}
