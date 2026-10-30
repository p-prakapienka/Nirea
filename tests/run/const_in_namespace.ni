// Constants can be declared inside a namespace; they are used unqualified
// inside it and qualified outside (§5.5, §11).

namespace Config {
    enum Level {
        LOW,
        HIGH
    }

    const int RETRIES = 3;
    const String NAME = "config";
    const Level DEFAULT_LEVEL = Level.HIGH;

    int doubledRetries() {
        return RETRIES * 2;
    }
}

void main() {
    println(Config.RETRIES);            // expect: 3
    println(Config.NAME);               // expect: config
    println(Config.doubledRetries());   // expect: 6
    if (Config.DEFAULT_LEVEL == Config.Level.HIGH) {
        println("high");                // expect: high
    }
}
