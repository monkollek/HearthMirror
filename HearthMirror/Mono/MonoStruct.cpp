//
//  MonoStruct.cpp
//  MonoReader
//
//  Created by Istvan Fehervari on 23/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#include "MonoStruct.hpp"
#include "MonoClass.hpp"
#include "MonoObject.hpp"

namespace hearthmirror {
    
    MonoStruct::MonoStruct(HANDLE task, MonoClass* mClass, proc_address pStruct, bool is64bit) : _task(task), _monoClass(mClass), _pStruct(pStruct), _is64bit(is64bit) {}
    
    MonoStruct::~MonoStruct() {
        delete _monoClass;
    }
    
    std::map<std::string, MonoValue> MonoStruct::getFields() {
        
        std::vector<MonoClassField*> fields = _monoClass->getFields();
        
        std::map<std::string, MonoValue> res;
        
        for (MonoClassField* f : fields) {
            MonoType* type = f->getType();
            if (type) {
                std::string fname = f->getName();
                if (!type->isStatic() && (!fname.empty()) ) {
                    MonoObject* o = new MonoObject(_task, _pStruct - 8, _is64bit);
                    res[fname] = f->getValue(o);
                    delete o;
                }
                delete type;
            }
            delete f;
        }
        
        return res;
    }
    
    MonoValue MonoStruct::operator[](const std::string& key) {
        std::map<std::string, MonoValue> fields =  getFields();
        MonoValue res(0);
        for (auto it = fields.begin(); it != fields.end(); it++) {
            if (it->first == key) {
                res = it->second;
            } else {
                DeleteMonoValue(it->second);
            }
        }
        return res;
    }
}
