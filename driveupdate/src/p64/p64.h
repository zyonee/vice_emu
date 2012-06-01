
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

#ifndef P64_H
#define P64_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* (16 MHz * 60) / 300 = 3200000 samples per track rotation (at 5 rotations per second) */
#define P64PulseSamplesPerRotation 3200000

#define P64FirstHalfTrack 2

/* including 42.5 */
#define P64LastHalfTrack 85

#pragma pack(push)
#pragma pack(1)

typedef uint8_t TP64Signature[8];

typedef TP64Signature* PP64Signature;

typedef uint8_t TP64ChunkSignature[4];

typedef TP64ChunkSignature* PP64ChunkSignature;

typedef struct
{
    TP64Signature Signature;
    uint32_t Version;
    uint32_t Flags;
    uint32_t Size;
    uint32_t Checksum;
} TP64Header;

typedef TP64Header* PP64Header;

typedef struct
{
    TP64ChunkSignature Signature;
    uint32_t Size;
    uint32_t Checksum;
} TP64ChunkHeader;

typedef TP64ChunkHeader* PP64ChunkHeader;

typedef struct
{
    uint32_t BitStreamLength;
    uint8_t SpeedZone;
} TP64HalfTrackMetaInfoHeader;

typedef TP64HalfTrackMetaInfoHeader* PP64HalfTrackMetaInfoHeader;

typedef struct
{
    uint32_t CountPulses;
    uint32_t Size;
} TP64HalfTrackHeader;

typedef TP64HalfTrackHeader* PP64HalfTrackHeader;

#pragma pack(pop)

typedef struct
{
    int32_t Previous;
    int32_t Next;
    uint32_t Position;
    uint32_t Strength;
} TP64Pulse;

typedef TP64Pulse* PP64Pulse;

typedef TP64Pulse* PP64Pulses;

typedef struct
{
    uint32_t BitStreamLength;
    uint8_t SpeedZone;
    PP64Pulses Pulses;
    uint32_t PulsesAllocated;
    uint32_t PulsesCount;
    int32_t UsedFirst;
    int32_t UsedLast;
    int32_t FreeList;
    int32_t CurrentIndex;
} TP64PulseStream;

typedef TP64PulseStream* PP64PulseStream;

typedef TP64PulseStream TP64PulseStreams[(P64LastHalfTrack-0)+2];

typedef TP64PulseStreams* PP64PulseStreams;

typedef struct
{
    TP64PulseStreams PulseStreams;
    uint32_t WriteProtected;
} TP64Image;

typedef TP64Image* PP64Image;

typedef struct
{
    uint8_t* Data;
    uint32_t Allocated;
    uint32_t Size;
    uint32_t Position;
} TP64MemoryStream;

typedef TP64MemoryStream* PP64MemoryStream;

extern void P64MemoryStreamCreate(PP64MemoryStream Instance);
extern void P64MemoryStreamDestroy(PP64MemoryStream Instance);
extern void P64MemoryStreamClear(PP64MemoryStream Instance);
extern uint32_t P64MemoryStreamSeek(PP64MemoryStream Instance, uint32_t Position);
extern uint32_t P64MemoryStreamRead(PP64MemoryStream Instance, uint8_t* Data, uint32_t Count);
extern uint32_t P64MemoryStreamWrite(PP64MemoryStream Instance, uint8_t* Data, uint32_t Count);
extern uint32_t P64MemoryStreamAssign(PP64MemoryStream Instance, PP64MemoryStream FromInstance);
extern uint32_t P64MemoryStreamAppend(PP64MemoryStream Instance, PP64MemoryStream FromInstance);
extern uint32_t P64MemoryStreamAppendFrom(PP64MemoryStream Instance, PP64MemoryStream FromInstance);
extern uint32_t P64MemoryStreamAppendFromCount(PP64MemoryStream Instance, PP64MemoryStream FromInstance, uint32_t Count);

extern void P64PulseStreamCreate(PP64PulseStream Instance);
extern void P64PulseStreamDestroy(PP64PulseStream Instance);
extern void P64PulseStreamClear(PP64PulseStream Instance);
extern int32_t P64PulseStreamAllocatePulse(PP64PulseStream Instance);
extern void P64PulseStreamFreePulse(PP64PulseStream Instance, int32_t Index);
extern void P64PulseStreamAddPulse(PP64PulseStream Instance, uint32_t Position, uint32_t Strength);
extern void P64PulseStreamRemovePulses(PP64PulseStream Instance, uint32_t Position, uint32_t Count);
extern void P64PulseStreamRemovePulse(PP64PulseStream Instance, uint32_t Position);
extern uint32_t P64PulseStreamDeltaPositionToNextPulse(PP64PulseStream Instance, uint32_t Position);
extern uint32_t P64PulseStreamGetNextPulse(PP64PulseStream Instance, uint32_t Position);
extern uint32_t P64PulseStreamGetPulseCount(PP64PulseStream Instance);
extern uint32_t P64PulseStreamGetPulse(PP64PulseStream Instance, uint32_t Position);
extern void P64PulseStreamSetPulse(PP64PulseStream Instance, uint32_t Position, uint32_t Strength);
extern void P64PulseStreamSeek(PP64PulseStream Instance, uint32_t Position);
extern void P64PulseStreamConvertFromGCR(PP64PulseStream Instance, uint8_t* Bytes, uint32_t Len);
extern void P64PulseStreamConvertToGCR(PP64PulseStream Instance, uint8_t* Bytes, uint32_t Len);
extern uint32_t P64PulseStreamConvertToGCRWithLogic(PP64PulseStream Instance, uint8_t* Bytes, uint32_t Len, uint32_t SpeedZone);
extern uint32_t P64PulseStreamReadMetaInfoFromStream(PP64PulseStream Instance, PP64MemoryStream Stream);
extern uint32_t P64PulseStreamWriteMetaInfoToStream(PP64PulseStream Instance, PP64MemoryStream Stream);
extern uint32_t P64PulseStreamReadFromStream(PP64PulseStream Instance, PP64MemoryStream Stream);
extern uint32_t P64PulseStreamWriteToStream(PP64PulseStream Instance, PP64MemoryStream Stream);

extern void P64ImageCreate(PP64Image Instance);
extern void P64ImageDestroy(PP64Image Instance);
extern void P64ImageClear(PP64Image Instance);
extern uint32_t P64ImageReadFromStream(PP64Image Instance, PP64MemoryStream Stream);
extern uint32_t P64ImageWriteToStream(PP64Image Instance, PP64MemoryStream Stream);

#endif
