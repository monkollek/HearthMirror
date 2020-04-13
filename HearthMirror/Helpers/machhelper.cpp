//
//  machhelper.cpp
//  MacOS memory reading functions
//
//  Created by Istvan Fehervari on 22/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#include "machhelper.h"
#include <mach/mach_vm.h>
#include <mach/error.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <MacTypes.h>
#include <stdexcept>
#include <stdlib.h>

struct dyld_image_info_32 {
    uint32_t imageLoadAddress;
    uint32_t imageFilePath;
    uint32_t imageFileModDate;
};

struct load_command_with_segname {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t segname;
};

struct dyld_image_info_64 {
    uint64_t imageLoadAddress;
    uint64_t imageFilePath;
    uint64_t imageFileModDate;
};

struct dyld_all_image_infos_32 {
    uint32_t version;
    uint32_t infoArrayCount;
    uint32_t infoArray;
    uint32_t notification;
    bool processDetachedFromSharedRegion;
    bool libSystemInitialized;
    uint32_t dyldImageLoadAddress;
    uint32_t jitInfo;
    uint32_t dyldVersion;
    uint32_t errorMessage;
    uint32_t terminationFlags;
    uint32_t coreSymbolicationShmPage;
    uint32_t systemOrderFlag;
    uint32_t uuidArrayCount;
    uint32_t uuidArray;
    uint32_t dyldAllImageInfosAddress;
    uint32_t initialImageCount;
    uint32_t errorKind;
    uint32_t errorClientOfDylibPath;
    uint32_t errorTargetDylibPath;
    uint32_t errorSymbol;
    uint32_t sharedCacheSlide;
    uint8_t	 sharedCacheUUID[16];
    uint32_t reserved[16];
};

struct dyld_all_image_infos_64 {
    uint32_t version;
    uint32_t infoArrayCount;
    uint64_t infoArray;
    uint64_t notification;
    int32_t processDetachedFromSharedRegion;
    int32_t libSystemInitialized;
    uint64_t dyldImageLoadAddress;
    uint64_t jitInfo;
    uint64_t dyldVersion;
    uint64_t errorMessage;
    uint64_t terminationFlags;
    uint64_t coreSymbolicationShmPage;
    uint64_t systemOrderFlag;
    uint64_t uuidArrayCount;
    uint64_t uuidArray;
    uint64_t dyldAllImageInfosAddress;
    uint64_t initialImageCount;
    uint64_t errorKind;
    uint64_t errorClientOfDylibPath;
    uint64_t errorTargetDylibPath;
    uint64_t errorSymbol;
    uint64_t sharedCacheSlide;
    uint8_t	 sharedCacheUUID[16];
    uint64_t reserved[16];
};

#if __LP64__
//#define mem_address mach_vm_address_t
#else
//#define mem_address mach_vm_address_t
#endif

/** @see _copyin_string() */
#define kRemoteStringBufferSize 2048

uint32_t findLibBaseAddress32(mach_port_t task, const char* libname, task_dyld_info_data_t dyld_info) {
    
    vm_address_t address = (vm_address_t)dyld_info.all_image_info_addr;
    mach_msg_type_number_t size = sizeof(struct dyld_all_image_infos);
    vm_offset_t readMem;
    vm_read(task,address,size,&readMem,&size);
    
    void* pImageInfos = malloc(size);
    memcpy(pImageInfos, (void*)readMem, size);
    struct dyld_all_image_infos_32* infos = (struct dyld_all_image_infos_32 *) pImageInfos;
    vm_deallocate(mach_task_self(), readMem, size);

    size = sizeof(struct dyld_image_info_32) * infos->infoArrayCount;
    vm_read(task,(vm_address_t) infos->infoArray,size,&readMem,&size);
    void* pInfoArray = malloc(size);
    memcpy(pInfoArray, (void*)readMem, size);
    struct dyld_image_info_32* info = (struct dyld_image_info_32*) pInfoArray;
    vm_deallocate(mach_task_self(), readMem, size);
    
    mach_msg_type_number_t sizeMax=512;
    for (int i=0; i < infos->infoArrayCount; i++) {
        vm_read(task,(vm_address_t) info[i].imageFilePath,sizeMax,&readMem,&sizeMax);
        char *path = (char *) readMem;
        if (strstr(path, libname) != NULL) {
            vm_deallocate(mach_task_self(), readMem, sizeMax);
            free(pImageInfos);
            uint32_t loadAddress = info[i].imageLoadAddress;
            free(pInfoArray);
            return loadAddress;
        }
        vm_deallocate(mach_task_self(), readMem, sizeMax);
    }
    free(pImageInfos);
    free(pInfoArray);
    return NULL;
}

