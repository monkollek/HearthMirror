//
//  MonoClass.hpp
//  MonoReader
//
//  Created by Istvan Fehervari on 22/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#ifndef MonoClass_hpp
#define MonoClass_hpp

#include <string>
#include <vector>

#include "../memhelper.h"
#include "MonoType.hpp"
#include "MonoClassField.hpp"

namespace hearthmirror {

    class MonoClass {
    public:
        MonoClass(HANDLE task, proc_address pClass, bool is64bit);
        MonoClass(const MonoClass* other);
        ~MonoClass();
        
        std::string getName();
        std::string getNameSpace();
        std::string getFullName();
        
        MonoClass* getNestedIn();
        proc_address getVTable();
        
        bool isValueType();
        bool isEnum();
        int32_t size();
        
        MonoClass* getParent() const;
        MonoType* byValArg();
        
        uint32_t getNumFields() const;
        std::vector<MonoClassField*> getFields() const;
        
        MonoValue operator[](const std::string& key);
        
    private:
		const HANDLE _task;
        const proc_address _pClass;
        const bool _is64bit;
    };

} // HSReader namespace

#endif /* MonoClass_hpp */
