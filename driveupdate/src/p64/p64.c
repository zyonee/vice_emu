
/*
*************************************************************
** P64 reference implementation by Benjamin 'BeRo' Rosseaux *
*************************************************************
**
** Copyright (c) 2011-2012, Benjamin Rosseaux
**
** This software is provided 'as-is', without any express or implied
** warranty. In no event will the authors be held liable for any damages
** arising from the use of this software.
**
** Permission is granted to anyone to use this software for any purpose,
** including commercial applications, and to alter it and redistribute it
** freely, subject to the following restrictions:
**
**    1. The origin of this software must not be misrepresented; you must not
**    claim that you wrote the original software. If you use this software
**    in a product, an acknowledgment in the product documentation would be
**    appreciated but is not required.
**
**    2. Altered source versions must be plainly marked as such, and must not be
**    misrepresented as being the original software.
**
**    3. This notice may not be removed or altered from any source
**   distribution.
**
*/

#include "p64.h"

enum
{
    P64_LITTLE_ENDIAN = 0x03020100ul,
    P64_BIG_ENDIAN = 0x00010203ul
};

#pragma pack(push)
#pragma pack(1)

static const union
{
    uint8_t bytes[4];
    uint32_t value;
} P64_host_order = { { 0, 1, 2, 3 } };

#pragma pack(pop)

void P64EndianSwap(void* x)
{
    if (P64_host_order.value == P64_BIG_ENDIAN)
    {
        *((uint32_t*)x) = ((*((uint32_t*)x)) > 24) | (((*((uint32_t*)x)) << 8) & 0x00ff0000) | (((*((uint32_t*)x)) >> 8) & 0x0000ff00) | ((*((uint32_t*)x)) << 24);
    }
}


uint32_t P64CRC32(uint8_t* Data, uint32_t Len)
{

    const uint32_t CRC32Table[16] = {0x00000000UL, 0x1db71064UL, 0x3b6e20c8UL, 0x26d930acUL,
                                     0x76dc4190UL, 0x6b6b51f4UL, 0x4db26158UL, 0x5005713cUL,
                                     0xedb88320UL, 0xf00f9344UL, 0xd6d6a3e8UL, 0xcb61b38cUL,
                                     0x9b64c2b0UL, 0x86d3d2d4UL, 0xa00ae278UL, 0xbdbdf21cUL
                                    };

    uint32_t value, pos;

    if (!Len)
    {
        return 0;
    }

    for (value = 0xffffffffUL,pos = 0; pos < Len; pos++)
    {
        value ^= Data[pos];
        value = CRC32Table[value & 0xf] ^(value >> 4);
        value = CRC32Table[value & 0xf] ^(value >> 4);
    }

    return value ^ 0xffffffffUL;
}

typedef uint32_t* PP64RangeCoderProbabilities;

typedef struct
{
    uint8_t* Buffer;
    uint32_t BufferSize;
    uint32_t BufferPosition;
    uint32_t RangeCode;
    uint32_t RangeLow;
    uint32_t RangeHigh;
    uint32_t RangeMiddle;
} TP64RangeCoder;

typedef TP64RangeCoder* PP64RangeCoder;

PP64RangeCoderProbabilities P64RangeCoderProbabilitiesAllocate(uint32_t Count)
{
    return malloc(Count * sizeof(uint32_t));
}

void P64RangeCoderProbabilitiesFree(PP64RangeCoderProbabilities Probabilities)
{
    free(Probabilities);
}

void P64RangeCoderProbabilitiesReset(PP64RangeCoderProbabilities Probabilities, uint32_t Count)
{
    uint32_t Index;
    for (Index = 0; Index < Count; Index++)
    {
        Probabilities[Index] = 2048;
    }
}

uint8_t P64RangeCoderRead(PP64RangeCoder Instance)
{
    if (Instance->BufferPosition < Instance->BufferSize)
    {
        return Instance->Buffer[Instance->BufferPosition++];
    }
    return 0;
}

void P64RangeCoderWrite(PP64RangeCoder Instance, uint8_t Value)
{
    if (Instance->BufferPosition >= Instance->BufferSize)
    {
        if (Instance->BufferSize < 16)
        {
            Instance->BufferSize = 16;
        }
        while (Instance->BufferPosition >= Instance->BufferSize)
        {
            Instance->BufferSize += Instance->BufferSize;
        }
        if (Instance->Buffer)
        {
            Instance->Buffer = realloc(Instance->Buffer, Instance->BufferSize);
        }
        else
        {
            Instance->Buffer = malloc(Instance->BufferSize);
        }
    }
    Instance->Buffer[Instance->BufferPosition++] = Value;
}

void P64RangeCoderInit(PP64RangeCoder Instance)
{
    Instance->RangeCode = 0;
    Instance->RangeLow = 0;
    Instance->RangeHigh = 0xffffffffUL;
}

void P64RangeCoderStart(PP64RangeCoder Instance)
{
    uint32_t Counter;
    for (Counter = 0; Counter < 4; Counter++)
    {
        Instance->RangeCode = (Instance->RangeCode << 8) | P64RangeCoderRead(Instance);
    }
}

void P64RangeCoderFlush(PP64RangeCoder Instance)
{
    uint32_t Counter;
    for (Counter = 0; Counter < 4; Counter++)
    {
        P64RangeCoderWrite(Instance, Instance->RangeHigh >> 24);
        Instance->RangeHigh <<= 8;
    }
}

void P64RangeCoderEncodeNormalize(PP64RangeCoder Instance)
{
    while (!((Instance->RangeLow ^ Instance->RangeHigh) & 0xff000000UL))
    {
        P64RangeCoderWrite(Instance, Instance->RangeHigh >> 24);
        Instance->RangeLow <<= 8;
        Instance->RangeHigh = (Instance->RangeHigh << 8) | 0xff;
    }
}

uint32_t P64RangeCoderEncodeBit(PP64RangeCoder Instance, uint32_t* Probability, uint32_t Shift, uint32_t BitValue)
{
    Instance->RangeMiddle = Instance->RangeLow + ((uint32_t)((uint32_t)(Instance->RangeHigh - Instance->RangeLow) >> 12) * (*Probability));
    if (BitValue)
    {
        *Probability += (0xfff - *Probability) >> Shift;
        Instance->RangeHigh = Instance->RangeMiddle;
    }
    else
    {
        *Probability -= *Probability >> Shift;
        Instance->RangeLow = Instance->RangeMiddle + 1;
    }
    P64RangeCoderEncodeNormalize(Instance);
    return BitValue;
}

