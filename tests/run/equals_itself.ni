// equals takes its argument by value, so a value can be compared with itself,
// directly or through a reference (§6.9).

struct Engine {
    int power;
    String name;
}

void main() {
    Engine engine = Engine();
    engine.name = "v8";
    println(engine.equals(engine));      // expect: true
    Engine* reference = engine;
    println(reference.equals(reference)); // expect: true
    int[] numbers = {1, 2};
    println(numbers.equals(numbers));    // expect: true
}
