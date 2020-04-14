//
//  MonoClass.cpp
//  MonoReader
//
//  Created by Istvan Fehervari on 22/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#include "MonoClass.hpp"
#include "../Helpers/offsets.h"

#include <mach/mach_vm.h>
#include <mach/error.h>
#include <MacTypes.h>
#include <stdlib.h>

namespace hearthmirror {

    MonoClass::MonoClass(HANDLE task, proc_address pClass, bool is64bit) : _task(task), _pClass(pClass), _is64bit(is64bit) {}

    MonoClass::~MonoClass() {}
    
    MonoClass::MonoClass(const MonoClass* other) : _task(other->_task), _pClass(other->_pClass), _is64bit(other->_is64bit) {}
    
    std::string MonoClass::getName() {
        // printf("In MonoImage::getName - 1\n");
        proc_address addr = ReadPointer(_task, _is64bit ? _pClass + kMonoClassName64 + 8 : _pClass + kMonoClassName, _is64bit);
        // printf("In MonoImage::getName - 2\n");
        if (addr == 0) {
            std::string result("");
            return result;
        }
        // printf("In MonoImage::getName - 3\n");
        
        char* pName = ReadCString(_task, addr);

        if (pName != NULL) {
            // printf("In MonoImage::getName - 4\n");
            std::string name(pName);
            free(pName);
            return name;
        }
        
        throw std::runtime_error("Could not read MonoClass namespace");
    }

    int32_t MonoClass::getNextMonoClass() {
        

        /*
        proc_address addr = ReadPointer(_task, _is64bit ? _pClass + kMonoClassNameSpace64 : _pClass + kMonoClassNameSpace, _is64bit);
        char* pNamespace = ReadCString(_task, addr);
        */
        //kMonoClassNextClassCache64

        char buf[2048] = {0}; // too long symbol names might not fit in
        mach_vm_size_t buf_size = sizeof(buf);
    
        vm_offset_t pointer = NULL;
        vm_size_t size = 8;
        mach_msg_type_number_t data_read = 0;
        kern_return_t err1, err2, err3;
        char *result;
        uint64_t v;
        
        //printf("offset1: %lu offset2: %lu\n",kMonoClassName64,kMonoClassNameSpace64);
        
        // get pointer to namespace char*
        err1 = mach_vm_read(_task, _pClass+kMonoClassNameSpace64, size, &pointer, &data_read);
        v = 0;
        memcpy((char *)&v, (Byte*)pointer, sizeof(uint64_t));

        // copy contents from memory and hope it's a char array
        err2 = mach_vm_read_overwrite(_task, v, buf_size, (mach_vm_address_t)&buf, &buf_size);

        buf[2047] = '\0';
        result = strdup(buf);
        printf("String found: %s err1: %d err2: %d\n", result, err1, err2);
        free(result);

        // CHECK BEFORE
        // get pointer to namespace char*
        err1 = mach_vm_read(_task, _pClass+kMonoClassNameSpace64-8, size, &pointer, &data_read);
        v = 0;
        memcpy((char *)&v, (Byte*)pointer, sizeof(uint64_t));

        // copy contents from memory and hope it's a char array
        err2 = mach_vm_read_overwrite(_task, v, buf_size, (mach_vm_address_t)&buf, &buf_size);

        buf[2047] = '\0';
        result = strdup(buf);
        printf("String found -8 before: %s err1: %d err2: %d\n", result, err1, err2);
        free(result);

        // CHECK AFTER
        // get pointer to namespace char*
        err1 = mach_vm_read(_task, _pClass+kMonoClassNameSpace64+8, size, &pointer, &data_read);
        v = 0;
        memcpy((char *)&v, (Byte*)pointer, sizeof(uint64_t));

        // copy contents from memory and hope it's a char array
        err2 = mach_vm_read_overwrite(_task, v, buf_size, (mach_vm_address_t)&buf, &buf_size);

        buf[2047] = '\0';
        result = strdup(buf);
        printf("String found +8 after: %s err1: %d err2: %d\n", result, err1, err2);
        free(result);


        
        for (uint32_t i = kMonoClassNextClassCache64-12; i < kMonoClassNextClassCache64+12; i++){
            err1 = mach_vm_read(_task, _pClass+i, size, &pointer, &data_read);
            v = 0;
            memcpy((char *)&v, (Byte*)pointer, sizeof(uint64_t));

            if (err1 != KERN_SUCCESS) continue;

            err2 = mach_vm_read(_task, v+kMonoClassNameSpace64, size, &pointer, &data_read);
            v = 0;
            memcpy((char *)&v, (Byte*)pointer, sizeof(uint64_t));

            if (err2 != KERN_SUCCESS) continue;

            err3 = mach_vm_read_overwrite(_task, v, buf_size, (mach_vm_address_t)&buf, &buf_size);

            mach_vm_deallocate(mach_task_self(), pointer, size);

            if (err3 != KERN_SUCCESS) continue;

            // add ending
            buf[2047] = '\0';
            result = strdup(buf);

            printf("String found: %s old_offset: %d offset: %d err1: %d err2: %d err3: %d\n", result, kMonoClassNextClassCache64, i, err1, err2, err3);
            free(result);

        }
        


        return 0;

        /*
        if (pNamespace != NULL) {
            std::string ns(pNamespace);
            free(pNamespace);
            return ns;
        }

        throw std::runtime_error("Could not read MonoClass namespace");
        */
    }
    
