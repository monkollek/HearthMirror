//
//  MonoObject.hpp
//  MonoReader
//
//  Created by Istvan Fehervari on 23/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#ifndef MonoObject_hpp
#define MonoObject_hpp

#include <map>
#include <string>

#include "../memhelper.h"
#include "MonoType.hpp"

namespace hearthmirror {
    
    class MonoClass;
    
    class MonoObject {
    public:
        MonoObject(HANDLE task, proc_address pObject, bool is64bit);
        ~MonoObject();
        
        const proc_address _pObject;
        
        MonoClass* getClass();
        std::map<std::string, MonoValue> getFields();
        
        MonoValue operator[](const std::string& key);
        
    private:
		const HANDLE _task;
        const proc_address _vtable;
        const bool _is64bit;
    };
    
} // end namespace


#endif /* MonoObject_hpp */
