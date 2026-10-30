// Floats print in their shortest round-trip form, also next to powers of two (§15).

void main() {
    float f = 1.0;
    for (int i = 0; i < 87; i++) {
        f = f * 2.0;
    }
    println(f);               // expect: 1.5474251E26
    float64 d = 1.0;
    for (int i = 0; i < 1017; i++) {
        d = d / 2.0;
    }
    println(d);               // expect: 7.120236347223045E-307
}
