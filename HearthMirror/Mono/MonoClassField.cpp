//
//  MonoClassField.cpp
//  MonoReader
//
//  Created by Istvan Fehervari on 23/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#include "MonoClassField.hpp"
#include "../Helpers/offsets.h"
#include "MonoClass.hpp"
#include "MonoObject.hpp"
#include "MonoStruct.hpp"

namespace hearthmirror {
    
    MonoClassField::MonoClassField(HANDLE task, proc_address pField, bool is64bit) : _task(task), _pField(pField), _is64bit(is64bit) {}
    
    MonoClassField::~MonoClassField() {}

    std::string MonoClassField::getName() {
        try {
            char* pName = ReadCString(_task, ReadPointer(_task, _is64bit ? _pField + kMonoClassFieldName64 : _pField + kMonoClassFieldName, _is64bit));
            if (pName == NULL) return std::string("");
            std::string name(pName);
            free(pName);
            return name;
        } catch (std::exception& exp) {
            return std::string("");
        }
    }
    
    int32_t MonoClassField::getOffset() {
        return ReadInt32(_task, _is64bit ? _pField + kMonoClassFieldOffset64 : _pField + kMonoClassFieldOffset);
    }
    
    MonoType* MonoClassField::getType() {
        try {
            return new MonoType(_task, ReadPointer(_task, _is64bit ? _pField + kMonoClassFieldType64 : _pField + kMonoClassFieldType, _is64bit), _is64bit);
        } catch (std::exception& ex) {
            return NULL;
        }
    }
    
    MonoClass* MonoClassField::getParent() {
        try {
            auto parent = new MonoClass(_task, ReadPointer(_task, _is64bit ? _pField + kMonoClassFieldParent64 : _pField + kMonoClassFieldParent, _is64bit), _is64bit);
            if (parent->getName().empty()) {
                delete parent;
                return NULL;
            }
            return parent;
        } catch (std::exception& ex) {
            return NULL;
        }
    }
    
    MonoValue MonoClassField::getStaticValue() {
        printf("MonoClassField::getStaticValue - 1\n");
        MonoValue ret(0);
        MonoType* type = getType();
        if (type == NULL) {
            printf("MonoClassField::getStaticValue - 5\n");
            return ret;
        }
        
        if (type->isStatic()) {
            try {
                printf("MonoClassField::getStaticValue - 2\n");
                ret = getValue(NULL);
            } catch (std::runtime_error& ex) {
                printf("MonoClassField::getStaticValue - 3\n");
                delete type;
                return MonoValue(0);
            }
        }
		
        printf("MonoClassField::getStaticValue - 4\n");
        printf("Monotype: %d\n",type->getType());

        if(type->getType() == MonoTypeEnum::MONO_TYPE_GENERICINST){
            printf("MonoClassField::getStaticValue - 5\n");
            proc_address genericClass = type->getData();
            MonoClass* container = new MonoClass(_task, ReadPointer(_task, genericClass, _is64bit), _is64bit);

            printf("GENERICINST has %d fields\n", container->getNumFields());
            std::vector<MonoClassField*> tmp = container->getNumFields();

        }

        delete type;
        return ret;
    }
    
