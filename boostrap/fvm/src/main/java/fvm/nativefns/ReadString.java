package fvm.nativefns;

import fvm.VirtualMachine;

import java.util.Collections;
import java.util.List;

import fvm.NativeFunction;
import fvm.Symbol;

public class ReadString implements NativeFunction {

    @Override
    public List<Symbol> parameters() {
        return Collections.emptyList();
    }

    @Override
    public void apply(VirtualMachine vm) {
        String str = System.console().readLine();
        vm.pushStack(new fvm.String(str));
    }
}