uint32_t P64RangeCoderEncodeBitWithoutProbability(PP64RangeCoder Instance, uint32_t BitValue)
{
    Instance->RangeMiddle = Instance->RangeLow + ((Instance->RangeHigh - Instance->RangeLow) >> 1);
    if (BitValue)
    {
        Instance->RangeHigh = Instance->RangeMiddle;
    }
    else
    {
        Instance->RangeLow = Instance->RangeMiddle + 1;
    }
    P64RangeCoderEncodeNormalize(Instance);
    return BitValue;
}

void P64RangeCoderDecodeNormalize(PP64RangeCoder Instance)
{
    while (!((Instance->RangeLow ^ Instance->RangeHigh) & 0xff000000UL))
    {
        Instance->RangeLow <<= 8;
        Instance->RangeHigh = (Instance->RangeHigh << 8) | 0xff;
        Instance->RangeCode = (Instance->RangeCode << 8) | P64RangeCoderRead(Instance);
    }
}

uint32_t P64RangeCoderDecodeBit(PP64RangeCoder Instance, uint32_t *Probability, uint32_t Shift)
{
    uint32_t bit;
    Instance->RangeMiddle = Instance->RangeLow + ((uint32_t)((uint32_t)(Instance->RangeHigh - Instance->RangeLow) >> 12) * (*Probability));
    if (Instance->RangeCode <= Instance->RangeMiddle)
    {
        *Probability += (0xfff - *Probability) >> Shift;
        Instance->RangeHigh = Instance->RangeMiddle;
        bit = 1;
    }
    else
    {
        *Probability -= *Probability >> Shift;
        Instance->RangeLow = Instance->RangeMiddle + 1;
        bit = 0;
    }
    P64RangeCoderDecodeNormalize(Instance);
    return bit;
}

uint32_t P64RangeCoderDecodeBitWithoutProbability(PP64RangeCoder Instance)
{
    uint32_t bit;
    Instance->RangeMiddle = Instance->RangeLow + ((Instance->RangeHigh - Instance->RangeLow) >> 1);
    if (Instance->RangeCode <= Instance->RangeMiddle)
    {
        Instance->RangeHigh = Instance->RangeMiddle;
        bit = 1;
    }
    else
    {
        Instance->RangeLow = Instance->RangeMiddle + 1;
        bit = 0;
    }
    P64RangeCoderDecodeNormalize(Instance);
    return bit;
}

uint32_t P64RangeCoderEncodeDirectBits(PP64RangeCoder Instance, uint32_t Bits, uint32_t Value)
{
    while (Bits--)
    {
        P64RangeCoderEncodeBitWithoutProbability(Instance, (Value >> Bits) & 1);
    }
    return Value;
}

uint32_t P64RangeCoderDecodeDirectBits(PP64RangeCoder Instance, uint32_t Bits)
{
    uint32_t Value = 0;
    while (Bits--)
    {
        Value += Value + P64RangeCoderDecodeBitWithoutProbability(Instance);
    }
    return Value;
}

void P64MemoryStreamCreate(PP64MemoryStream Instance)
{
    memset(Instance, 0, sizeof(TP64MemoryStream));
}

void P64MemoryStreamDestroy(PP64MemoryStream Instance)
{
    if (Instance->Data)
    {
        free(Instance->Data);
    }
    memset(Instance, 0, sizeof(TP64MemoryStream));
}

void P64MemoryStreamClear(PP64MemoryStream Instance)
{
    if (Instance->Data)
    {
        free(Instance->Data);
    }
    memset(Instance, 0, sizeof(TP64MemoryStream));
}

uint32_t P64MemoryStreamSeek(PP64MemoryStream Instance, uint32_t Position)
{
    if (Position < Instance->Size)
    {
        Instance->Position = Position;
    }
    return Instance->Position;
}

uint32_t P64MemoryStreamRead(PP64MemoryStream Instance, uint8_t* Data, uint32_t Count)
{
    uint32_t ToDo = 0;
    if ((Count > 0) && (Instance->Position < Instance->Size))
    {
        ToDo = Instance->Size - Instance->Position;
        if (ToDo > Count)
        {
            ToDo = Count;
        }
        memmove(Data, Instance->Data + Instance->Position, ToDo);
        Instance->Position += ToDo;
    }
    return ToDo;
}

uint32_t P64MemoryStreamWrite(PP64MemoryStream Instance, uint8_t* Data, uint32_t Count)
{
    if (Count)
    {
        if ((Instance->Position + Count) >= Instance->Allocated)
        {
            if (Instance->Allocated < 16)
            {
                Instance->Allocated = 16;
            }
            while ((Instance->Position + Count) >= Instance->Allocated)
            {
                Instance->Allocated += Instance->Allocated;
            }
            if (Instance->Data)
            {
                Instance->Data = realloc(Instance->Data, Instance->Allocated);
            }
            else
            {
                Instance->Data = malloc(Instance->Allocated);
            }
        }
        memmove(Instance->Data + Instance->Position, Data, Count);
        Instance->Position += Count;
        if (Instance->Size < Instance->Position)
        {
            Instance->Size = Instance->Position;
        }
        return Count;
    }
    else
    {
        return 0;
    }
}

uint32_t P64MemoryStreamAssign(PP64MemoryStream Instance, PP64MemoryStream FromInstance)
{
    if (Instance->Data)
    {
        free(Instance->Data);
    }
    memset(Instance, 0, sizeof(TP64MemoryStream));
    Instance->Data = malloc(FromInstance->Allocated);
    Instance->Size = FromInstance->Size;
    Instance->Allocated = FromInstance->Allocated;
    Instance->Position = 0;
    if (Instance->Size)
    {
        memmove(Instance->Data, FromInstance->Data, Instance->Size);
    }
    return Instance->Size;
}

uint32_t P64MemoryStreamAppend(PP64MemoryStream Instance, PP64MemoryStream FromInstance)
{
    if (FromInstance->Size)
    {
        FromInstance->Position = FromInstance->Size;
        return P64MemoryStreamWrite(Instance, FromInstance->Data, FromInstance->Size);
    }
    return 0;
}

