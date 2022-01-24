package fvm;

public class ListBuilder {
    
    private Value head = Nil.instance();
    private Value tail = Nil.instance();

    public void append(Value value) {
        if (head.equals(Nil.instance())) {
            head = new Pair(value, Nil.instance());
            tail = head;
        } else {
            Pair newTail = new Pair(value, Nil.instance());
            Pair currentTail = (Pair) tail;
            currentTail.setSecond(newTail);
            tail = newTail;
        }
    }

    public Value head() {
        return this.head;
    }
}
