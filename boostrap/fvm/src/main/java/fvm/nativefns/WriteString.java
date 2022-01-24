package fvm.nativefns;

import java.util.Collections;
import java.util.List;

import fvm.NativeFunction;
import fvm.Symbol;
import fvm.Value;
import fvm.VirtualMachine;

public class WriteString implements NativeFunction {

    private static final Symbol str = new Symbol("str");

    @Override
    public List<Symbol> parameters() {
        return Collections.singletonList(str);
    }

    @Override
    public void apply(VirtualMachine vm) {
        Value value = vm.lookup(str).orElseThrow(IllegalStateException::new);
        if (!(value instanceof fvm.String)) {
            throw new IllegalStateException("Expected parameter to display to be string");
        }
        fvm.String casted = (fvm.String) value;
        System.out.print(casted.value());
    }
    
}
