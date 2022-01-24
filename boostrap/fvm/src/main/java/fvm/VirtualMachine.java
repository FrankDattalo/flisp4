package fvm;

import java.util.Optional;

public class VirtualMachine {
    private Envrionment global;

    private Frame current;

    public VirtualMachine() {

    }

    public void invoke(Invocable fn, int args) {
        Frame frame = new Frame();
        if (fn.arity() != args) {
            throw new IllegalStateException(java.lang.String.format("Invalid arity, wanted %s, but got %s", fn.arity(), args));
        }
        
        fn.apply(this);
        current.pushStack(frame.popStack());
    }

    public void pushStack(Value value) {
        current.pushStack(value);
    }

    public Optional<Value> lookup(Symbol symbol) {
        return current.lookup(symbol);
    }

    public void registerNativeFunction(Symbol symbol, NativeFunction fn) {
        global.define(symbol, fn);
    }
}
