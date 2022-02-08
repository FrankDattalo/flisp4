// #include "cmd/entry.hh"
#include "refactor/object.hh"

#include <iostream>

using namespace runtime;

int main(int argc, char** argv) {
    // return cmd::entry(argc, argv);
    struct Visitor : Object::Visitor {
        void OnNil(const Object* obj) {
            std::cout << "nil" << std::endl;
        }
        void OnInteger(const Object* obj) {
            std::cout << "integer " << obj->GetInteger() << std::endl;
        }
        void OnReal(const Object* obj) {
            std::cout << "real " << obj->GetReal() << std::endl;
        }
        void OnSymbol(const Object* obj) {
            std::cout << "symbol " << obj->GetSymbol() << std::endl;
        }
        void OnReference(const Object* obj) {
            std::cout << "reference " << obj->GetReference() << std::endl;
        }
        void OnBoolean(const Object* obj) {
            std::cout << "boolean " << (obj->GetBoolean() ? "true" : "false") << std::endl;
        }
        void OnCharacter(const Object* obj) {
            std::cout << "char " << obj->GetCharacter() << std::endl;
        }
        void OnNativeReference(const Object* obj) {
            std::cout << "native " << obj->GetNativeReference() << std::endl;
        }
    } visitor;

    Object obj;
    obj.Visit(visitor);

    obj.SetInteger(1);
    obj.Visit(visitor);

    obj.SetSymbol(2);
    obj.Visit(visitor);

    obj.SetReference(nullptr);
    obj.Visit(visitor);

    HeapObject ref;
    obj.SetReference(&ref);
    obj.Visit(visitor);

    obj.SetBoolean(true);
    obj.Visit(visitor);

    obj.SetCharacter('a');
    obj.Visit(visitor);

    obj.SetReal(1.5);
    obj.Visit(visitor);

    obj.SetNativeReference(&obj);
    obj.Visit(visitor);
}