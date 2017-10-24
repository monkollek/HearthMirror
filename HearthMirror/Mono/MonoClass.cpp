//
//  MonoClass.cpp
//  MonoReader
//
//  Created by Istvan Fehervari on 22/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#include "MonoClass.hpp"
#include "../Helpers/offsets.h"

namespace hearthmirror {

    MonoClass::MonoClass(HANDLE task, proc_address pClass, bool is64bit) : _task(task), _pClass(pClass), _is64bit(is64bit) {}

    MonoClass::~MonoClass() {}
    
    MonoClass::MonoClass(const MonoClass* other) : _task(other->_task), _pClass(other->_pClass), _is64bit(other->_is64bit) {}
    
    std::string MonoClass::getName() {
        proc_address addr = ReadPointer(_task, _is64bit ? _pClass + kMonoClassName64 : _pClass + kMonoClassName, _is64bit);
        if (addr == 0) {
            std::string result("");
            return result;
        }
        char* pName = ReadCString(_task, addr);
        if (pName != NULL) {
            std::string name(pName);
            free(pName);
            return name;
        }
        
        throw std::runtime_error("Could not read MonoClass namespace");
    }
    
    std::string MonoClass::getNameSpace() {
        proc_address addr = ReadPointer(_task, _is64bit ? _pClass + kMonoClassNameSpace64 : _pClass + kMonoClassNameSpace, _is64bit);
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
        std::string name = getName();
        std::string ns = getNameSpace();
        
        MonoClass* nestedIn = getNestedIn();
        
        while(nestedIn != NULL)
        {
            name = nestedIn->getName() + "+" + name;
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
        uint32_t monoClassSize = _is64bit ? sizeof(FakeMonoClass64) : sizeof(FakeMonoClass);
        uint8_t* buf = new uint8_t[monoClassSize];
        ReadBytes(_task, (proc_address)buf, monoClassSize, _pClass);
        bool isValueType = 0 != _is64bit ? ((FakeMonoClass64*)buf)->valuetype : ((FakeMonoClass*)buf)->valuetype;
        delete [] buf;
        return isValueType;
        //0 != (ReadByte(_task, _is64bit ? _pClass + kMonoClassBitfields64 : _pClass + kMonoClassBitfields) & 8);
    }
    
    bool MonoClass::isEnum() {
        uint32_t monoClassSize = _is64bit ? sizeof(FakeMonoClass64) : sizeof(FakeMonoClass);
        uint8_t* buf = new uint8_t[monoClassSize];
        ReadBytes(_task, (proc_address)buf, monoClassSize, _pClass);
        bool isEnum = 0 != _is64bit ? ((FakeMonoClass64*)buf)->enumtype : ((FakeMonoClass*)buf)->enumtype;
        delete [] buf;
        return isEnum;
        //0 != (ReadUInt32(_task, _pClass + kMonoClassBitfields) & 0x10);
    }
    
    int32_t MonoClass::size() {
        return ReadInt32(_task, _is64bit ? _pClass + kMonoClassSizes64 : _pClass + kMonoClassSizes);
    }
    
    MonoClass* MonoClass::getParent() {
        proc_address pParent = ReadPointer(_task, _is64bit ? _pClass + kMonoClassParent64 : _pClass + kMonoClassParent, _is64bit);
            return pParent == 0 ? NULL : new MonoClass(_task, pParent, _is64bit);
    }
    
    MonoType* MonoClass::byValArg() {
        return new MonoType(_task, _is64bit ? _pClass + kMonoClassByvalArg64 : _pClass + kMonoClassByvalArg, _is64bit);
    }
	
	/** Number of own fields */
    uint32_t MonoClass::getNumFields() {
        uint32_t numFields = 0;
        try {
            numFields = ReadUInt32(_task, _is64bit ? _pClass + kMonoClassFieldCount64 : _pClass + kMonoClassFieldCount);
        } catch (std::runtime_error& e) {
            numFields = 0;
        }
        return numFields;
    }

    std::vector<MonoClassField*> MonoClass::getFields() {
		std::vector<MonoClassField*> result;
		
		// Add own fields first
        uint32_t nFields = getNumFields();
        if (nFields > 1000) {
            // this is an ugly hack to prevent leak
            return result;
        }
        proc_address pFields = ReadPointer(_task, _is64bit ? _pClass + kMonoClassFields64 : _pClass + kMonoClassFields, _is64bit);
		if (pFields != 0) {
			// add own fields first
            try {
                for (uint32_t i = 0; i < nFields; i++) {
                    MonoClassField* mcf = new MonoClassField(_task, _is64bit ? pFields + (uint32_t) i*kMonoClassFieldSizeof64 : pFields + (uint32_t) i*kMonoClassFieldSizeof, _is64bit);
                    result.push_back(mcf);
                }
            } catch (std::runtime_error& e) {
                auto it = result.begin();
                while (it != result.end()) {
                    delete *it;
                    it = result.erase(it);
                }
            }
		}
		
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
