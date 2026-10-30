// The value returned by int main() is the process exit status (§5.1).
// exit: 42

int answer() {
    return 42;
}

int main() {
    println("returning");           // expect: returning
    return answer();
}
