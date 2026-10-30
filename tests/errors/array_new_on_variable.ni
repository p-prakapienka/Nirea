// T[N]() creates an array only when T is a type; an element of a variable
// can't be called (§6.6, Appendix A).

void main() {
    int[3] values = int[3]();
    int[3] other = values[2](); // error: an array element can't be called
}
