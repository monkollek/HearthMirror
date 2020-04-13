//
//  MonoImage.cpp
//  MonoReader
//
//  Created by Istvan Fehervari on 22/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#include "MonoImage.hpp"
#include "../Helpers/offsets.h"

namespace hearthmirror {
    
    MonoImage::MonoImage(HANDLE task, proc_address pImage, bool is64bit) : _task(task), _pImage(pImage), _is64bit(is64bit) {
        this->loadClasses();
    }

    MonoImage::~MonoImage() {
        for (auto it = _classes.begin(); it != _classes.end(); it++) {
            delete it->second;
        }
        _classes.clear();
    }
    
    bool MonoImage::hasClasses() {
        size_t size = this->_classes.size();
        return size > 0;
    }

    MonoClass* MonoImage::get(const std::string& key) {
        if (this->_classes.count(key)) {
            return this->_classes.at(key);
        }
        return NULL;
    }

    void MonoImage::loadClasses() {
        for (auto it = _classes.begin(); it != _classes.end(); it++) {
            delete it->second;
        }
        _classes.clear();
        
        proc_address classCache = _is64bit ? _pImage + kMonoImageClassCache64 : _pImage + kMonoImageClassCache;
        int32_t size = ReadInt32(_task, _is64bit ? classCache + kMonoInternalHashTableSize64 : classCache + kMonoInternalHashTableSize);
        proc_address table = ReadPointer(_task, _is64bit ? classCache + kMonoInternalHashTableTable64 : classCache + kMonoInternalHashTableTable, _is64bit);

        for (uint32_t i = 0; i < size; i++) {
            proc_address pClass = ReadPointer(_task, _is64bit ? table + 8*i : table + 4*i, _is64bit);
            while (pClass != 0) {
                MonoClass* klass = new MonoClass(_task, pClass, _is64bit);
                std::string cfname = klass->getFullName();
				if (cfname != "") {
					_classes[cfname] = klass;
				}
                pClass = ReadPointer(_task, _is64bit ? pClass + kMonoClassNextClassCache64 : pClass + kMonoClassNextClassCache, _is64bit);
            }
        }
    }
    
    int MonoImage::getMonoImage(int pid, bool isBlocking, HANDLE* handle, MonoImage** monoimage) {
        printf("In MonoImage::getMonoImage - 1\n");
#ifdef __APPLE__
        kern_return_t kret = task_for_pid(mach_task_self(), pid, handle);
        if (kret!=KERN_SUCCESS) {
            printf("task_for_pid() failed with message %s!\n",mach_error_string(kret));
            return 3;
        }
#else
        *handle = OpenProcess(PROCESS_QUERY_INFORMATION |
                            PROCESS_VM_READ,
                            FALSE, pid);
		if (*handle == NULL) {
			return 3;
		}
#endif
        
        do {
            bool is64bit = false;
            //printf("In MonoImage::getMonoImage - calling getMonoLoadAddress\n");
            proc_address baseaddress = getMonoLoadAddress(*handle, &is64bit);
            if (baseaddress == 0) return 4;
            
            // we need to find the address of "mono_root_domain"
            //printf("In MonoImage::getMonoImage - calling getMonoRootDomainAddr\n");
            proc_address mono_grd_addr = getMonoRootDomainAddr(*handle, baseaddress, is64bit);
            if (mono_grd_addr == 0) return 5;
            
            proc_address rootDomain;
            
            try {
#ifdef __APPLE__
                if (is64bit) {
                    rootDomain = ReadUInt64(*handle, baseaddress+mono_grd_addr);
                } else {
                    rootDomain = ReadUInt32(*handle, baseaddress+mono_grd_addr);
                }
#else
                rootDomain = ReadUInt32(*handle, mono_grd_addr);
#endif
            } catch (std::runtime_error& err) {
                return 6;
            }
            if (rootDomain == 0) return 7;
            
            proc_address pImage = 0;
            try {
                // iterate domain_assemblies;
                proc_address next = ReadPointer(*handle, is64bit ? rootDomain+kMonoDomainDomainAssemblies64 : rootDomain+kMonoDomainDomainAssemblies, is64bit);
                
                while (next != 0) {
                    proc_address assemblyPtr = ReadPointer(*handle, next, is64bit);
                    if (is64bit) {
                        next = ReadPointer(*handle, next + 8, is64bit);
                    } else {
                        next = ReadPointer(*handle, next + 4, is64bit);
                    }
                    char* name = is64bit ? ReadCString(*handle, ReadPointer(*handle, (proc_address)assemblyPtr + kMonoAssemblyName64, true)) : ReadCString(*handle, ReadPointer(*handle, (proc_address)assemblyPtr + kMonoAssemblyName, false));
                    printf("String search: %s\n", name);
                    if(strcmp(name, "Assembly-CSharp") == 0) {
                        free(name);
                        pImage = ReadPointer(*handle, is64bit ? assemblyPtr + kMonoAssemblyImage64 : assemblyPtr + kMonoAssemblyImage, is64bit);
                        break;
                    }
                    free(name);
                }
            } catch (std::runtime_error& err) {
                return 8;
            }
            
            // we have a pointer now to the right assembly image
            try {
                *monoimage = new MonoImage(*handle, pImage, is64bit);
                if ((*monoimage)->hasClasses()) break;
                
                delete *monoimage;
                *monoimage = NULL;
            } catch (std::runtime_error& err) {
                delete *monoimage;
                *monoimage = NULL;
            }
        } while (isBlocking);
        
        return *monoimage == NULL ? 10 : 0;
    }

} // namespace HSReader