    std::string MonoClass::getNameSpace() {
        proc_address addr = ReadPointer(_task, _is64bit ? _pClass + kMonoClassNameSpace64 + 8 : _pClass + kMonoClassNameSpace, _is64bit);
        if (addr == 0) return "";
        char* pNamespace = ReadCString(_task, addr);
        if (pNamespace != NULL) {
            std::string ns(pNamespace);
            free(pNamespace);
            return ns;
        }

        throw std::runtime_error("Could not read MonoClass namespace");
    }
    
    std::string MonoClass::getFullName() {
        // printf("In MonoImage::getFullName - 1\n");
        std::string name = getName();

        // printf("In MonoImage::getFullName - 2\n");
        std::string ns = getNameSpace();

        // printf("In MonoImage::getFullName - 3\n");        
        MonoClass* nestedIn = getNestedIn();
        
        while(nestedIn != NULL)
        {
            //printf("In MonoImage::getFullName - 4\n");        
            name = nestedIn->getName() + "+" + name;
            //printf("In MonoImage::getFullName - 5\n");        
            ns = nestedIn->getNameSpace();
            
            MonoClass* nestedIn_t = nestedIn->getNestedIn();
            delete nestedIn;
            nestedIn = nestedIn_t;
        }
		if (name == "") return "";
		
        return ns.size() == 0 ? name : ns + "." + name;
    }
    
    MonoClass* MonoClass::getNestedIn() {
        proc_address pNestedIn = ReadPointer(_task, _is64bit ? _pClass + kMonoClassNestedIn64 : _pClass + kMonoClassNestedIn, _is64bit);
        return pNestedIn == 0 ? NULL : new MonoClass(_task, pNestedIn, _is64bit);
    }
    
    proc_address MonoClass::getVTable() {
        proc_address runtimeInfoPtr = ReadPointer(_task, _is64bit ? _pClass + kMonoClassRuntimeInfo64 : _pClass + kMonoClassRuntimeInfo, _is64bit);
        return ReadPointer(_task, _is64bit ? runtimeInfoPtr + kMonoClassRuntimeInfoDomainVtables64 : runtimeInfoPtr + kMonoClassRuntimeInfoDomainVtables, _is64bit);
    }
    
    bool MonoClass::isValueType() {
#ifdef __APPLE__
        uint32_t monoClassSize = _is64bit ? sizeof(FakeMonoClass64) : sizeof(FakeMonoClass);
        uint8_t* buf = new uint8_t[monoClassSize];
        ReadBytes(_task, (proc_address)buf, monoClassSize, _pClass);
        bool isValueType = 0 != _is64bit ? ((FakeMonoClass64*)buf)->valuetype : ((FakeMonoClass*)buf)->valuetype;
        delete [] buf;
        return isValueType;
#else
        return 0 != (ReadByte(_task, _is64bit ? _pClass + kMonoClassBitfields64 : _pClass + kMonoClassBitfields) & 8);
#endif
    }
    
