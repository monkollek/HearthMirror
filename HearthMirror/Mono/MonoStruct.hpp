//
//  MonoStruct.hpp
//  MonoReader
//
//  Created by Istvan Fehervari on 23/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#ifndef MonoStruct_hpp
#define MonoStruct_hpp

#include "../memhelper.h"
#include <map>
#include <string>
#include "MonoType.hpp"

namespace hearthmirror {
    
    class MonoClass;
    
    class MonoStruct {
    public:
        /** Class takes ownership of the passed MonoClass instance */
        MonoStruct(HANDLE task, MonoClass* mClass, proc_address pStruct, bool is64bit);
        ~MonoStruct();
        
        std::map<std::string, MonoValue> getFields();
        
        MonoValue operator[](const std::string& key);
        
    private:
		const HANDLE _task;
        const MonoClass* _monoClass;
        const proc_address _pStruct;
        const bool _is64bit;
    };
    
} // end namespace

#endif /* MonoStruct_hpp */