uint32_t P64MemoryStreamAppendFrom(PP64MemoryStream Instance, PP64MemoryStream FromInstance)
{
    if ((FromInstance->Size > 0) && (FromInstance->Position < FromInstance->Size))
    {
        if (P64MemoryStreamWrite(Instance, FromInstance->Data + FromInstance->Position, FromInstance->Size - FromInstance->Position))
        {
            FromInstance->Position = FromInstance->Size;
            return 1;
        }
        FromInstance->Position = FromInstance->Size;
    }
    return 0;
}

uint32_t P64MemoryStreamAppendFromCount(PP64MemoryStream Instance, PP64MemoryStream FromInstance, uint32_t Count)
{
    uint32_t ToDo = 0;
    if ((Count > 0) && (FromInstance->Position < FromInstance->Size))
    {
        ToDo = FromInstance->Size - FromInstance->Position;
        if (ToDo > Count)
        {
            ToDo = Count;
        }
        if (ToDo > 0)
        {
            ToDo = P64MemoryStreamWrite(Instance, FromInstance->Data + FromInstance->Position, ToDo);
            FromInstance->Position += ToDo;
        }
    }
    return ToDo;
}

void P64PulseStreamCreate(PP64PulseStream Instance)
{
    memset(Instance, 0, sizeof(TP64PulseStream));
    Instance->Pulses = 0;
    Instance->PulsesAllocated = 0;
    Instance->PulsesCount = 0;
    Instance->UsedFirst = -1;
    Instance->UsedLast = -1;
    Instance->FreeList = -1;
    Instance->CurrentIndex = -1;
    Instance->BitStreamLength = 0;
    Instance->SpeedZone = 0;
}

void P64PulseStreamDestroy(PP64PulseStream Instance)
{
    P64PulseStreamClear(Instance);
    memset(Instance, 0, sizeof(TP64PulseStream));
}

void P64PulseStreamClear(PP64PulseStream Instance)
{
    if (Instance->Pulses)
    {
        free(Instance->Pulses);
    }
    Instance->Pulses = 0;
    Instance->PulsesAllocated = 0;
    Instance->PulsesCount = 0;
    Instance->UsedFirst = -1;
    Instance->UsedLast = -1;
    Instance->FreeList = -1;
    Instance->CurrentIndex = -1;
    Instance->BitStreamLength = 0;
    Instance->SpeedZone = 0;
}

int32_t P64PulseStreamAllocatePulse(PP64PulseStream Instance)
{
    int32_t Index;
    if (Instance->FreeList < 0)
    {
        if (Instance->PulsesCount >= Instance->PulsesAllocated)
        {
            if (Instance->PulsesAllocated < 16)
            {
                Instance->PulsesAllocated = 16;
            }
            while (Instance->PulsesCount >= Instance->PulsesAllocated)
            {
                Instance->PulsesAllocated += Instance->PulsesAllocated;
            }
            if (Instance->Pulses)
            {
                Instance->Pulses = realloc(Instance->Pulses, Instance->PulsesAllocated * sizeof(TP64Pulse));
            }
            else
            {
                Instance->Pulses = malloc(Instance->PulsesAllocated * sizeof(TP64Pulse));
            }
        }
        Index = Instance->PulsesCount++;
    }
    else
    {
        Index = Instance->FreeList;
        Instance->FreeList = Instance->Pulses[Index].Next;
    }
    Instance->Pulses[Index].Previous	= -1;
    Instance->Pulses[Index].Next	= -1;
    Instance->Pulses[Index].Position	= 0;
    Instance->Pulses[Index].Strength	= 0;
    return Index;
}

void P64PulseStreamFreePulse(PP64PulseStream Instance, int32_t Index)
{
    if (Instance->CurrentIndex == Index)
    {
        Instance->CurrentIndex = Instance->Pulses[Index].Next;
    }
    if (Instance->Pulses[Index].Previous < 0)
    {
        Instance->UsedFirst = Instance->Pulses[Index].Next;
    }
    else
    {
        Instance->Pulses[Instance->Pulses[Index].Previous].Next = Instance->Pulses[Index].Next;
    }
    if (Instance->Pulses[Index].Next < 0)
    {
        Instance->UsedLast = Instance->Pulses[Index].Previous;
    }
    else
    {
        Instance->Pulses[Instance->Pulses[Index].Next].Previous = Instance->Pulses[Index].Previous;
    }
    Instance->Pulses[Index].Previous = -1;
    Instance->Pulses[Index].Next = Instance->FreeList;
    Instance->FreeList = Index;
}

void P64PulseStreamAddPulse(PP64PulseStream Instance, uint32_t Position, uint32_t Strength)
{
    int32_t Current, Index;
    while (Position >= P64PulseSamplesPerRotation)
    {
        Position -= P64PulseSamplesPerRotation;
    }
    Current = Instance->CurrentIndex;
    if ((Instance->UsedLast >= 0) && (Instance->Pulses[Instance->UsedLast].Position < Position))
    {
        Current = -1;
    }
    else
    {
        if ((Current < 0) || ((Current != Instance->UsedFirst) && ((Instance->Pulses[Current].Previous >= 0) && (Instance->Pulses[Instance->Pulses[Current].Previous].Position >= Position))))
        {
            Current = Instance->UsedFirst;
        }
        while ((Current >= 0) && (Instance->Pulses[Current].Position < Position))
        {
            Current = Instance->Pulses[Current].Next;
        }
    }
    if (Current < 0)
    {
        Index = P64PulseStreamAllocatePulse(Instance);
        if (Instance->UsedLast < 0)
        {
            Instance->UsedFirst = Index;
        }
        else
        {
            Instance->Pulses[Instance->UsedLast].Next = Index;
            Instance->Pulses[Index].Previous = Instance->UsedLast;
        }
        Instance->UsedLast = Index;
    }
    else
    {
        if (Instance->Pulses[Current].Position == Position)
        {
            Index = Current;
        }
        else
        {
            Index = P64PulseStreamAllocatePulse(Instance);
            Instance->Pulses[Index].Previous = Instance->Pulses[Current].Previous;
            Instance->Pulses[Index].Next = Current;
            Instance->Pulses[Current].Previous = Index;
            if (Instance->Pulses[Index].Previous < 0)
            {
                Instance->UsedFirst = Index;
            }
            else
            {
                Instance->Pulses[Instance->Pulses[Index].Previous].Next = Index;
            }
        }
    }
    Instance->Pulses[Index].Position = Position;
    Instance->Pulses[Index].Strength = Strength;
    Instance->CurrentIndex = Index;
}

