// Global objects are created before main in dependency order, whatever the
// order of their declarations (§5.6).

struct Config {
    String label;

    Config(String label) {
        println("config created");
        this.label = label;
    }

    String describe() {
        return "config " + label;
    }
}

struct Labels {
    Labels() {
        println("labels created");
    }

    String first() {
        return "primary";
    }
}

Config CONFIG = Config(LABELS.first());
Labels LABELS = Labels();

void main() {
    println("main");                // expect: labels created
    println(CONFIG.describe());     // expect: config created
}                                   // expect: main
                                    // expect: config primary
