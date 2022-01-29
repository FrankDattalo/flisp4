package fvm;

public class App {
    public static void main(java.lang.String[] args) {
        if (args.length != 2) {
            throw new IllegalStateException("Two arguments are required: <inputFile>.flang <outputFile>.fasm");
        }
        Compiler compiler = new Compiler();
        compiler.compile(args[0], args[1]);
    }
}