void P64PulseStreamRemovePulses(PP64PulseStream Instance, uint32_t Position, uint32_t Count)
{
    uint32_t ToDo;
    int32_t Current, Next;
    while (Position >= P64PulseSamplesPerRotation)
    {
        Position -= P64PulseSamplesPerRotation;
    }
    while (Count)
    {
        ToDo = ((Position + Count) > P64PulseSamplesPerRotation) ? (P64PulseSamplesPerRotation - Position) : Count;
        Current = Instance->CurrentIndex;
        if ((Current < 0) || ((Current != Instance->UsedFirst) && ((Instance->Pulses[Current].Previous >= 0) && (Instance->Pulses[Instance->Pulses[Current].Previous].Position >= Position))))
        {
            Current = Instance->UsedFirst;
        }
        while ((Current >= 0) && (Instance->Pulses[Current].Position < Position))
        {
            Current = Instance->Pulses[Current].Next;
        }
        while ((Current >= 0) && ((Instance->Pulses[Current].Position >= Position) && (Instance->Pulses[Current].Position < (Position + ToDo))))
        {
            Next = Instance->Pulses[Current].Next;
            P64PulseStreamFreePulse(Instance, Current);
            Current = Next;
        }
        Position += ToDo;
        Count -= ToDo;
    }
}

void P64PulseStreamRemovePulse(PP64PulseStream Instance, uint32_t Position)
{
    int32_t Current;
    while (Position >= P64PulseSamplesPerRotation)
    {
        Position -= P64PulseSamplesPerRotation;
    }
    Current = Instance->CurrentIndex;
    if ((Current < 0) || ((Current != Instance->UsedFirst) && ((Instance->Pulses[Current].Previous >= 0) && (Instance->Pulses[Instance->Pulses[Current].Previous].Position >= Position))))
    {
        Current = Instance->UsedFirst;
    }
    while ((Current >= 0) && (Instance->Pulses[Current].Position < Position))
    {
        Current = Instance->Pulses[Current].Next;
    }
    if ((Current >= 0) && (Instance->Pulses[Current].Position == Position))
    {
        P64PulseStreamFreePulse(Instance, Current);
    }
}

uint32_t P64PulseStreamDeltaPositionToNextPulse(PP64PulseStream Instance, uint32_t Position)
{
    int32_t Current;
    while (Position >= P64PulseSamplesPerRotation)
    {
        Position -= P64PulseSamplesPerRotation;
    }
    Current = Instance->CurrentIndex;
    if ((Current < 0) || ((Current != Instance->UsedFirst) && ((Instance->Pulses[Current].Previous >= 0) && (Instance->Pulses[Instance->Pulses[Current].Previous].Position >= Position))))
    {
        Current = Instance->UsedFirst;
    }
    while ((Current >= 0) && (Instance->Pulses[Current].Position < Position))
    {
        Current = Instance->Pulses[Current].Next;
    }
    if (Current < 0)
    {
        if (Instance->UsedFirst < 0)
        {
            return P64PulseSamplesPerRotation - Position;
        }
        else
        {
            return (P64PulseSamplesPerRotation + Instance->Pulses[Instance->UsedFirst].Position) - Position;
        }
    }
    else
    {
        Instance->CurrentIndex = Current;
        return Instance->Pulses[Current].Position - Position;
    }
}

uint32_t P64PulseStreamGetNextPulse(PP64PulseStream Instance, uint32_t Position)
{
    int32_t Current;
    while (Position >= P64PulseSamplesPerRotation)
    {
        Position -= P64PulseSamplesPerRotation;
    }
    Current = Instance->CurrentIndex;
    if ((Current < 0) || ((Current != Instance->UsedFirst) && ((Instance->Pulses[Current].Previous >= 0) && (Instance->Pulses[Instance->Pulses[Current].Previous].Position >= Position))))
    {
        Current = Instance->UsedFirst;
    }
    while ((Current >= 0) && (Instance->Pulses[Current].Position < Position))
    {
        Current = Instance->Pulses[Current].Next;
    }
    if (Current < 0)
    {
        if (Instance->UsedFirst < 0)
        {
            return 0;
        }
        else
        {
            return Instance->Pulses[Instance->UsedFirst].Strength;
        }
    }
    else
    {
        Instance->CurrentIndex = Current;
        return Instance->Pulses[Current].Strength;
    }
}

uint32_t P64PulseStreamGetPulseCount(PP64PulseStream Instance)
{
    int32_t Current, Count = 0;
    Current = Instance->CurrentIndex;
    while (Current >= 0)
    {
        Count++;
        Current = Instance->Pulses[Current].Next;
    }
    return Count;
}

uint32_t P64PulseStreamGetPulse(PP64PulseStream Instance, uint32_t Position)
{
    int32_t Current;
    while (Position >= P64PulseSamplesPerRotation)
    {
        Position -= P64PulseSamplesPerRotation;
    }
    Current = Instance->CurrentIndex;
    if ((Current < 0) || ((Current != Instance->UsedFirst) && ((Instance->Pulses[Current].Previous >= 0) && (Instance->Pulses[Instance->Pulses[Current].Previous].Position >= Position))))
    {
        Current = Instance->UsedFirst;
    }
    while ((Current >= 0) && (Instance->Pulses[Current].Position < Position))
    {
        Current = Instance->Pulses[Current].Next;
    }
    if ((Current < 0) || (Instance->Pulses[Current].Position != Position))
    {
        return 0;
    }
    else
    {
        Instance->CurrentIndex = Current;
        return Instance->Pulses[Current].Strength;
    }
}

void P64PulseStreamSetPulse(PP64PulseStream Instance, uint32_t Position, uint32_t Strength)
{
    if (Strength)
    {
        P64PulseStreamAddPulse(Instance, Position, Strength);
    }
    else
    {
        P64PulseStreamRemovePulse(Instance, Position);
    }
}

