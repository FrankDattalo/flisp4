package fvm;

public class Nil implements Value {

    private static final Nil INSTANCE;

    static {
        INSTANCE = new Nil();
    }

    private Nil() {
    }

    public static Nil instance() {
        return INSTANCE;
    }

    @Override
    public int hashCode() {
        return 0;
    }
    
    @Override
    public boolean equals(Object obj) {
        return obj == this;
    }

    @Override
    public java.lang.String toString() {
        return "'()";
    }
}