    MonoValue MonoClassField::getValue(MonoObject* o) {
        printf("MonoClassField::getValue - 1\n");
        int32_t offset = getOffset();
        MonoType* type = getType();
        if (type == NULL) {
            return MonoValue(0);
        }
        MonoTypeEnum typeType = type->getType();

        bool isRef;
        // get data type
        switch(typeType)
        {
            case MonoTypeEnum::MONO_TYPE_STRING:
            case MonoTypeEnum::MONO_TYPE_SZARRAY:
                isRef = false;
                break;
            case MonoTypeEnum::MONO_TYPE_OBJECT:
            case MonoTypeEnum::MONO_TYPE_CLASS:
            case MonoTypeEnum::MONO_TYPE_ARRAY:
                isRef = true;
                break;
            case MonoTypeEnum::MONO_TYPE_GENERICINST: {
                proc_address genericClass = type->getData();
                MonoClass* container = new MonoClass(_task, ReadPointer(_task, genericClass, _is64bit), _is64bit);
                isRef = !container->isValueType();
                delete container;
                break;
            }
            default:
                isRef = type->byRef();
                break;
        }
        
        if(type->isStatic()) {
            printf("MonoClassField::getValue - 2\n");
            MonoClass* parent = getParent();
            if (parent == NULL) {
                printf("MonoClassField::getValue - 3\n");
                delete type;
                return MonoValue(0);
            }
            proc_address vtable = 0;
            try {
                printf("MonoClassField::getValue - 4\n");
                vtable = parent->getVTable();
            } catch (std::exception& ex) {
                printf("MonoClassField::getValue - 5\n");
                delete parent;
                delete type;
                return MonoValue(0);
            }
            
            delete parent;
            
            if (vtable == 0) {
                delete type;
                return MonoValue(0);
            }
            printf("MonoClassField::getValue - 6\n");
            proc_address data = ReadPointer(_task, _is64bit ? vtable + kMonoVTableData64 : vtable + kMonoVTableData, _is64bit);
            
            if (isRef) {
                proc_address po = ReadPointer(_task, data + offset, _is64bit);
                
                delete type;
                if (po == 0) {
                    return MonoValue(0);
                } else {
                    MonoValue mv;
                    mv.type = typeType;
                    mv.value.obj.o = new MonoObject(_task, po, _is64bit);
                    
                    auto mclass = mv.value.obj.o->getClass();
                    if (mclass->getName().empty()) {
                        delete mclass;
                        delete mv.value.obj.o;
                        return MonoValue(0);
                    }
                    delete mclass;
                    
                    return mv;
                }
            }
            
            if(typeType == MonoTypeEnum::MONO_TYPE_VALUETYPE) {
                
                MonoClass* sClass = new MonoClass(_task, type->getData(), _is64bit);
                if (sClass->isEnum()) {
                    MonoClass* c = new MonoClass(_task, ReadPointer(_task, type->getData(), _is64bit), _is64bit);
                    MonoType* bva = c->byValArg();
                    delete c;
                    
                    MonoValue mv = ReadValue(bva->getType(), data + offset);
                    delete bva;
                    
                    delete type;
                    delete sClass;
                    return mv;
                }
                delete type;
                MonoValue mv;
                mv.type = MonoTypeEnum::MONO_TYPE_VALUETYPE;
                MonoClass* c2 = new MonoClass(sClass);
                mv.value.obj.s = new MonoStruct(_task, c2, data + offset, _is64bit);
                delete sClass;
                return mv;
            }
            delete type;
            if (typeType == MonoTypeEnum::MONO_TYPE_GENERICINST) {
                return MonoValue(0);
            } else {
                return ReadValue(typeType, data + offset);
            }
        }
        
        if (o == NULL) {
            //throw std::runtime_error("passed object is not value type and is NULL");
            return MonoValue(0);
        }
        
        if(isRef) {
            
            proc_address po = ReadPointer(_task, o->_pObject + offset, _is64bit);
            delete type;
            if (po == 0) {
                return MonoValue(0);
            } else {
                MonoValue mv;
                mv.type = MONO_TYPE_OBJECT;
                mv.value.obj.o = new MonoObject(_task, po, _is64bit);
                return mv;
            }
        }
        
        if(typeType == MonoTypeEnum::MONO_TYPE_VALUETYPE) {
            
            MonoClass* sClass = new MonoClass(_task, type->getData(), _is64bit);
            if(sClass->isEnum()) {
                MonoClass* c = new MonoClass(_task, ReadPointer(_task, type->getData(), _is64bit), _is64bit);
                MonoType* bva = c->byValArg();
                delete c;
                
                MonoValue res = ReadValue(bva->getType(), o->_pObject + offset);
                delete bva;
                
                delete type;
                delete sClass;
                
                return res;
            }
            delete type;
            MonoValue mv;
            mv.type = MonoTypeEnum::MONO_TYPE_VALUETYPE;
            
            MonoClass* c2 = new MonoClass(sClass);
            mv.value.obj.s = new MonoStruct(_task, c2, o->_pObject + offset, _is64bit);
            delete sClass;
            return mv;
        }
        
        delete type;
        if (typeType == MonoTypeEnum::MONO_TYPE_GENERICINST) {
            MonoValue mv(0);
            return mv;
        } else {
            return ReadValue(typeType, o->_pObject + offset);
        }
    }
    
