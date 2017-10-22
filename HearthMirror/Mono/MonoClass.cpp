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
    
    MonoClass::MonoClass(bool is64bit) : _is64bit(is64bit) {}

    MonoClass::MonoClass(HANDLE task, proc_address pClass, bool is64bit) : _task(task), _pClass(pClass), _is64bit(is64bit) {}

    MonoClass::~MonoClass() {}
    
    MonoClass::MonoClass(const MonoClass* other) : _is64bit(other->_is64bit) {
        _task = other->_task;
        _pClass = other->_pClass;
    }
    
    std::string MonoClass::getName() {
        uint32_t addr = ReadUInt32(_task, _pClass + kMonoClassName);
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
        uint32_t addr = ReadUInt32(_task, _pClass + kMonoClassNameSpace);
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
        uint32_t pNestedIn = ReadUInt32(_task, _pClass + kMonoClassNestedIn);
        return pNestedIn == 0 ? NULL : new MonoClass(_task, pNestedIn);
    }
    
    uint32_t MonoClass::getVTable() {
        uint32_t rti = ReadUInt32(_task, _pClass + kMonoClassRuntimeInfo);
        return ReadUInt32(_task, rti + kMonoClassRuntimeInfoDomainVtables);
    }
    
    bool MonoClass::isValueType() {
        return 0 != (ReadByte(_task, _pClass + kMonoClassBitfields) & 8);
    }
    
    bool MonoClass::isEnum() {
        return 0 != (ReadUInt32(_task, _pClass + kMonoClassBitfields) & 0x10);
    }
    
    uint32_t MonoClass::size() {
        return ReadUInt32(_task, _pClass + kMonoClassSizes);
    }
    
    MonoClass* MonoClass::getParent() {
            uint32_t pParent = ReadUInt32(_task, _pClass + kMonoClassParent);
            return pParent == 0 ? NULL : new MonoClass(_task, pParent);
    }
    
    MonoType* MonoClass::byValArg() {
        return new MonoType(_task, _pClass + kMonoClassByvalArg);
    }
	
	/** Number of own fields */
    uint32_t MonoClass::getNumFields() {
        uint32_t numFields = 0;
        try {
            numFields = ReadUInt32(_task, _pClass + kMonoClassFieldCount);
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
		uint32_t pFields = ReadUInt32(_task, _pClass + kMonoClassFields);
		if (pFields != 0) {
			
			// add own fields first
            try {
                for(uint32_t i = 0; i < nFields; i++) {
                    MonoClassField* mcf = new MonoClassField(_task, pFields + (uint32_t) i*kMonoClassFieldSizeof);
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
