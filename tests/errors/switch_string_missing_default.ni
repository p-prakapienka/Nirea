// String switches require a default branch (§10.2). The switch is on one line
// so the diagnostic location is unambiguous.

int code(String command) {
    return switch (command) { case "start" -> 1; case "stop" -> 2; }; // error
}

void main() {
    println(code("start"));
}
