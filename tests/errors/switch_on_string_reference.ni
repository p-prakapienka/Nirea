// A switch compares its subject with values, so the subject can't be a reference (§10.1).

int score(String* answer) {
    return switch (answer) { // error
        case "yes" -> 1;
        default -> 0;
    };
}

void main() {
    String answer = "yes";
    println(score(answer));
}
