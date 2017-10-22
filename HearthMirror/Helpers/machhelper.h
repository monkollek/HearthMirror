//
//  machhelper.h
//  MonoReader
//
//  Created by Istvan Fehervari on 22/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#ifndef machhelper_h
#define machhelper_h

#include <mach-o/dyld_images.h>

typedef mach_port_t HANDLE;
typedef mach_vm_address_t proc_address;

bool ReadBytes(HANDLE task, proc_address buf, uint32_t size, proc_address address);

proc_address ReadPointer(const HANDLE task, const proc_address address, const bool is64bit);
uint64_t ReadUInt64(HANDLE task, proc_address address);
int64_t ReadInt64(HANDLE task, proc_address address);
uint32_t ReadUInt32(HANDLE task, proc_address address);
int32_t ReadInt32(HANDLE task, proc_address address);
char* ReadCString(HANDLE task, proc_address address);
bool ReadBool(HANDLE task, proc_address address);
uint8_t ReadByte(HANDLE task, proc_address address);
int8_t ReadSByte(HANDLE task, proc_address address);
int16_t ReadShort(HANDLE task, proc_address address);
uint16_t ReadUShort(HANDLE task, proc_address address);

float ReadFloat(HANDLE task, proc_address address);
double ReadDouble(HANDLE task, proc_address address);

proc_address getLibLoadAddress(HANDLE task, const char* libname, bool* is64bit);
proc_address getMonoLoadAddress(HANDLE task, bool* is64bit);
proc_address getMonoRootDomainAddr(HANDLE task, proc_address baseAddress, bool is64bit);

#endif /* machhelper_h */
