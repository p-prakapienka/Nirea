// a.equals(b) compares the values references point to, in place (§6.9).

struct Engine {
    int power;
    String name;

    Engine(int power, String name) {
        this.power = power;
        this.name = name;
    }

    boolean matches(Engine* other) {
        return this.equals(other);
    }
}

boolean sameContents(Engine* a, Engine* b) {
    return a.equals(b);
}

boolean isYes(String* answer) {
    return answer.equals("yes");
}

int total(int[3]* values, int[3]* expected) {
    if (values.equals(expected)) {
        return 1;
    }
    return 0;
}

void main() {
    Engine first = Engine(100, "v8");
    Engine second = Engine(100, "v8");
    Engine third = Engine(90, "v6");
    println(sameContents(first, second));    // expect: true
    println(sameContents(first, third));     // expect: false
    println(first.matches(second));          // expect: true
    println(first.equals(Engine(100, "v8"))); // expect: true

    String answer = "yes";
    println(isYes(answer));                  // expect: true
    answer.appendChar('!');
    println(isYes(answer));                  // expect: false

    int[3] a = {1, 2, 3};
    int[3] b = {1, 2, 3};
    println(total(a, b));                    // expect: 1
    println(a.equals(b) == (a == b));        // expect: true
}
