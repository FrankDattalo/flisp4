// #include "cmd/entry.hh"
#include "refactor/object.hh"

#include <iostream>

using namespace runtime;

int main(int argc, char** argv) {
    // return cmd::entry(argc, argv);
    struct Visitor : Primitive::Visitor {
        void OnNil(const Primitive* obj) {
            std::cout << "nil" << std::endl;
        }
        void OnInteger(const Primitive* obj) {
            std::cout << "integer " << obj->GetInteger() << std::endl;
        }
        void OnReal(const Primitive* obj) {
            std::cout << "real " << obj->GetReal() << std::endl;
        }
        void OnSymbol(const Primitive* obj) {
            std::cout << "symbol " << obj->GetSymbol() << std::endl;
        }
        void OnReference(const Primitive* obj) {
            std::cout << "reference " << obj->GetReference() << std::endl;
        }
        void OnBoolean(const Primitive* obj) {
            std::cout << "boolean " << (obj->GetBoolean() ? "true" : "false") << std::endl;
        }
        void OnCharacter(const Primitive* obj) {
            std::cout << "char " << obj->GetCharacter() << std::endl;
        }
        void OnNativeReference(const Primitive* obj) {
            std::cout << "native " << obj->GetNativeReference() << std::endl;
        }
    } visitor;

    Primitive obj;
    obj.Visit(visitor);

    obj.SetInteger(1);
    obj.Visit(visitor);

    obj.SetSymbol(2);
    obj.Visit(visitor);

    obj.SetReference(nullptr);
    obj.Visit(visitor);

    Object ref;
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