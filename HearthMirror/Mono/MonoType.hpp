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
#include "../Helpers/fakemonostructs.h"
#include <locale>

namespace hearthmirror {
    
    class MonoObject;
    class MonoStruct;
    
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
			type = MONO_TYPE_END;
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
