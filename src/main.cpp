// #include "cmd/entry.hh"
#include "objects/primitive.hh"
#include "heap.hh"

#include <iostream>
#include <string>

using namespace runtime;

void print(Primitive p) {
    struct ObjVisitor : Object::Visitor {
        void OnPair(const Object* obj) override {
            const Pair* p = obj->AsConstPair();
            std::cout << "pair ("; 
            print(p->GetFirst());
            std::cout << " . ";
            print(p->GetSecond());
            std::cout << ")";
        }
        void OnVector(const Object* obj) override {
            std::cout << "[";
            const Vector* casted = obj->AsConstVector();
            std::size_t n = casted->GetLength().GetInteger();
            for (std::size_t i = 0; i < n; i++) {
                if (i != 0) {
                    std::cout << ", ";
                }
                print(casted->GetItem(i));
            }
            std::cout << "]";
        }
        void OnString(const Object* obj) override {
            std::cout << "\"";
            const String* casted = obj->AsConstString();
            std::size_t n = casted->GetLength().GetInteger();
            for (std::size_t i = 0; i < n; i++) {
                std::cout << casted->GetChar(i).GetCharacter();
            }
            std::cout << "\"";
        }
        void OnMap(const Object* obj) override { std::cout << "todo"; }
        void OnEnvrionment(const Object* obj) override { std::cout << "todo"; }
        void OnStack(const Object* obj) override { std::cout << "todo"; }
        void OnFrame(const Object* obj) override { std::cout << "todo"; }
        void OnVirtualMachine(const Object* obj) override { std::cout << "todo"; }
        void OnNativeFunction(const Object* obj) override { std::cout << "todo"; }
        void OnFunction(const Object* obj) override { std::cout << "todo"; }
        void OnClosure(const Object* obj) override { std::cout << "todo"; }
        void OnSymbolTable(const Object* obj) override { std::cout << "todo"; }
    } obj_visitor;

    struct PrimVisitor : Primitive::Visitor {
        ObjVisitor& obj_visitor;

        PrimVisitor(ObjVisitor& _obj_visitor): obj_visitor{_obj_visitor} {}

        void OnNil(const Primitive* obj) override {
            std::cout << "nil";
        }
        void OnInteger(const Primitive* obj) override {
            std::cout << "integer " << obj->GetInteger();
        }
        void OnReal(const Primitive* obj) override {
            std::cout << "real " << obj->GetReal();
        }
        void OnSymbol(const Primitive* obj) override {
            std::cout << "symbol " << obj->GetSymbol();
        }
        void OnReference(const Primitive* obj) override {
            Object* ref = obj->GetReference();
            ref->Visit(obj_visitor);
        }
        void OnBoolean(const Primitive* obj) override {
            std::cout << "boolean " << (obj->GetBoolean() ? "true" : "false");
        }
        void OnCharacter(const Primitive* obj) override {
            std::cout << "char " << obj->GetCharacter();
        }
        void OnNativeReference(const Primitive* obj) override {
            std::cout << "native " << obj->GetNativeReference();
        }
    } visitor(obj_visitor);

    p.Visit(visitor);
}

int main(int argc, char** argv) {

    Primitive obj;
    print(obj);
    std::cout << std::endl;

    obj.SetInteger(1);
    print(obj);
    std::cout << std::endl;

    obj.SetSymbol(2);
    print(obj);
    std::cout << std::endl;

    obj.SetReference(nullptr);
    print(obj);
    std::cout << std::endl;

    obj.SetBoolean(true);
    print(obj);
    std::cout << std::endl;

    obj.SetCharacter('a');
    print(obj);
    std::cout << std::endl;

    obj.SetReal(1.5);
    print(obj);
    std::cout << std::endl;

    obj.SetNativeReference(&obj);
    print(obj);
    std::cout << std::endl;

    Heap heap{1000};

    Object* ref = heap.NewPair(Primitive::Nil(), Primitive::Integer(1));
    obj.SetReference(ref);
    print(obj);
    std::cout << std::endl;

    ref = heap.NewVector(10);
    obj.SetReference(ref);
    obj.GetReference()->AsVector()->SetItem(1, Primitive::Integer(1));
    obj.GetReference()->AsVector()->SetItem(0, Primitive::Integer(22));
    print(obj);
    std::cout << std::endl;

    ref = heap.NewString("hello, world!");
    obj.SetReference(ref);
    print(obj);
    std::cout << std::endl;

    Heap heap2{200};
    Handle handle = heap2.GetHandle(Primitive::Nil());
    Handle pairHandle = heap2.GetHandle(Primitive::Nil());
    pairHandle->SetReference(heap2.NewPair(Primitive::Nil(), Primitive::Nil()));
    for (std::size_t i = 0; i < 100; i++) {
        std::cout << "Iteration " << i << std::endl;
        std::string str{"Some string to allocate "};
        str.append(std::to_string(i));
        handle->SetReference(heap2.NewString(str));
        print(handle.GetData());
        std::cout << std::endl;
        std::string line;
        std::cout << "Press [Enter] to continue...";
        std::getline(std::cin, line);
    }

    Heap heap3{10000000};

    heap3.NewVirtualMachine()->Execute();

    return 0;
}