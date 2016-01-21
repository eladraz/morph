/*
 * Copyright (c) 2008-2016, Integrity Project Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the Integrity Project nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE
 */

#include "xStl/data/datastream.h"
#include "xStl/stream/traceStream.h"
#include "format/MSILScanInterface.h"

void MSILScanInterface::scanMSIL(const uint8* msil, uint size)
{
    mdToken token;
    int offset;
    for (uint i = 0; i < size;)
    {
        if (msil[i] == 0xFE) //  2 bytes opcode
        {
            switch (msil[++i])
            {
                case 0x01: // ceq
                case 0x02: // cgt
                case 0x03: // cgt.un
                case 0x04: // clt
                case 0x05: // clt.un
                case 0x0F: // localloc
                    // No extra data to read
                    i++;
                    continue;

                case 0x09: // ldarg
                case 0x0A: // ldarga
                case 0x0B: // starg
                case 0x0C: // ldloc
                case 0x0D: // ldloca
                case 0x0E: // stloc
                    // opcode + 16bit immediate
                    i++;
                    i+= 2;
                    continue;

                case 0x15: // initobj
                case 0x16: // constrained
                case 0x1C: // sizeof
                case 0x06: // ldftn
                case 0x07: // ldvirtftn
                    i++;
                    // Add token
                    token = cLittleEndian::readUint32(msil + i);
                    OnToken(token);
                    i+= 4;
                    continue;
                default:
                    traceHigh("MSILScanInterface: Unknown opcode - FE " << HEXBYTE(msil[i]) << endl);
                    CHECK_FAIL();
            }
        } else switch (msil[i])
        {
            case 0: // nop
            case 1: // break;
            case 2: case 3: case 4: case 5: // ldarg 0-3. Load argument into stack
            case 6: case 7: case 8: case 9: // ldloc 0-3.
            case 0x0A: case 0x0B: case 0x0C: case 0x0D: // // stloc 0-3
            case 0x14:  // ldnull
            case 0x15: case 0x16: case 0x17: case 0x18: // ldc.-1..2
            case 0x19: case 0x1A: case 0x1B: case 0x1C:
            case 0x1D: case 0x1E:                       // ldc.8
            case 0x25: // dup
            case 0x26: // pop
            case 0x2A: // ret
            case 0x46: // ldind.i1
            case 0x47: // ldind.u1
            case 0x48: // ldind.i2
            case 0x49: // ldind.u2
            case 0x4A: // ldind.i4
            case 0x4B: // ldind.u4
            case 0x4C: // ldind.i8.u8
            case 0x4D: // ldind.i
            case 0x4E: // ldind.r4
            case 0x4F: // ldind.r8
            case 0x50: // ldind.ref
            case 0x51: // stind.i1
            case 0x52: // stind.i1
            case 0x53: // stind.i2
            case 0x54: // stind.i4
            case 0x55: // stind.i8
            case 0x56: // stind.r4
            case 0x57: // stind.r8
            case 0xDF: // stind.i
            case 0x58: // add
            case 0x59: // sub
            case 0x5A: // mul
            case 0xD9: // mul.ovf.un
            case 0x5B: // div
            case 0x5C: // div.un
            case 0x5D: // rem
            case 0x5E: // rem.un
            case 0x5F: // and
            case 0x60: // or
            case 0x61: // xor
            case 0x62: // shl
            case 0x63: // shr
            case 0x64: // shr.un
            case 0x65: // neg
            case 0x66: // not
            case 0x67: // conv.i1
            case 0x68: // conv.i2
            case 0x69: // conv.i4
            case 0x6A: // conv.i8
            case 0x6B: // conv.r4
            case 0x6C: // conv.r8
            case 0x6D: // conv.u4
            case 0x6E: // conv.u8
            case 0x76: // conv.r.un
            case 0x7a: // throw
            case 0x8E: // ldlen
            case 0x90: // ldelem.i1
            case 0x91: // ldelem.u1
            case 0x92: // ldelem.i2
            case 0x93: // ldelem.u2
            case 0x94: // ldelem.i4
            case 0x95: // ldelem.u4
            case 0x96: // ldelem.i8 -> Not implemented
            case 0x97: // ldelem.i
            case 0x98: // ldelem.r4 -> Not implemented
            case 0x99: // ldelem.r8 -> Not implemented
            case 0x9b: // stelem.i
            case 0x9c: // stelem.i1
            case 0x9d: // stelem.i2
            case 0x9e: // stelem.i4
            case 0x9f: // stelem.i8 -> Not implemented
            case 0xA0: // stelem.r4 -> Not implemented
            case 0xA1: // stelem.r8 -> Not implemented
            case 0x9A: // ldelem.ref   Load the element of type object, at index onto
            case 0xA2: // stelem.ref
            case 0xD1: // conv.u2
            case 0xD2: // conv.u1
            case 0xD3: // conv.i
            case 0xDC: // endfinally
            case 0xE0: // conv.u
                // No extra data to read
                i++;
                continue;

            case 0x0E: // ldarg.s
            case 0x11: // ldloc.s
            case 0x13: // stloc.s (uint8)
            case 0x0F: // ldarga.s
            case 0x10: // starg (uint8)
            case 0x12: // ldloca.s
            case 0x1F: // ldc.u8
                // Read opcode + index
                i+= 2;
                continue;

            case 0x2B:  // br.s <int8>
            case 0xDE: // leave <int8>
                // Unconditional branch
                // Read offset
                offset = (char)msil[i + 1];
                // Skip opcode + offset
                i += 2;
                // Trigger the callback
                OnOffset(i+offset);
                continue;

            case 0x2C: // brfalse.s
            case 0x2D: // brtrue.s
            case 0x2E:  // beq.s  <int8>
            case 0x2F:  // bge.s <int8>
            case 0x34:  // bge.un.s
            case 0x30:  // bgt.s <int8>
            case 0x31:  // ble.s  <int8>
            case 0x32:  // blt.s  <int8>
            case 0x33:  // bne.un.s <int8>
            case 0x35:  // bgt.un.s
            case 0x36:  // ble.un.s
            case 0x37:  // blt.un.s
                // Conditional branch
                // Read offset
                offset = (char)msil[i + 1];
                // Skip opcode + offset
                i += 2;
                // Trigger the callback for both code paths
                OnOffset(i);
                OnOffset(i+offset);
                continue;

            case 0x38:  // br   <int32>
            case 0x39:  // brfalse
            case 0x3A:  // brtrue
            case 0x3B:  // beq
            case 0x3C:  // bge   <int32>
            case 0x3D:  // bgt   <int32>
            case 0x3E:  // ble    <int32>
            case 0x3F:  // blt    <int32>
            case 0x40:  // bne.un   <int32>
            case 0x41:  // bge.un
            case 0x42:  // bgt.un
            case 0x43:  // ble.un
            case 0x44:  // blt.un
            case 0xDD: // leave <int32>
                // Conditional branch
                // Read offset
                offset = *(int*)(msil + i + 1);
                // Skip opcode + 4 bytes offset
                i+= 5;
                // Trigger the callback for both code paths
                OnOffset(i);
                OnOffset(i+offset);
                continue;

            case 0x20:  // ldc.i4
            case 0x22:  // ldc.r4
            case 0x79: // unbox
            case 0xA5: // unbox.any
            case 0x7B: // ldfld <fieldToken>
            case 0x7C: // ldflda <fieldToken>
            case 0x7D: // stfld <fieldToken>
            case 0x7E: // ldsfld <fieldToken>
            case 0x7F: // ldsflda <fieldToken>
            case 0x80: // stsfld <fieldToken>
            case 0x81: // stobj <token>
            case 0x8C: // box <valueType>
            case 0x8D: // newarr <token>
            case 0x8F: // ldelema <T>
            case 0xA3: // ldelem
            case 0xA4: // stelem
                // Read opcode + 4 bytes operand
                i+= 5;
                continue;

            case 0x70: // cpobj <token>
            case 0x71: // ldobj <token>
            case 0x72: // ldstr <token>
            case 0x28: // call
            case 0x6F: // callvirt
            case 0x73: // newobj <T>
            case 0x74: // castclass <T>
            case 0x75: // isinst <T>
                i++;
                // Add token
                token = cLittleEndian::readUint32(msil + i);
                OnToken(token);
                i+= 4;
                continue;

            case 0xD0: // ldtoken
                i++;
                // Add token
                token = cLittleEndian::readUint32(msil + i);
                OnToken(token, true);
                i+= 4;
                continue;

            case 0x21: // ldc.i8
            case 0x23: // ldc.r8
                // Read opcode + 8 bytes opcode
                i+= 9;
                continue;

            case 0x45: // switch - Not implemented yet
            default:
                traceHigh("MSILScanInterface: Unknown opcode - " << HEXBYTE(msil[i]) << endl);
                CHECK_FAIL();
        }
    }
}

void MSILScanInterface::OnToken(mdToken token, bool bLdToken)
{
}

void MSILScanInterface::OnOffset(uint instructionIndex)
{
}
