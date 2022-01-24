package fvm;

public class App {
    public static void main(java.lang.String[] args) {
        Reader reader = new Reader();
        Value value = reader.read("1");
        System.out.println(value);
    }
}
