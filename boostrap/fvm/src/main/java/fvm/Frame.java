package fvm;

import java.util.Optional;

public class Frame {
    private Value stack = Nil.instance();

    private Envrionment env;

    public void pushStack(Value value) {
        this.stack = new Pair(value, this.stack);
    }

    public Value popStack() {
        if (!(stack instanceof Pair)) {
            throw new IllegalStateException("Stack pop on empty stack");
        }
        Pair pair = (Pair) stack;
        stack = pair.second();
        return pair.first();
    }

    public Optional<Value> lookup(Symbol symbol) {
        return this.env.lookup(symbol);
    }
}
