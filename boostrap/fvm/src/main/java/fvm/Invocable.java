package fvm;

public interface Invocable {
    int arity();

    void apply(VirtualMachine vm);
}