uint64_t findLibBaseAddress64(mach_port_t task, const char* libname, task_dyld_info_data_t dyld_info) {
    //printf("In findLibBaseAddress64 - 1\n");
    vm_address_t address = (vm_address_t)dyld_info.all_image_info_addr;
    mach_msg_type_number_t size = sizeof(struct dyld_all_image_infos);
    vm_offset_t readMem;
    vm_read(task,address,size,&readMem,&size);
    
    struct dyld_all_image_infos_64* infos = (struct dyld_all_image_infos_64 *) readMem;
    size = sizeof(struct dyld_image_info_64) * infos->infoArrayCount;
    vm_read(task,(vm_address_t) infos->infoArray,size,&readMem,&size);
    
    struct dyld_image_info_64* info = (struct dyld_image_info_64*) readMem;
    mach_msg_type_number_t sizeMax=512;
    
    for (int i=0; i < infos->infoArrayCount; i++) {
        vm_read(task,(vm_address_t) info[i].imageFilePath,sizeMax,&readMem,&sizeMax);
        char *path = (char *) readMem;
        if (strstr(path, libname) != NULL) {
            return info[i].imageLoadAddress;
        }
    }
    return NULL;
}

proc_address getLibLoadAddress(mach_port_t task, const char* libname, bool* is64bit) {
    //printf("In getLibLoadAddress - 1\n");
    task_dyld_info_data_t dyld_info;
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    if (task_info(task, TASK_DYLD_INFO, (task_info_t)&dyld_info, &count) == KERN_SUCCESS)
    {
        if (dyld_info.all_image_info_format == TASK_DYLD_ALL_IMAGE_INFO_32) {
            *is64bit = false;
            return findLibBaseAddress32(task, libname, dyld_info);
        } else {
            //printf("In getLibLoadAddress - 2 -- case 64bit\n");
            *is64bit = true;
            return findLibBaseAddress64(task, libname, dyld_info);
        }
    }
    
    return NULL;
}

proc_address getMonoLoadAddress(HANDLE task, bool* is64bit) {
    return getLibLoadAddress(task, "libmono.0.dylib", is64bit);
}

