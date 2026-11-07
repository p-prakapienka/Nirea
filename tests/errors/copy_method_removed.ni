// There is no copy() method: assigning an object to a variable copies it (§7.2).

struct Engine {
    int power;
}

void main() {
    Engine engine = Engine();
    Engine spare = engine.copy(); // error: there is no copy() method
    String text = "a";
    String other = text.copy(); // error: there is no copy() method
    int[] values = int[2];
    int[] more = values.copy(); // error: there is no copy() method
}