void P64PulseStreamSeek(PP64PulseStream Instance, uint32_t Position)
{
    int32_t Current;
    while (Position >= P64PulseSamplesPerRotation)
    {
        Position -= P64PulseSamplesPerRotation;
    }
    Current = Instance->CurrentIndex;
    if ((Current < 0) || ((Current != Instance->UsedFirst) && ((Instance->Pulses[Current].Previous >= 0) && (Instance->Pulses[Instance->Pulses[Current].Previous].Position >= Position))))
    {
        Current = Instance->UsedFirst;
    }
    while ((Current >= 0) && (Instance->Pulses[Current].Position < Position))
    {
        Current = Instance->Pulses[Current].Next;
    }
    Instance->CurrentIndex = Current;
}

void P64PulseStreamConvertFromGCR(PP64PulseStream Instance, uint8_t* Bytes, uint32_t Len)
{
    uint64_t Position, Increment;
    uint32_t BitStreamPosition;
    P64PulseStreamClear(Instance);
    if (Len)
    {
        Instance->BitStreamLength = Len;
        Increment = (((uint64_t)P64PulseSamplesPerRotation) << 32) / Instance->BitStreamLength;
        Position = Increment >> 1;
        for (BitStreamPosition = 0; BitStreamPosition < Instance->BitStreamLength; BitStreamPosition++)
        {
            if (((uint8_t)(Bytes[BitStreamPosition >> 3])) & (1 << ((~BitStreamPosition) & 7)))
            {
                P64PulseStreamAddPulse(Instance, (Position >> 32) & 0xffffffff, 0xffffffff);
            }
            Position += Increment;
        }
    }
}

void P64PulseStreamConvertToGCR(PP64PulseStream Instance, uint8_t* Bytes, uint32_t Len)
{
    uint64_t Range = ((uint64_t)P64PulseSamplesPerRotation) << 32;
    uint64_t Position, Increment;
    uint32_t BitStreamPosition, BitStreamLen;
    int32_t Current;
    if (Len)
    {
        memset(Bytes, 0, (Len + 7) >> 3);
        BitStreamLen = Len;
        Increment = Range / BitStreamLen;
        Current = Instance->UsedFirst;
        Position = (Current >= 0) ? (((((uint64_t)Instance->Pulses[Current].Position)) << 32) - 1) : 0;
        for (BitStreamPosition = 0; BitStreamPosition < BitStreamLen; BitStreamPosition++)
        {
            Position += Increment;
            while (1)
            {
                if ((Current >= 0) && (Instance->Pulses[Current].Position<(Position >> 32)))
                {
                    Position = (((uint64_t)Instance->Pulses[Current].Position) << 32) + (Increment - ((uint64_t)0x400000000LL)); /* 1.25 microseconds headroom */
                    Current = Instance->Pulses[Current].Next;
                    Bytes[BitStreamPosition >> 3] |= 1 << ((~BitStreamPosition) & 7);
                }
                else if (Position >= Range)
                {
                    Position -= Range;
                    Current = Instance->UsedFirst;
                    continue;
                }
                break;
            }
        }

        /* optional: add here GCR byte-realigning-to-syncmark-borders code, if your GCR routines are working bytewise-only */

    }
}

uint32_t P64PulseStreamReadMetaInfoFromStream(PP64PulseStream Instance, PP64MemoryStream Stream)
{
    TP64HalfTrackMetaInfoHeader Header;
    if (P64MemoryStreamRead(Stream, (void*)&Header, sizeof(TP64HalfTrackMetaInfoHeader)) == sizeof(TP64HalfTrackMetaInfoHeader))
    {
        P64EndianSwap(&Header.BitStreamLength);
        Instance->BitStreamLength = Header.BitStreamLength;
        Instance->SpeedZone = Header.SpeedZone;
        return 1;
    }
    return 0;
}

uint32_t P64PulseStreamWriteMetaInfoToStream(PP64PulseStream Instance, PP64MemoryStream Stream)
{
    TP64HalfTrackMetaInfoHeader Header;
    memset(&Header, 0, sizeof(TP64HalfTrackMetaInfoHeader));
    Header.BitStreamLength = Instance->BitStreamLength;
    Header.SpeedZone = Instance->SpeedZone;
    P64EndianSwap(&Header.BitStreamLength);
    return P64MemoryStreamWrite(Stream, (void*)&Header, sizeof(TP64HalfTrackMetaInfoHeader)) == sizeof(TP64HalfTrackMetaInfoHeader);
}

#define ModelPosition 0
#define ModelStrength 4
#define ModelPositionFlag 8
#define ModelStrengthFlag 9

#define ProbabilityModelCount 10

const uint32_t ProbabilityCounts[ProbabilityModelCount] = {65536, 65536, 65536, 65536, 65536, 65536, 65536, 65536, 4, 4};