    MonoValue MonoClassField::ReadValue(MonoTypeEnum type, proc_address addr) {
        MonoValue result;
        result.type = type;
        switch(type)
        {
            case MonoTypeEnum::MONO_TYPE_BOOLEAN: {
                result.value.b = ReadBool(_task, addr);
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_U1: {
                result.value.u8 = ReadByte(_task, addr);
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_I1: {
                result.value.i8 = ReadSByte(_task, addr);
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_I2: {
                result.value.i16 = ReadShort(_task, addr);
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_U2: {
                result.value.u16 = ReadUShort(_task, addr);
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_CHAR: {
                result.value.c = (char)ReadUShort(_task, addr);
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_I:
            case MonoTypeEnum::MONO_TYPE_I4: {
                result.value.i32 = ReadInt32(_task, addr);
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_U:
            case MonoTypeEnum::MONO_TYPE_U4: {
                result.value.u32 = ReadUInt32(_task, addr);
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_I8: {
                result.value.i64 = ReadInt64(_task, addr);
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_U8: {
                result.value.u64 = ReadUInt64(_task, addr);
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_R4: {
                result.value.f = ReadFloat(_task, addr);
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_R8:{
                result.value.d = ReadDouble(_task, addr);
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_SZARRAY: {
                addr = ReadPointer(_task, addr, _is64bit); // deref object
                if (addr == 0) {
                    return MonoValue(0);
                }
                proc_address vt = ReadPointer(_task, addr, _is64bit);
                proc_address pArrClass = ReadPointer(_task, vt, _is64bit);
                MonoClass* arrClass = new MonoClass(_task, pArrClass, _is64bit);
                MonoClass* elClass = new MonoClass(_task, ReadPointer(_task, pArrClass, _is64bit), _is64bit);
       
                uint32_t count = ReadInt32(_task, addr + (_is64bit ? kMonoArrayMaxLength64 : kMonoArrayMaxLength));
                proc_address start = addr + (_is64bit ? kMonoArrayVector64 : kMonoArrayVector);
                result.arrsize = count;
                
                if (count > 0) {
                    result.value.arr = new MonoValue[count];
                    for (uint32_t i = 0; i < count; i++) {
                        proc_address ea = start + (i * arrClass->size());
                        if(elClass->isValueType()) {
                            MonoType* mt = elClass->byValArg();
                            auto itype = mt->getType();
                            if (itype == MonoTypeEnum::MONO_TYPE_VALUETYPE || itype == MONO_TYPE_GENERICINST) {
                                MonoClass* c2 = new MonoClass(elClass);
                                MonoStruct* stc = new MonoStruct(_task, c2, ea, _is64bit);
                                MonoValue mv;
                                mv.type = MonoTypeEnum::MONO_TYPE_VALUETYPE;
                                mv.value.obj.s = stc;
                                result[i] = mv;
                            } else {
                                result[i] = ReadValue(mt->getType(), ea);
                            }
                            delete mt;
                        } else {
                            MonoType* bvmt = elClass->byValArg();
                            auto bvmtType = bvmt->getType();
                            delete bvmt;
                            if (bvmtType == MONO_TYPE_SZARRAY) {
                                result[i] = ReadValue(MONO_TYPE_SZARRAY, ea);
                            } else {
                                proc_address po = ReadPointer(_task, ea, _is64bit);
                                MonoValue mv;
                                if (po == 0) {
                                    result[i] = MonoValue(0);
                                } else {
                                    mv.type = MonoTypeEnum::MONO_TYPE_GENERICINST;
                                    mv.value.obj.o = new MonoObject(_task, po, _is64bit);
                                    result[i] = mv;
                                }
                            }
                        }
                    }
                }
                
                delete arrClass;
                delete elClass;
                return result;
            }
            case MonoTypeEnum::MONO_TYPE_STRING: {
                proc_address pArr = ReadPointer(_task, addr, _is64bit);
                result.str =  std::u16string();
                if(pArr == 0) {
                    return result;
                }
   
                int32_t strlen = ReadInt32(_task, pArr + (_is64bit ? kMonoStringLength64 : kMonoStringLength));
                if(strlen == 0) {
                    return result;
                }
                
                uint32_t size = strlen*2;
                uint8_t* buf = new uint8_t[size];
                
                ReadBytes(_task, (proc_address)buf, size, pArr + (_is64bit ? kMonoStringChars64 : kMonoStringChars));
                result.str = std::u16string((char16_t*)buf, strlen);
				delete[]  buf;
                return result;
            }
            default:
                // This will trigger for System.Nullable too often
                // printf("Error: %d not implemented\n",type);
                break;
        }
        result.arrsize = 0;
        return result;
    }
}
