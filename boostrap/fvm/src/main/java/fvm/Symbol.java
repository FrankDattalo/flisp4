package fvm;

public class Symbol implements Value {

    private java.lang.String value;

    public Symbol(java.lang.String value) {
        this.value = value.intern();
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof Symbol)) {
            return false;
        }
        Symbol casted = (Symbol) other;
        return casted.value == this.value;
    }

    @Override
    public int hashCode() {
        return value.hashCode();
    }

    @Override
    public java.lang.String toString() {
        return value.toString();
    }
}