proc_address getMonoRootDomainAddr(HANDLE task, proc_address baseAddress, bool is64bit) {
    // lookup "mono_root_domain"
    const char *symbol_name = "mono_root_domain";
    
    int err = KERN_FAILURE;
    
    if (baseAddress == 0) {
        return 0;
    }
    
    bool sixtyfourbit;
    mach_vm_size_t size;
    uint32_t num_load_cmds;
    
    if (is64bit) {
        size = sizeof(struct mach_header_64);
        struct mach_header_64 header = {0};
        
        err = mach_vm_read_overwrite(task, baseAddress, size, (mach_vm_address_t)&header, &size);
        if (err != KERN_SUCCESS) return 0;
        
        if (header.magic != MH_MAGIC && header.magic != MH_MAGIC_64) {
            return 0;
        }
        
        sixtyfourbit = (header.magic == MH_MAGIC_64);
        num_load_cmds = header.ncmds;
    } else {
        size = sizeof(struct mach_header);
        struct mach_header header = {0};
        
        err = mach_vm_read_overwrite(task, baseAddress, size, (mach_vm_address_t)&header, &size);
        if (err != KERN_SUCCESS) return 0;
        
        if (header.magic != MH_MAGIC && header.magic != MH_MAGIC_64) {
            return 0;
        }
        
        sixtyfourbit = (header.magic == MH_MAGIC_64);
        num_load_cmds = header.ncmds;
    }
    
    mach_vm_address_t symtab_addr = 0;
    mach_vm_address_t linkedit_addr = 0;
    mach_vm_address_t text_addr = 0;
    mach_vm_address_t data_addr = 0;
    
    size_t mach_header_size = sizeof(struct mach_header);
    if (sixtyfourbit) {
        mach_header_size = sizeof(struct mach_header_64);
    }
    mach_vm_address_t command_addr = baseAddress + mach_header_size;
    struct load_command command = {0};
    size = sizeof(command);
    
    for (uint32_t i = 0; i < num_load_cmds; i++) {
        err = mach_vm_read_overwrite(task, command_addr, size, (mach_vm_address_t)&command, &size);
        if (err != KERN_SUCCESS) return 0;
        
        if (command.cmd == LC_SYMTAB) {
            symtab_addr = command_addr;
        } else if (command.cmd == LC_DYSYMTAB) {
        } else if (command.cmd == LC_SEGMENT || command.cmd == LC_SEGMENT_64) {
            /* struct load_command only has two fields (cmd & cmdsize), while its "child" type
             * struct segment_command has way more fields including `segname` at index 3, so we just
             * pretend that we have a real segment_command and skip first two fields away */
            size_t segname_field_offset = sizeof(command);
            mach_vm_address_t segname_addr = command_addr + segname_field_offset;
            char *segname = ReadCString(task, segname_addr);//_copyin_string(task, segname_addr);
            if (0 == strcmp(SEG_TEXT, segname)) {
                text_addr = command_addr;
            } else if (0 == strcmp(SEG_DATA, segname)) {
                data_addr = command_addr;
            } else if (0 == strcmp(SEG_LINKEDIT, segname)) {
                linkedit_addr = command_addr;
            }
            free(segname);
        }
        // go to next load command
        command_addr += command.cmdsize;
    }
    
    if (!symtab_addr || !linkedit_addr || !text_addr) {
        return 0;
    }
    
    struct symtab_command symtab = {0};
    size = sizeof(struct symtab_command);
    err = mach_vm_read_overwrite(task, symtab_addr, size, (mach_vm_address_t)&symtab, &size);
    if (err != KERN_SUCCESS) return 0;
    
    if (sixtyfourbit) {
        struct segment_command_64 linkedit = {0};
        size = sizeof(struct segment_command_64);
        err = mach_vm_read_overwrite(task, linkedit_addr, size,
                                     (mach_vm_address_t)&linkedit, &size);
        if (err != KERN_SUCCESS) return 0;
        struct segment_command_64 text = {0};
        err = mach_vm_read_overwrite(task, text_addr, size, (mach_vm_address_t)&text, &size);
        if (err != KERN_SUCCESS) return 0;
        
        uint64_t file_slide = linkedit.vmaddr - text.vmaddr - linkedit.fileoff;
        uint64_t strings = baseAddress + symtab.stroff + file_slide;
        uint64_t sym_addr = baseAddress + symtab.symoff + file_slide;
        
        for (uint32_t i = 0; i < symtab.nsyms; i++) {
            struct nlist_64 sym = {{0}};
            size = sizeof(struct nlist_64);
            err = mach_vm_read_overwrite(task, sym_addr, size, (mach_vm_address_t)&sym, &size);
            if (err != KERN_SUCCESS) return 0;
            sym_addr += size;
            
            if (!sym.n_value) continue;
            
            uint64_t symname_addr = strings + sym.n_un.n_strx;
            char *symname = ReadCString(task, symname_addr);
            // Ignore the leading "_" character in a symbol name
            if (0 == strcmp(symbol_name, symname+1)) {
                free(symname);
                return (mach_vm_address_t)sym.n_value;
            }
            free(symname);
        }
    } else {
        struct segment_command linkedit = {0};
        size = sizeof(struct segment_command);
        err = mach_vm_read_overwrite(task, linkedit_addr, size,
                                     (mach_vm_address_t)&linkedit, &size);
        if (err != KERN_SUCCESS) return 0;
        struct segment_command text = {0};
        err = mach_vm_read_overwrite(task, text_addr, size, (mach_vm_address_t)&text, &size);
        if (err != KERN_SUCCESS) return 0;
        
        if (data_addr != 0) {
            struct segment_command data = {0};
            err = mach_vm_read_overwrite(task, data_addr, size, (mach_vm_address_t)&data, &size);
            if (err != KERN_SUCCESS) return 0;
        }
        uint32_t file_slide = linkedit.vmaddr - text.vmaddr - linkedit.fileoff;
        uint32_t strings = (uint32_t)baseAddress + symtab.stroff + file_slide;
        uint32_t sym_addr = (uint32_t)baseAddress + symtab.symoff + file_slide;
        
        for (uint32_t i = 0; i < symtab.nsyms; i++) {
            struct nlist sym = {{0}};
            size = sizeof(struct nlist);
            err = mach_vm_read_overwrite(task, sym_addr, size, (mach_vm_address_t)&sym, &size);
            if (err != KERN_SUCCESS) return 0;
            sym_addr += size;
            
            if (!sym.n_value) continue;
            
            uint32_t symname_addr = strings + sym.n_un.n_strx;
            char *symname = ReadCString(task, symname_addr);
            /* Ignore the leading "_" character in a symbol name */
            if (0 == strcmp(symbol_name, symname+1)) {                
                free(symname);
                return (mach_vm_address_t)sym.n_value;
            }
            free(symname);
        }
    }
    
    return 0;
}

