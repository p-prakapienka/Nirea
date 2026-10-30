// && and || evaluate their right operand only when needed (§10).

boolean check(String label, boolean result) {
    println(label);
    return result;
}

void main() {
    if (check("a", false) && check("b", true)) {
        println("unexpected");
    }
    if (check("c", true) || check("d", true)) {
        println("or short-circuited");
    }
    if (check("e", true) && check("f", false)) {
        println("unexpected");
    } else {
        println("and evaluated both");
    }
    if (check("g", false) || check("h", true)) {
        println("or evaluated both");
    }
    int zero = 0;
    if (zero != 0 && 10 / zero > 1) {
        println("unexpected");
    }
    println("no division");
}

// expect: a
// expect: c
// expect: or short-circuited
// expect: e
// expect: f
// expect: and evaluated both
// expect: g
// expect: h
// expect: or evaluated both
// expect: no division
