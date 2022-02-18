// #include "cmd/entry.hh"
#include "vm.hh"

#include <iostream>
#include <string>

void print(Primitive p) {
    struct ObjVisitor : Object::Visitor {
        void OnPair(const Pair * p) override {
            std::cout << "pair ("; 
            print(p->ConstFirst());
            std::cout << " . ";
            print(p->ConstSecond());
            std::cout << ")";
        }
        void OnVector(const Vector* casted) override {
            std::cout << "[";
            std::size_t n = casted->Length().Value();
            for (std::size_t i = 0; i < n; i++) {
                if (i != 0) {
                    std::cout << ", ";
                }
                print(casted->GetItem(Integer(i)));
            }
            std::cout << "]";
        }
        void OnString(const String* casted) override {
            std::cout << "\"";
            std::size_t n = casted->Length().Value();
            for (std::size_t i = 0; i < n; i++) {
                std::cout << casted->GetChar(Integer(i)).Value();
            }
            std::cout << "\"";
        }
        void OnMap(const Map* obj) override { std::cout << "todo"; }
        void OnEnvrionment(const Envrionment* obj) override { std::cout << "todo"; }
        void OnStack(const Stack* obj) override { std::cout << "todo"; }
        void OnFrame(const Frame* obj) override { std::cout << "todo"; }
        void OnNativeFunction(const NativeFunction* obj) override { std::cout << "todo"; }
        void OnLambda(const Lambda* obj) override { std::cout << "todo"; }
        void OnContinuation(const Continuation* obj) override { std::cout << "todo"; }
    } obj_visitor;

    struct PrimVisitor : Primitive::Visitor {
        ObjVisitor& obj_visitor;

        PrimVisitor(ObjVisitor& _obj_visitor): obj_visitor{_obj_visitor} {}

        void OnNil(const Nil* obj) override {
            std::cout << "nil";
        }
        void OnInteger(const Integer* obj) override {
            std::cout << "integer " << obj->Value();
        }
        void OnReal(const Real* obj) override {
            std::cout << "real " << obj->Value();
        }
        void OnSymbol(const Symbol* obj) override {
            std::cout << "symbol " << obj->Value();
        }
        void OnReference(const Reference* obj) override {
            Object* ref = obj->Value();
            ref->Visit(obj_visitor);
        }
        void OnBoolean(const Boolean* obj) override {
            std::cout << "boolean " << (obj->Value() ? "true" : "false");
        }
        void OnCharacter(const Character* obj) override {
            std::cout << "char " << obj->Value();
        }
        void OnNativeReference(const NativeReference* obj) override {
            std::cout << "native " << obj->Value();
        }
    } visitor(obj_visitor);

    p.Visit(visitor);
}

int main(int argc, char** argv) {

    // Primitive obj;
    // print(obj);
    // std::cout << std::endl;
// 
    // obj.SetInteger(1);
    // print(obj);
    // std::cout << std::endl;
// 
    // obj.SetSymbol(2);
    // print(obj);
    // std::cout << std::endl;
// 
    // obj.SetReference(nullptr);
    // print(obj);
    // std::cout << std::endl;
// 
    // obj.SetBoolean(true);
    // print(obj);
    // std::cout << std::endl;
// 
    // obj.SetCharacter('a');
    // print(obj);
    // std::cout << std::endl;
// 
    // obj.SetReal(1.5);
    // print(obj);
    // std::cout << std::endl;
// 
    // obj.SetNativeReference(&obj);
    // print(obj);
    // std::cout << std::endl;
// 
    // Heap heap{1000};
// 
    // Object* ref = heap.NewPair(Primitive::Nil(), Primitive::Integer(1));
    // obj.SetReference(ref);
    // print(obj);
    // std::cout << std::endl;
// 
    // ref = heap.NewVector(10);
    // obj.SetReference(ref);
    // obj.GetReference()->AsVector()->SetItem(1, Primitive::Integer(1));
    // obj.GetReference()->AsVector()->SetItem(0, Primitive::Integer(22));
    // print(obj);
    // std::cout << std::endl;
// 
    // ref = heap.NewString("hello, world!");
    // obj.SetReference(ref);
    // print(obj);
    // std::cout << std::endl;

    // Heap heap2{200};
    // Handle handle = heap2.GetHandle(Primitive::Nil());
    // Handle pairHandle = heap2.GetHandle(Primitive::Nil());
    // pairHandle->SetReference(heap2.NewPair(Primitive::Nil(), Primitive::Nil()));
    // for (std::size_t i = 0; i < 100; i++) {
    //     std::cout << "Iteration " << i << std::endl;
    //     std::string str{"Some string to allocate "};
    //     str.append(std::to_string(i));
    //     handle->SetReference(heap2.NewString(str));
    //     print(handle.GetData());
    //     std::cout << std::endl;
    //     std::string line;
    //     std::cout << "Press [Enter] to continue...";
    //     std::getline(std::cin, line);
    // }

    // Heap heap3{1000};

    VirtualMachine vm;

    return 0;
}