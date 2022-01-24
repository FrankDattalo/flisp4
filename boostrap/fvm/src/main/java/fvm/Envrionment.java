package fvm;

import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

public class Envrionment {
    private Envrionment outer;
    private Map<Symbol, Value> lookup;

    public Envrionment() {
        lookup = new HashMap<>();
    }

    public Envrionment push() {
        Envrionment ret = new Envrionment();
        ret.outer = this;
        return ret;
    }

    public Optional<Envrionment> pop() {
        return Optional.ofNullable(outer);
    }

    public void define(Symbol symbol, Value value) {
        this.lookup.put(symbol, value);
    }

    public Optional<Value> lookup(Symbol symbol) {
        Envrionment env = this;
        while (env != null) {
            if (env.lookup.containsKey(symbol)) {
                return Optional.ofNullable(env.lookup.get(symbol));
            }
            env = env.outer;
        }
        return Optional.empty();
    }
}
