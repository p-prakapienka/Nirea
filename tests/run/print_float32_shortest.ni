// A float prints in the shortest form that reads back as the same float
// value, not as the digits of its float64 widening (§15).

void main() {
    float tenth = 0.1;
    println(tenth);                 // expect: 0.1
}
