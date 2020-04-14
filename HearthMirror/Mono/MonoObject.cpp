//
//  MonoObject.cpp
//  MonoReader
//
//  Created by Istvan Fehervari on 23/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#include "MonoObject.hpp"
#include "MonoClass.hpp"

namespace hearthmirror {

    MonoObject::MonoObject(HANDLE task, proc_address pObject, bool is64bit) : _task(task), _pObject(pObject), _vtable(ReadPointer(_task, pObject, is64bit)), _is64bit(is64bit) {}

    MonoObject::~MonoObject() {}
    
    MonoClass* MonoObject::getClass() {
        return new MonoClass(_task, ReadPointer(_task, _vtable, _is64bit), _is64bit);
    }
    
    std::map<std::string, MonoValue> MonoObject::getFields() {
        printf("In MonoObject::getFields - 1");
        MonoClass* c = getClass();
        std::vector<MonoClassField*> fields = c->getFields();
        delete c;
        
        std::map<std::string, MonoValue> res;
        
        for (MonoClassField* f : fields) {
            MonoType* type = f->getType();
			if (type) {
				if (!type->isStatic()) {
					auto name = f->getName();
					if (!name.empty()) {
                        printf("In MonoObject::getFields - 2 Parse field: %s\n",name.c_str());
						res[name] = f->getValue(this);
					}
				}
				delete type;
			}
			
            delete f;
        }

        return res;
    }
    
    MonoValue MonoObject::operator[](const std::string& key) {
        printf("In MonoObject::operator - 1");
        std::map<std::string, MonoValue> fields = getFields();
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