    bool MonoClass::isEnum() {
#ifdef __APPLE__
        uint32_t monoClassSize = _is64bit ? sizeof(FakeMonoClass64) : sizeof(FakeMonoClass);
        uint8_t* buf = new uint8_t[monoClassSize];
        ReadBytes(_task, (proc_address)buf, monoClassSize, _pClass);
        bool isEnum = 0 != _is64bit ? ((FakeMonoClass64*)buf)->enumtype : ((FakeMonoClass*)buf)->enumtype;
        delete [] buf;
        return isEnum;
#else
        return 0 != (ReadUInt32(_task, _pClass + kMonoClassBitfields) & 0x10);
#endif
    }
    
    int32_t MonoClass::size() {
        return ReadInt32(_task, _is64bit ? _pClass + kMonoClassSizes64 : _pClass + kMonoClassSizes);
    }
    
    MonoClass* MonoClass::getParent() const {
        proc_address pParent = ReadPointer(_task, _is64bit ? _pClass + kMonoClassParent64 : _pClass + kMonoClassParent, _is64bit);
            return pParent == 0 ? NULL : new MonoClass(_task, pParent, _is64bit);
    }
    
    MonoType* MonoClass::byValArg() {
        return new MonoType(_task, _is64bit ? _pClass + kMonoClassByvalArg64 : _pClass + kMonoClassByvalArg, _is64bit);
    }
	
	/** Number of own fields */
    uint32_t MonoClass::getNumFields() const {
        uint32_t numFields = 0;
        try {
            // think this is off by 64 bits (8 bytes)
            numFields = ReadUInt32(_task, _is64bit ? _pClass + kMonoClassFieldCount64 + 8 : _pClass + kMonoClassFieldCount);
        } catch (std::runtime_error& e) {
            numFields = 0;
        }
        return numFields;
    }

    std::vector<MonoClassField*> MonoClass::getFields() const {
        printf("MonoClass::getFields - 1\n");
		std::vector<MonoClassField*> result;
		
		// Add own fields first
        uint32_t nFields = getNumFields();
        printf("Number of fields: %d\n", nFields);
        if (nFields > 1000) {
            // this is an ugly hack to prevent leak
            return result;
        }
        printf("MonoClass::getFields - 2\n");
        // think this is off by 64 bits (8 bytes)
        proc_address pFields = ReadPointer(_task, _is64bit ? _pClass + kMonoClassFields64 + 8 : _pClass + kMonoClassFields, _is64bit);
		if (pFields != 0) {
			// add own fields first
            try {
                printf("MonoClass::getFields - 3\n");
                for (uint32_t i = 0; i < nFields; i++) {
                    // not sure if sizeof(FakeMonoClassField64) is still correct
                    MonoClassField* mcf = new MonoClassField(_task, _is64bit ? pFields + (uint32_t) i*kMonoClassFieldSizeof64 : pFields + (uint32_t) i*kMonoClassFieldSizeof, _is64bit);
                    std::string mcf_name = mcf->getName();
                    printf("mcf name: %s\n", mcf_name.c_str());

                    result.push_back(mcf);
                }
            } catch (std::runtime_error& e) {
                printf("MonoClass::getFields - 4\n");
                auto it = result.begin();
                while (it != result.end()) {
                    delete *it;
                    it = result.erase(it);
                }
            }
		}
        printf("MonoClass::getFields - 5\n");
		
		// add parent fields (if available)
        MonoClass* parent = getParent();
        if (parent) {
			std::vector<MonoClassField*> parent_fields = parent->getFields();
			result.insert(result.end(), parent_fields.begin(), parent_fields.end());
			delete parent;
        }
        return result;
    }
    
    MonoValue MonoClass::operator[](const std::string& key) {
        
        std::vector<MonoClassField*> fields = getFields();
        MonoValue ret(0);
        
        for (MonoClassField* mcf : fields) {
            if (mcf->getName() == key) {
                try {
                    ret = mcf->getStaticValue();
                } catch (std::exception ex) {
                    // could not read
                    //printf("failed to read");
                }
            }
            delete mcf;
        }
        return ret;
    }
} // namespace hearthmirror