uint32_t P64PulseStreamReadFromStream(PP64PulseStream Instance, PP64MemoryStream Stream)
{
    PP64RangeCoderProbabilities RangeCoderProbabilities;
    uint32_t RangeCoderProbabilityOffsets[ProbabilityModelCount];
    uint32_t RangeCoderProbabilityStates[ProbabilityModelCount];
    TP64RangeCoder RangeCoderInstance;
    uint32_t ProbabilityCount, Index, Count, DeltaPosition, Position, Strength, result;
    uint8_t *Buffer;
    TP64HalfTrackHeader Header;

    if (P64MemoryStreamRead(Stream, (void*)&Header, sizeof(TP64HalfTrackHeader)) == sizeof(TP64HalfTrackHeader))
    {
        P64EndianSwap(&Header.CountPulses);
        P64EndianSwap(&Header.Size);

        if (!Header.Size)
        {
            return 0;
        }

        Buffer = malloc(Header.Size);

        if (P64MemoryStreamRead(Stream, Buffer, Header.Size) == Header.Size)
        {

            ProbabilityCount = 0;
            for (Index = 0; Index < ProbabilityModelCount; Index++)
            {
                RangeCoderProbabilityOffsets[Index] = ProbabilityCount;
                ProbabilityCount += ProbabilityCounts[Index];
                RangeCoderProbabilityStates[Index] = 0;
            }
            RangeCoderProbabilities = P64RangeCoderProbabilitiesAllocate(ProbabilityCount);
            P64RangeCoderProbabilitiesReset(RangeCoderProbabilities, ProbabilityCount);

            memset(&RangeCoderInstance, 0, sizeof(TP64RangeCoder));
            P64RangeCoderInit(&RangeCoderInstance);

            RangeCoderInstance.Buffer = Buffer;
            RangeCoderInstance.BufferSize = Header.Size;
            RangeCoderInstance.BufferPosition = 0;
            P64RangeCoderStart(&RangeCoderInstance);

            Count = 0;

            Position = 0;
            DeltaPosition = 0;

            Strength = 0;

#define ReadBit(Model) (RangeCoderProbabilityStates[Model] = P64RangeCoderDecodeBit(&RangeCoderInstance, RangeCoderProbabilities + (RangeCoderProbabilityOffsets[Model] + RangeCoderProbabilityStates[Model]), 4))

#define ReadDWord(Model) \
          { \
            uint32_t ByteValue, ByteIndex, Context; \
            int32_t Bit; \
					  result = 0; \
					  for (ByteIndex = 0; ByteIndex < 4; ByteIndex++) { \
						  Context = 1; \
  						for (Bit = 7; Bit >= 0; Bit--) { \
	  						Context = (Context << 1) | P64RangeCoderDecodeBit(&RangeCoderInstance, RangeCoderProbabilities + (RangeCoderProbabilityOffsets[Model + ByteIndex] + (((RangeCoderProbabilityStates[Model + ByteIndex] << 8) | Context) & 0xfffful)), 4); \
		  				} \
			  			ByteValue = Context & 0xff; \
				  		RangeCoderProbabilityStates[Model + ByteIndex] = ByteValue; \
					  	result |= ((ByteValue & 0xff) << (ByteIndex << 3)); \
            } \
					} \

            while (Count < Header.CountPulses)
            {

                if (ReadBit(ModelPositionFlag))
                {
                    ReadDWord(ModelPosition);
                    DeltaPosition = result;
                    if (!DeltaPosition)
                    {
                        break;
                    }
                }
                Position += DeltaPosition;

                if (ReadBit(ModelStrengthFlag))
                {
                    ReadDWord(ModelStrength);
                    Strength += result;
                }

                P64PulseStreamAddPulse(Instance, Position, Strength);

                Count++;
            }

            P64RangeCoderProbabilitiesFree(RangeCoderProbabilities);

            free(Buffer);

            return Count == Header.CountPulses;

        }

        free(Buffer);
    }


#undef ReadBit
#undef ReadDWord

    return 0;
}

uint32_t P64PulseStreamWriteToStream(PP64PulseStream Instance, PP64MemoryStream Stream)
{
    PP64RangeCoderProbabilities RangeCoderProbabilities;
    uint32_t RangeCoderProbabilityOffsets[ProbabilityModelCount];
    uint32_t RangeCoderProbabilityStates[ProbabilityModelCount];
    TP64RangeCoder RangeCoderInstance;
    int32_t Index, Current;
    uint32_t ProbabilityCount, LastPosition, PreviousDeltaPosition, DeltaPosition, LastStrength;
    TP64HalfTrackHeader Header;

    memset((void*)&Header, 0, sizeof(TP64HalfTrackHeader));

    ProbabilityCount = 0;
    for (Index = 0; Index < ProbabilityModelCount; Index++)
    {
        RangeCoderProbabilityOffsets[Index] = ProbabilityCount;
        ProbabilityCount += ProbabilityCounts[Index];
        RangeCoderProbabilityStates[Index] = 0;
    }
    RangeCoderProbabilities = P64RangeCoderProbabilitiesAllocate(ProbabilityCount);
    P64RangeCoderProbabilitiesReset(RangeCoderProbabilities, ProbabilityCount);

    memset(&RangeCoderInstance, 0, sizeof(TP64RangeCoder));
    P64RangeCoderInit(&RangeCoderInstance);

    LastPosition = 0;
    PreviousDeltaPosition = 0;

    LastStrength = 0;

#define WriteBit(Model, BitValue) RangeCoderProbabilityStates[Model] = P64RangeCoderEncodeBit(&RangeCoderInstance, RangeCoderProbabilities + (RangeCoderProbabilityOffsets[Model] + RangeCoderProbabilityStates[Model]), 4, BitValue)

#define WriteDWord(Model, DWordValue) \
      { \
        uint32_t ByteValue, ByteIndex, Context, Value; \
        int32_t Bit; \
        Value = DWordValue; \
	   		for (ByteIndex = 0; ByteIndex < 4; ByteIndex++) { \
			  	ByteValue = (Value >> (ByteIndex << 3)) & 0xff; \
				  Context = 1; \
				  for (Bit = 7; Bit >= 0; Bit--) { \
					  Context = (Context << 1) | P64RangeCoderEncodeBit(&RangeCoderInstance, RangeCoderProbabilities + (RangeCoderProbabilityOffsets[Model + ByteIndex] + (((RangeCoderProbabilityStates[Model+ByteIndex] << 8) | Context) & 0xfffful)), 4, (ByteValue >> Bit) & 1); \
  				} \
	  			RangeCoderProbabilityStates[Model + ByteIndex] = ByteValue; \
	  		}	\
			}

    Current = Instance->UsedFirst;
    while (Current >= 0)
    {
        DeltaPosition = Instance->Pulses[Current].Position - LastPosition;
        if (PreviousDeltaPosition != DeltaPosition)
        {
            PreviousDeltaPosition = DeltaPosition;
            WriteBit(ModelPositionFlag, 1);
            WriteDWord(ModelPosition, DeltaPosition);
        }
        else
        {
            WriteBit(ModelPositionFlag, 0);
        }
        LastPosition = Instance->Pulses[Current].Position;

        if (LastStrength != Instance->Pulses[Current].Strength)
        {
            WriteBit(ModelStrengthFlag, 1);
            WriteDWord(ModelStrength, Instance->Pulses[Current].Strength - LastStrength);
        }
        else
        {
            WriteBit(ModelStrengthFlag, 0);
        }
        LastStrength = Instance->Pulses[Current].Strength;

        Header.CountPulses++;

        Current = Instance->Pulses[Current].Next;
    }

    WriteBit(ModelPositionFlag, 1);
    WriteDWord(ModelPosition, 0);

    P64RangeCoderFlush(&RangeCoderInstance);

    P64RangeCoderProbabilitiesFree(RangeCoderProbabilities);

    if (RangeCoderInstance.Buffer)
    {
        Header.Size = RangeCoderInstance.BufferPosition;
    }

    P64EndianSwap(&Header.CountPulses);
    P64EndianSwap(&Header.Size);

    if (P64MemoryStreamWrite(Stream, (void*)&Header, sizeof(TP64HalfTrackHeader)) == sizeof(TP64HalfTrackHeader))
    {
        if (RangeCoderInstance.Buffer)
        {
            if (P64MemoryStreamWrite(Stream, RangeCoderInstance.Buffer, RangeCoderInstance.BufferPosition) == RangeCoderInstance.BufferPosition)
            {
                free(RangeCoderInstance.Buffer);
                return 1;
            }
            free(RangeCoderInstance.Buffer);
            return 0;
        }
        return 1;
    }

#undef WriteBit
#undef WriteDWord
    return 0;
}

