//
//  MonoType.hpp
//  MonoReader
//
//  Created by Istvan Fehervari on 23/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#ifndef MonoType_hpp
#define MonoType_hpp

#include "../memhelper.h"
#include <locale>

namespace hearthmirror {
    
    class MonoObject;
    class MonoStruct;
    
    enum MonoTypeEnum
    {
        End = 0x00,
        Void = 0x01,
        Boolean = 0x02,
        Char = 0x03,
        I1 = 0x04,
        U1 = 0x05,
        I2 = 0x06,
        U2 = 0x07,
        I4 = 0x08,
        U4 = 0x09,
        I8 = 0x0a,
        U8 = 0x0b,
        R4 = 0x0c,
        R8 = 0x0d,
        String = 0x0e,
        Ptr = 0x0f,
        ByRef = 0x10,
        ValueType = 0x11,
        Class = 0x12,
        Var = 0x13,
        Array = 0x14,
        GenericInst = 0x15,
        TypedByRef = 0x16,
        I = 0x18,
        U = 0x19,
        FnPtr = 0x1b,
        Object = 0x1c,
        Szarray = 0x1d,
        Mvar = 0x1e,
        Cmod_reqd = 0x1f,
        Cmod_opt = 0x20,
        Internal = 0x21,
        Modifier = 0x40,
        Sentinel = 0x41,
        Pinned = 0x45,
        Enum = 0x55
    };
    
    typedef struct _MonoValue {
        MonoTypeEnum type;
        union _value {
            bool b;
            uint8_t u8;
            int8_t i8;
            uint16_t u16;
            int16_t i16;
            uint32_t u32;
            int32_t i32;
            uint64_t u64;
            int64_t i64;
            char c;
            float f;
            double d;
            union _obj {
                MonoObject* o;
                MonoStruct* s; // type = ValueType
            } obj;
            
            _MonoValue* arr;
        } value;
        std::u16string str;
        uint32_t arrsize = 1;
        
        _MonoValue& operator[](unsigned int idx) {return value.arr[idx];}
        _MonoValue(int asize = 1) : arrsize(asize) {
            value.obj.o = NULL;
			type = End;
        }
    } MonoValue;
    
    /** Frees the underlying data of the monovalue. */
    void DeleteMonoValue(MonoValue& mv);
    
    /** Returns true is value is empty i.e. no data is contained. */
    bool IsMonoValueEmpty(const MonoValue& mv);
    
    /** Returns true if the value represents an array of data. False otherwise. */
    bool IsMonoValueArray(const MonoValue& mv);
    
    class MonoType {
    public:
        MonoType(HANDLE task, proc_address pType, bool is64bit);
        ~MonoType();
        
        uint32_t getAttrs();
        
        proc_address getData();
        
        bool isStatic();        
        bool isPublic();
        bool isLiteral();
        bool hasDefault();
        bool hasFieldRva();
        bool byRef();
        MonoTypeEnum getType();
        
    private:
		const HANDLE _task;
        const proc_address _pType;
        const bool _is64bit;
    };
    
} // end namespace

#endif /* MonoType_hpp */