float ToFloat(Byte* buffer, int start=0) {
    float f;
    memcpy(&f, &buffer, sizeof(f));
    return f;
}

double ToDouble(Byte* buffer, int start=0) {
    double f;
    memcpy(&f, &buffer, sizeof(f));
    return f;
}

proc_address ReadPointer(const HANDLE task, const proc_address address, const bool is64bit) {
    if (is64bit) {
        return ReadUInt64(task, address);
    }
    return ReadUInt32(task, address);
}

uint64_t ReadUInt64(HANDLE task, mach_vm_address_t address) {
    if (address == 0) throw std::runtime_error("Pointer is NULL");
    
    vm_offset_t readMem = NULL;
    vm_size_t size = 8;
    mach_msg_type_number_t data_read = 0;
    kern_return_t err = mach_vm_read(task, address, size, &readMem, &data_read);
    if (err != KERN_SUCCESS) {
        printf("Error code: %x", err);
        throw std::runtime_error("Could not read memory region");
    }
    
    uint64_t v = 0;
    memcpy((char *)&v, (Byte*)readMem, sizeof(uint64_t));
    mach_vm_deallocate(mach_task_self(), readMem, size);
    return v;
}

int64_t ReadInt64(HANDLE task, mach_vm_address_t address) {
    if (address == 0) throw std::runtime_error("Pointer is NULL");
    
    vm_offset_t readMem = NULL;
    vm_size_t size = 8;
    mach_msg_type_number_t data_read = 0;
    kern_return_t err = mach_vm_read(task,address,size,&readMem,&data_read);
    if (err != KERN_SUCCESS) {
        throw std::runtime_error("Could not read memory region");
    }
    
    int64_t v = 0;
    memcpy((char *)&v, (Byte*)readMem, sizeof(int64_t));
    mach_vm_deallocate(mach_task_self(), readMem, size);
    return v;
}

uint32_t ReadUInt32(HANDLE task, mach_vm_address_t address) {
    if (address == 0) throw std::runtime_error("Pointer is NULL");
    
    vm_offset_t readMem = NULL;
    vm_size_t size = 4;
    mach_msg_type_number_t data_read = 0;
    kern_return_t err = mach_vm_read(task,address,size,&readMem,&data_read);
    if (err != KERN_SUCCESS) {
        throw std::runtime_error("Could not read memory region");
    }
    
    uint32_t v = 0;
    memcpy((UInt8*)&v, (UInt8*)readMem, data_read);
    mach_vm_deallocate(mach_task_self(), readMem, data_read);
    return v;
}

int32_t ReadInt32(HANDLE task, mach_vm_address_t address) {
    if (address == 0) throw std::runtime_error("Pointer is NULL");
    
    vm_offset_t readMem = NULL;
    vm_size_t size = 4;
    mach_msg_type_number_t data_read = 0;
    kern_return_t err = mach_vm_read(task,address,size,&readMem,&data_read);
    if (err != KERN_SUCCESS) {
        throw std::runtime_error("Could not read memory region");
    }
    
    int32_t v = 0;
    memcpy((void *)&v, (void*)readMem, data_read);
    mach_vm_deallocate(mach_task_self(), readMem, data_read);
    return v;
}

bool ReadBool(HANDLE task, mach_vm_address_t address) {
    if (address == 0) throw std::runtime_error("Pointer is NULL");
    
    vm_offset_t readMem = NULL;
    vm_size_t size = 1;
    mach_msg_type_number_t data_read = 0;
    kern_return_t err = mach_vm_read(task,address,size,&readMem,&data_read);
    if (err != KERN_SUCCESS) {
        throw std::runtime_error("Could not read memory region");
    }
    
    Byte* buffer = (Byte*)readMem;
    bool result = (bool)buffer[0];
    mach_vm_deallocate(mach_task_self(), readMem, data_read);
    return result;
}

uint8_t ReadByte(HANDLE task, mach_vm_address_t address) {
    if (address == 0) throw std::runtime_error("Pointer is NULL");
    
    vm_offset_t readMem = NULL;
    vm_size_t size = 1;
    mach_msg_type_number_t data_read = 0;
    kern_return_t err = mach_vm_read(task,address,size,&readMem,&data_read);
    if (err != KERN_SUCCESS) {
        throw std::runtime_error("Could not read memory region");
    }
    
    Byte* buffer = (Byte*)readMem;
    uint8_t result = (Byte)buffer[0];
    mach_vm_deallocate(mach_task_self(), readMem, data_read);
    return result;
}

