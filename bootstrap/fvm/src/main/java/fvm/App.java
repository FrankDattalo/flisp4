package fvm;

public class App {
    public static void main(java.lang.String[] args) {
        Compiler compiler = new Compiler();
        compiler.compile(args[0], args[1]);
    }
}
