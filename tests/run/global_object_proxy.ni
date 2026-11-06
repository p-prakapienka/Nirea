// A global object can forward each call to another one, for example to choose
// an implementation in tests (§5.6).

interface Greeter {
    String greet(String name);
}

struct FormalGreeter : Greeter {
    String greet(String name) {
        return "Good day, " + name;
    }
}

struct CasualGreeter : Greeter {
    String greet(String name) {
        return "Hi " + name;
    }
}

struct Settings {
    private boolean casual;

    void setCasual(boolean casual) {
        this.casual = casual;
    }

    boolean isCasual() {
        return casual;
    }
}

struct Greeters : Greeter {
    String greet(String name) {
        if (SETTINGS.isCasual()) {
            return CASUAL.greet(name);
        }
        return FORMAL.greet(name);
    }
}

FormalGreeter FORMAL = FormalGreeter();
CasualGreeter CASUAL = CasualGreeter();
Settings SETTINGS = Settings();
Greeters GREETER = Greeters();

void main() {
    println(GREETER.greet("Ann"));  // expect: Good day, Ann
    SETTINGS.setCasual(true);
    println(GREETER.greet("Ann"));  // expect: Hi Ann
}