int8_t ReadSByte(HANDLE task, mach_vm_address_t address) {
    if (address == 0) throw std::runtime_error("Pointer is NULL");
    
    vm_offset_t readMem = NULL;
    vm_size_t size = 1;
    mach_msg_type_number_t data_read = 0;
    kern_return_t err = mach_vm_read(task,address,size,&readMem,&data_read);
    if (err != KERN_SUCCESS) {
        throw std::runtime_error("Could not read memory region");
    }
    
    SignedByte* buffer = (SignedByte*)readMem;
    int8_t result = (SignedByte)buffer[0];
    mach_vm_deallocate(mach_task_self(), readMem, data_read);
    return result;
}

int16_t ReadShort(HANDLE task, mach_vm_address_t address) {
    if (address == 0) throw std::runtime_error("Pointer is NULL");
    
    vm_offset_t readMem = NULL;
    vm_size_t size = 2;
    mach_msg_type_number_t data_read = 0;
    kern_return_t err = mach_vm_read(task,address,size,&readMem,&data_read);
    if (err != KERN_SUCCESS) {
        throw std::runtime_error("Could not read memory region");
    }
    
    int16_t v = 0;
    memcpy((char *)&v, (Byte*)readMem, sizeof(int16_t));
    mach_vm_deallocate(mach_task_self(), readMem, data_read);
    return v;
}

uint16_t ReadUShort(HANDLE task, mach_vm_address_t address) {
    if (address == 0) throw std::runtime_error("Pointer is NULL");
    
    vm_offset_t readMem = NULL;
    vm_size_t size = 2;
    mach_msg_type_number_t data_read = 0;
    kern_return_t err = mach_vm_read(task,address,size,&readMem,&data_read);
    if (err != KERN_SUCCESS) {
        throw std::runtime_error("Could not read memory region");
    }
    
    uint16_t v = 0;
    memcpy((void *)&v, (void*)readMem, data_read);
    mach_vm_deallocate(mach_task_self(), readMem, data_read);
    return v;
}

float ReadFloat(HANDLE task, mach_vm_address_t address) {
    if (address == 0) throw std::runtime_error("Pointer is NULL");
    
    vm_offset_t readMem = NULL;
    vm_size_t size = 4;
    mach_msg_type_number_t data_read = 0;
    kern_return_t err = mach_vm_read(task,address,size,&readMem,&data_read);
    if (err != KERN_SUCCESS) {
        throw std::runtime_error("Could not read memory region");
    }
    
    Byte* buffer = (Byte*)readMem;
    float result = ToFloat((Byte*)buffer);
    mach_vm_deallocate(mach_task_self(), readMem, data_read);
    return result;
}

double ReadDouble(HANDLE task, mach_vm_address_t address) {
    if (address == 0) throw std::runtime_error("Pointer is NULL");
    
    vm_offset_t readMem = NULL;
    vm_size_t size = 8;
    mach_msg_type_number_t data_read = 0;
    kern_return_t err = mach_vm_read(task,address,size,&readMem,&data_read);
    if (err != KERN_SUCCESS) {
        throw std::runtime_error("Could not read memory region");
    }
    
    Byte* buffer = (Byte*)readMem;
    double result = ToDouble((Byte*)buffer);
    mach_vm_deallocate(mach_task_self(), readMem, data_read);
    return result;
}

bool ReadBytes(HANDLE task, proc_address buf, uint32_t size, mach_vm_address_t address) {
    if (address == 0) throw std::runtime_error("Pointer is NULL");
    
    vm_offset_t readMem = NULL;
    mach_msg_type_number_t data_read = 0;
    kern_return_t err = mach_vm_read(task,address,size,&readMem,&data_read);
    if (err != KERN_SUCCESS) {
        throw std::runtime_error("Could not read memory region");
    }
    
    memcpy((void*)buf, (void*)readMem, data_read);
    mach_vm_deallocate(mach_task_self(), readMem, data_read);
    return true;
}

char *ReadCString(HANDLE task, mach_vm_address_t pointer)
{
    if (pointer == 0) throw std::runtime_error("Pointer is NULL");
    
    char buf[kRemoteStringBufferSize] = {0}; // too long symbol names might not fit in
    mach_vm_size_t size = sizeof(buf);
    kern_return_t err = mach_vm_read_overwrite(task, pointer, size,
                                 (mach_vm_address_t)&buf, &size);
    if (err != KERN_SUCCESS) {
        throw std::runtime_error("Could not read memory region");
    }
    
    // add ending
    buf[kRemoteStringBufferSize-1] = '\0';
    
    char *result = strdup(buf);
    return result;
}
