package fvm;

public class Pair implements Value {
    private Value first;
    private Value second;

    public Pair(Value first, Value second) {
        this.first = first;
        this.second = second;
    }

    public Value first() {
        return this.first;
    }

    public void setFirst(Value v) {
        this.first = v;
    }

    public Value second() {
        return this.second;
    }

    public void setSecond(Value v) {
        this.second = v;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (!(obj instanceof Pair)) {
            return false;
        }
        Pair casted = (Pair) obj;
        return first.equals(casted.first) && second.equals(casted.second);
    }

    @Override
    public java.lang.String toString() {
        return "(" + first() + " . " + second() + ")";
    }
}
