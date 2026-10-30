// A boolean switch covering both values needs no default; one value plus a
// default is also exhaustive (§10.2).

String describe(boolean flag) {
    return switch (flag) {
        case true -> "on";
        case false -> "off";
    };
}

void main() {
    println(describe(true));        // expect: on
    println(describe(1 > 2));       // expect: off
    boolean ready = true;
    switch (ready) {
        case false -> println("waiting");
        default -> println("ready");    // expect: ready
    }
}
