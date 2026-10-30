// A String reference can't be compared with a String value (§6.9).

boolean isYes(String* answer) {
    return answer == "yes"; // error
}

void main() {
    String answer = "yes";
    println(isYes(answer));
}
