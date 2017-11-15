//
//  MonoType.cpp
//  MonoReader
//
//  Created by Istvan Fehervari on 23/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#include "MonoType.hpp"
#include "MonoObject.hpp"
#include "MonoStruct.hpp"
#include "../Helpers/offsets.h"

namespace hearthmirror {
    
    MonoType::MonoType(HANDLE task, proc_address pType, bool is64bit) : _task(task), _pType(pType), _is64bit(is64bit) {}
    
    MonoType::~MonoType() {}
    
    uint32_t MonoType::getAttrs() {
#ifdef __APPLE__
        uint32_t monoTypeSize = _is64bit ? sizeof(FakeMonoType64) : sizeof(FakeMonoType);
        uint8_t* buf = new uint8_t[monoTypeSize];
        ReadBytes(_task, (proc_address)buf, monoTypeSize, _pType);
        uint32_t attr = _is64bit ? ((FakeMonoType64*)buf)->attrs : ((FakeMonoType*)buf)->attrs;
        delete [] buf;
        return attr;
#else
		return ReadUInt32(_task, _pType + kMonoTypeAttrs);
#endif
    }
    
    proc_address MonoType::getData() {
#ifdef __APPLE__
        return ReadPointer(_task, _is64bit ? _pType + kMonoTypeData64 : _pType + kMonoTypeData, _is64bit);
#else
		return ReadPointer(_task, _pType, false);
#endif // __APPLE__ 
    }
    
    bool MonoType::isStatic() {
        return 0 != (getAttrs() & 0x10);
    }
    
    bool MonoType::isPublic() {
        return 6 == (getAttrs() & 0x7);
    }
    
    bool MonoType::isLiteral() {
        return 0 != (getAttrs() & 0x40);
    }
    
    bool MonoType::hasDefault() {
        return 0 != (getAttrs() & 0x8000);
    }
    
    bool MonoType::hasFieldRva() {
        return 0 != (getAttrs() & 0x100);
    }
    
    bool MonoType::byRef() {
        return 0 != (getAttrs() & 0x40000000);
    }
    
    MonoTypeEnum MonoType::getType() {
#ifdef __APPLE__
        uint32_t monoTypeSize = _is64bit ? sizeof(FakeMonoType64) : sizeof(FakeMonoType);
        uint8_t* buf = new uint8_t[monoTypeSize];
        ReadBytes(_task, (proc_address)buf, monoTypeSize, _pType);
        MonoTypeEnum typeEnum = _is64bit ? ((FakeMonoType64*)buf)->type : ((FakeMonoType*)buf)->type;
        delete [] buf;
        return typeEnum;
#else
		return (MonoTypeEnum)(0xff & (getAttrs() >> 16));
#endif // __APPLE__ 
    }

    void DeleteMonoValue(MonoValue& mv) {
        if (mv.arrsize == 0) return; // value is null
        switch (mv.type) {
            case MonoTypeEnum::MONO_TYPE_OBJECT:
            case MonoTypeEnum::MONO_TYPE_VAR:
            case MonoTypeEnum::MONO_TYPE_GENERICINST:
            case MonoTypeEnum::MONO_TYPE_CLASS:
                delete mv.value.obj.o;
                break;
            case MonoTypeEnum::MONO_TYPE_VALUETYPE:
                delete mv.value.obj.s;
                break;
            case MonoTypeEnum::MONO_TYPE_SZARRAY:
                for (unsigned int i =0; i< mv.arrsize; i++) {
                    DeleteMonoValue(mv[i]);
                }
                delete[] mv.value.arr;
                break;
            default: {
            }
                break;
        }
    }
    
    bool IsMonoValueEmpty(const MonoValue& mv) {
        return (mv.arrsize == 0) || ((mv.type == MonoTypeEnum::MONO_TYPE_CLASS || mv.type == MonoTypeEnum::MONO_TYPE_GENERICINST || mv.type == MonoTypeEnum::MONO_TYPE_OBJECT || mv.type == MonoTypeEnum::MONO_TYPE_VAR) && mv.value.obj.o == NULL);
    }
    
    
    bool IsMonoValueArray(const MonoValue& mv) {
        return mv.type == MonoTypeEnum::MONO_TYPE_ARRAY || mv.type == MonoTypeEnum::MONO_TYPE_SZARRAY;
    }
}