const uint8_t P64SpeedMap[84] = {3,
                                 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                 3, 3, 3, 3, 3, 3, 3, 2, 2, 2,
                                 2, 2, 2, 2, 1, 1, 1, 1, 1, 1,
                                 0, 0, 0, 0, 0,
                                 2, 2, 2, 2, 2, 2, 2,
                                 0, 0
                                };

const uint16_t P64RawTrackSize[4] = {6250, 6666, 7142, 7692};

void P64ImageCreate(PP64Image Instance)
{
    int32_t HalfTrack;
    memset(Instance, 0, sizeof(TP64Image));
    for (HalfTrack = 0; HalfTrack <= P64LastHalfTrack; HalfTrack++)
    {
        P64PulseStreamCreate(&Instance->PulseStreams[HalfTrack]);
    }
    P64ImageClear(Instance);
}

void P64ImageDestroy(PP64Image Instance)
{
    int32_t HalfTrack;
    for (HalfTrack = 0; HalfTrack <= P64LastHalfTrack; HalfTrack++)
    {
        P64PulseStreamDestroy(&Instance->PulseStreams[HalfTrack]);
    }
    memset(Instance, 0, sizeof(TP64Image));
}

void P64ImageClear(PP64Image Instance)
{
    int32_t HalfTrack;
    Instance->WriteProtected = 0;
    for (HalfTrack = 0; HalfTrack <= P64LastHalfTrack; HalfTrack++)
    {
        P64PulseStreamClear(&Instance->PulseStreams[HalfTrack]);
        Instance->PulseStreams[HalfTrack].SpeedZone = P64SpeedMap[HalfTrack >> 1];
        Instance->PulseStreams[HalfTrack].BitStreamLength = P64RawTrackSize[P64SpeedMap[HalfTrack >> 1]] << 3;
    }
}

