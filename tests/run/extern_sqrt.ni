// Calling C's sqrt through an extern "C" block; float64 maps to double (§14).

extern "C" {
    float64 sqrt(float64 x);
}

void main() {
    println(sqrt(6.25));            // expect: 2.5
    float64 one = 1.0;
    println(sqrt(one));             // expect: 1.0
    if (sqrt(16.0) == 4.0) {
        println("four");            // expect: four
    }
}
