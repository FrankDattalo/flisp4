package fvm;

import java.util.List;

public interface NativeFunction extends Value, Invocable {

    @Override
    default int arity() {
        return this.parameters().size();
    }

    List<Symbol> parameters();
}
