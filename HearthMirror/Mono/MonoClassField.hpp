//
//  MonoClassField.hpp
//  MonoReader
//
//  Created by Istvan Fehervari on 23/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#ifndef MonoClassField_hpp
#define MonoClassField_hpp

#include <string>
#include "../memhelper.h"
#include "MonoType.hpp"

namespace hearthmirror {
    
    class MonoClass;
    class MonoObject;
    
    class MonoClassField {
    public:
        MonoClassField(HANDLE task, proc_address pField, bool is64bit);
        ~MonoClassField();
        
        std::string getName();
        int32_t getOffset();
        
        MonoType* getType();
        MonoClass* getParent();
        MonoValue getStaticValue();
        MonoValue getValue(MonoObject* o);
        
    private:
		const HANDLE _task;
        const proc_address _pField;
        const bool _is64bit;
        
        MonoValue ReadValue(MonoTypeEnum type, proc_address);
    };
    
} // end namespace


#endif /* MonoClassField_hpp */
