// String switches match by contents and have a default branch (§6.5, §10.1, §10.2).

int code(String command) {
    return switch (command) {
        case "start" -> 1;
        case "stop", "halt" -> 2;
        default -> 0;
    };
}

void main() {
    println(code("start"));         // expect: 1
    println(code("halt"));          // expect: 2
    println(code("stop"));          // expect: 2
    println(code("pause"));         // expect: 0
    String command = "st\x61rt";
    println(code(command));         // expect: 1
}