uint32_t P64ImageReadFromStream(PP64Image Instance, PP64MemoryStream Stream)
{
    TP64MemoryStream ChunksMemoryStream, ChunkMemoryStream;
    TP64Header Header;
    uint32_t HalfTrack, OK;
    TP64ChunkHeader ChunkHeader;
    OK = 0;
    P64ImageClear(Instance);
    if (P64MemoryStreamSeek(Stream, 0) == 0)
    {
        if (P64MemoryStreamRead(Stream, (void*)&Header, sizeof(TP64Header)) == sizeof(TP64Header))
        {
            P64EndianSwap(&Header.Version);
            P64EndianSwap(&Header.Flags);
            P64EndianSwap(&Header.Size);
            P64EndianSwap(&Header.Checksum);
            if (((Header.Signature[0] == 'P') && (Header.Signature[1] == '6') && (Header.Signature[2] == '4') && (Header.Signature[3] == '-') && (Header.Signature[4] == '1') && (Header.Signature[5] == '5') && (Header.Signature[6] == '4') && (Header.Signature[7] == '1')) &&
                    (Header.Version == 0x00000000))
            {
                Instance->WriteProtected = (Header.Flags & 1) != 0;
                P64MemoryStreamCreate(&ChunksMemoryStream);
                if (P64MemoryStreamAppendFromCount(&ChunksMemoryStream, Stream, Header.Size) == Header.Size)
                {
                    if (P64CRC32(ChunksMemoryStream.Data, Header.Size) == Header.Checksum)
                    {
                        if (P64MemoryStreamSeek(&ChunksMemoryStream, 0) == 0)
                        {
                            OK = 1;
                            while (OK && (ChunksMemoryStream.Position < ChunksMemoryStream.Size))
                            {
                                if (P64MemoryStreamRead(&ChunksMemoryStream, (void*)&ChunkHeader, sizeof(TP64ChunkHeader)) == sizeof(TP64ChunkHeader))
                                {
                                    P64EndianSwap(&ChunkHeader.Size);
                                    P64EndianSwap(&ChunkHeader.Checksum);
                                    OK = 0;
                                    P64MemoryStreamCreate(&ChunkMemoryStream);
                                    if (ChunkHeader.Size == 0)
                                    {
                                        if (ChunkHeader.Checksum == 0)
                                        {
                                            if ((ChunkHeader.Signature[0] == 'D') && (ChunkHeader.Signature[1] == 'O') && (ChunkHeader.Signature[2] == 'N') && (ChunkHeader.Signature[3] == 'E'))
                                            {
                                                OK = 1;
                                            }
                                            else
                                            {
                                                OK = 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (P64MemoryStreamAppendFromCount(&ChunkMemoryStream, &ChunksMemoryStream, ChunkHeader.Size) == ChunkHeader.Size)
                                        {
                                            if (P64MemoryStreamSeek(&ChunkMemoryStream, 0) == 0)
                                            {
                                                if (P64CRC32(ChunkMemoryStream.Data, ChunkHeader.Size) == ChunkHeader.Checksum)
                                                {
                                                    if ((ChunkHeader.Signature[0] == 'H') && (ChunkHeader.Signature[1] == 'T') && (ChunkHeader.Signature[2] == 'M') && ((ChunkHeader.Signature[3] >= P64FirstHalfTrack) && (ChunkHeader.Signature[3] <= P64LastHalfTrack)))
                                                    {
                                                        HalfTrack = ChunkHeader.Signature[3];
                                                        OK = P64PulseStreamReadMetaInfoFromStream(&Instance->PulseStreams[HalfTrack], &ChunkMemoryStream);
                                                    }
                                                    else if ((ChunkHeader.Signature[0] == 'H') && (ChunkHeader.Signature[1] == 'T') && (ChunkHeader.Signature[2] == 'P') && ((ChunkHeader.Signature[3] >= P64FirstHalfTrack) && (ChunkHeader.Signature[3] <= P64LastHalfTrack)))
                                                    {
                                                        HalfTrack = ChunkHeader.Signature[3];
                                                        OK = P64PulseStreamReadFromStream(&Instance->PulseStreams[HalfTrack], &ChunkMemoryStream);
                                                    }
                                                    else
                                                    {
                                                        OK = 1;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    P64MemoryStreamDestroy(&ChunkMemoryStream);
                                }
                                else
                                {
                                    break;
                                }
                            }

                        }
                    }
                }
                P64MemoryStreamDestroy(&ChunksMemoryStream);
            }
        }
    }
    return OK;
}

uint32_t P64ImageWriteToStream(PP64Image Instance, PP64MemoryStream Stream)
{
    TP64MemoryStream MemoryStream, ChunksMemoryStream, ChunkMemoryStream;
    TP64Header Header;
    uint32_t HalfTrack, result, WriteChunkResult;

    TP64ChunkSignature ChunkSignature;
    TP64ChunkHeader ChunkHeader;

#define WriteChunk() \
{\
  WriteChunkResult = 0; \
  memcpy(&ChunkHeader.Signature, &ChunkSignature, sizeof(TP64ChunkSignature));  \
  ChunkHeader.Size = ChunkMemoryStream.Size; \
  ChunkHeader.Checksum = (ChunkMemoryStream.Size>0) ? P64CRC32(ChunkMemoryStream.Data,ChunkMemoryStream.Size) : 0; \
  P64EndianSwap(&ChunkHeader.Size); \
  P64EndianSwap(&ChunkHeader.Checksum); \
  if (P64MemoryStreamWrite(&ChunksMemoryStream,(void*)&ChunkHeader,sizeof(TP64ChunkHeader))==sizeof(TP64ChunkHeader)){ \
    if (ChunkMemoryStream.Size==0){ \
     WriteChunkResult = 1; \
    }else{ \
      if (P64MemoryStreamSeek(&ChunkMemoryStream,0)==0){ \
        if (P64MemoryStreamAppendFromCount(&ChunksMemoryStream,&ChunkMemoryStream,ChunkMemoryStream.Size)==ChunkMemoryStream.Size){ \
          WriteChunkResult = 1; \
        } \
      } \
    } \
  }\
}

    result = 0;

    memset((void*)&Header, 0, sizeof(TP64Header));

    Header.Signature[0] = 'P';
    Header.Signature[1] = '6';
    Header.Signature[2] = '4';
    Header.Signature[3] = '-';
    Header.Signature[4] = '1';
    Header.Signature[5] = '5';
    Header.Signature[6] = '4';
    Header.Signature[7] = '1';
    Header.Version = 0x00000000;

    P64MemoryStreamCreate(&MemoryStream);
    P64MemoryStreamCreate(&ChunksMemoryStream);

    result = 1;
    for (HalfTrack = P64FirstHalfTrack; HalfTrack <= P64LastHalfTrack; HalfTrack++)
    {

        P64MemoryStreamCreate(&ChunkMemoryStream);
        result = P64PulseStreamWriteMetaInfoToStream(&Instance->PulseStreams[HalfTrack], &ChunkMemoryStream);
        if (result)
        {
            ChunkSignature[0] = 'H';
            ChunkSignature[1] = 'T';
            ChunkSignature[2] = 'M';
            ChunkSignature[3] = HalfTrack;
            WriteChunk();
            result = WriteChunkResult;
        }
        P64MemoryStreamDestroy(&ChunkMemoryStream);
        if (!result)
        {
            break;
        }

        P64MemoryStreamCreate(&ChunkMemoryStream);
        result = P64PulseStreamWriteToStream(&Instance->PulseStreams[HalfTrack], &ChunkMemoryStream);
        if (result)
        {
            ChunkSignature[0] = 'H';
            ChunkSignature[1] = 'T';
            ChunkSignature[2] = 'P';
            ChunkSignature[3] = HalfTrack;
            WriteChunk();
            result = WriteChunkResult;
        }
        P64MemoryStreamDestroy(&ChunkMemoryStream);
        if (!result)
        {
            break;
        }
    }


    if (result)
    {

        P64MemoryStreamCreate(&ChunkMemoryStream);
        ChunkSignature[0] = 'D';
        ChunkSignature[1] = 'O';
        ChunkSignature[2] = 'N';
        ChunkSignature[3] = 'E';
        WriteChunk();
        result = WriteChunkResult;
        P64MemoryStreamDestroy(&ChunkMemoryStream);

        if (result)
        {
            result = 0;

            Header.Flags = 0;
            if (Instance->WriteProtected)
            {
                Header.Flags |= 1;
            }

            Header.Size = ChunksMemoryStream.Size;
            Header.Checksum = P64CRC32(ChunksMemoryStream.Data, Header.Size);

            P64EndianSwap(&Header.Version);
            P64EndianSwap(&Header.Flags);
            P64EndianSwap(&Header.Size);
            P64EndianSwap(&Header.Checksum);

            if (P64MemoryStreamWrite(&MemoryStream, (void*)&Header, sizeof(TP64Header)) == sizeof(TP64Header))
            {
                if (P64MemoryStreamSeek(&ChunksMemoryStream, 0) == 0)
                {
                    if (P64MemoryStreamAppendFromCount(&MemoryStream, &ChunksMemoryStream, ChunksMemoryStream.Size) == ChunksMemoryStream.Size)
                    {
                        if (P64MemoryStreamSeek(&MemoryStream, 0) == 0)
                        {
                            if (P64MemoryStreamAppendFromCount(Stream, &MemoryStream, MemoryStream.Size) == MemoryStream.Size)
                            {
                                result = 1;
                            }
                        }
                    }
                }
            }

        }

    }

    P64MemoryStreamDestroy(&ChunksMemoryStream);
    P64MemoryStreamDestroy(&MemoryStream);

#undef WriteChunk

    return result;
}




