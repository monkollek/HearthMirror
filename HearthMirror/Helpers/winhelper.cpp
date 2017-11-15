//
//  winhelper.cpp
//  windows memory reading functions
//
//  Created by Istvan Fehervari on 29/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#include "winhelper.h"
#include <psapi.h>
#include "offsets.h"

// debug only imports
#include <stdio.h>
#include <tchar.h>

proc_address getMonoLoadAddress(HANDLE task, bool* is64bit) {

	HMODULE hMods[1024];
	DWORD cbNeeded;
	unsigned int i;
	TCHAR szModName[MAX_PATH];

	*is64bit = false;

	// enumerate all modules loaded by the process
	if (EnumProcessModules(task, hMods, sizeof(hMods), &cbNeeded)) {
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			GetModuleBaseName(task, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR));

			//_tprintf(TEXT("%s\n"), szModName);
			if (wcscmp(szModName,L"mono.dll") == 0) {
				MODULEINFO modinfo;
				GetModuleInformation(task, hMods[i], &modinfo, sizeof(MODULEINFO));
				return (proc_address)modinfo.lpBaseOfDll;
			}
		}
	}
	return NULL;
}

proc_address getMonoRootDomainAddr(HANDLE task, proc_address baseAddress, bool is64bit) {
	// PE header
	// IMAGE_DOS_HEADER.e_lfanew
	int32_t e_lfanew = ReadInt32(task, baseAddress+kImageDosHeader_e_lfanew);

	// IMAGE_FILE_HEADER.Signature
	int32_t sig = ReadInt32(task, baseAddress + e_lfanew + kImageNTHeadersSignature);
	if (sig != 0x4550) {
		return NULL;
	}

	// IMAGE_FILE_HEADER.Machine -- check to make sure this is 32-bit
	uint16_t machine = ReadUShort(task, baseAddress + e_lfanew + kImageNTHeadersMachine);
	if (machine != 0x14c) {
		return NULL;
	}

	int32_t _exportOffset = ReadInt32(task, baseAddress + e_lfanew + kImageNTHeadersExportDirectoryAddress);

	if (_exportOffset <= 0 /*&& _exportOffset < _module.Length*/) {
		return NULL;
	}

	// get export offset
	int32_t nFunctions = ReadInt32(task, baseAddress + _exportOffset + kImageExportDirectoryNumberOfFunctions);
	int32_t ofsFunctions = ReadInt32(task, baseAddress + _exportOffset + kImageExportDirectoryAddressOfFunctions);
	int32_t ofsNames = ReadInt32(task, baseAddress + _exportOffset + kImageExportDirectoryAddressOfNames);
	int32_t rootDomainFunc = 0;
	for (int32_t i = 0; i < nFunctions; i++) {
		int32_t nameRva = ReadInt32(task, baseAddress + ofsNames + 4 * i);
		char* fName = ReadCString(task, baseAddress + nameRva);
		if (0 == strcmp(fName, "mono_get_root_domain")) {
			rootDomainFunc = baseAddress + ReadInt32(task, baseAddress + ofsFunctions + 4 * i);
			free(fName);
			break;
		}
		free(fName);
	}
	
	if (rootDomainFunc == 0) return NULL;

	uint8_t* buffer = new uint8_t[6];
	ReadBytes(task, (proc_address)buffer, 6, rootDomainFunc);
	if (buffer[0] != 0xa1 || buffer[5] != 0xc3) {
		delete[] buffer;
		return NULL;
	}
	uint32_t pRootDomain;
	memcpy(&pRootDomain, buffer+1, sizeof(pRootDomain));

	return pRootDomain;
}

bool ReadBytes(HANDLE task, proc_address buf, uint32_t size, proc_address address) {
	int res = ReadProcessMemory(task, (void*)address, (void*)buf, size, 0);
	return res == 0;
}

proc_address ReadPointer(const HANDLE task, const proc_address address, const bool is64bit) {
	if (is64bit) {
		return ReadUInt64(task, address);
	}
	return ReadUInt32(task, address);
}

//TODO: use template instead
uint64_t ReadUInt64(HANDLE task, proc_address address) {
	uint64_t value = 0;
	ReadProcessMemory(task, (void*)address, &value, sizeof(value), 0);

	return value;
}

int64_t ReadInt64(HANDLE task, proc_address address) {
	int64_t value = 0;
	ReadProcessMemory(task, (void*)address, &value, sizeof(value), 0);

	return value;
}

uint32_t ReadUInt32(HANDLE task, proc_address address) {
	uint32_t value = 0;
	ReadProcessMemory(task, (void*)address, &value, sizeof(value), 0);

	return value;
}

int32_t ReadInt32(HANDLE task, proc_address address) {
	int32_t value = 0;
	ReadProcessMemory(task, (void*)address, &value, sizeof(value), 0);

	return value;
}

#define kRemoteStringBufferSize 2048
char* ReadCString(HANDLE task, proc_address address) {
	char buf[kRemoteStringBufferSize] = { 0 };
	size_t size = sizeof(buf);
	ReadProcessMemory(task, (void*)address, &buf, size, 0);

	// add ending
	buf[kRemoteStringBufferSize - 1] = '\0';

	char *result = _strdup(buf);

	return result;
}

bool ReadBool(HANDLE task, proc_address address) {
	bool value = 0;
	ReadProcessMemory(task, (void*)address, &value, sizeof(value), 0);

	return value;
}

uint8_t ReadByte(HANDLE task, proc_address address) {
	uint8_t value = 0;
	ReadProcessMemory(task, (void*)address, &value, sizeof(value), 0);

	return value;
}

int8_t ReadSByte(HANDLE task, proc_address address) {
	int8_t value = 0;
	ReadProcessMemory(task, (void*)address, &value, sizeof(value), 0);

	return value;
}

int16_t ReadShort(HANDLE task, proc_address address) {
	int16_t value = 0;
	ReadProcessMemory(task, (void*)address, &value, sizeof(value), 0);

	return value;
}

uint16_t ReadUShort(HANDLE task, proc_address address) {
	uint16_t value = 0;
	ReadProcessMemory(task, (void*)address, &value, sizeof(value), 0);

	return value;
}

float ReadFloat(HANDLE task, proc_address address) {
	float value = 0;
	ReadProcessMemory(task, (void*)address, &value, sizeof(value), 0);

	return value;
}

double ReadDouble(HANDLE task, proc_address address) {
	double value = 0;
	ReadProcessMemory(task, (void*)address, &value, sizeof(value), 0);

	return value;
}

