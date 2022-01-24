package fvm;

public class String implements Value {
    private java.lang.String value;

    public String(java.lang.String value) {
        this.value = value;
    }

    public java.lang.String value() {
        return this.value;
    }

    @Override
    public int hashCode() {
        return value.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof String)) {
            return false;
        }
        String casted = (String) obj;
        return this.value.equals(casted.value);
    }

    @Override
    public java.lang.String toString() {
        return value.toString();
    }
}
