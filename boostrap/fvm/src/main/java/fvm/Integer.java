package fvm;

public class Integer implements Value {
    long value;

    public Integer(long value) {
        this.value = value;
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof Integer)) {
            return false;
        }
        Integer casted = (Integer) obj;
        return this.value == casted.value;
    }

    @Override
    public int hashCode() {
        return java.lang.Long.hashCode(value);
    }

    @Override
    public java.lang.String toString() {
        return Long.toString(value);
    }
}
