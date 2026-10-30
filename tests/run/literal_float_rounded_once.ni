// A float literal is rounded to the nearest float directly (§6.3).

void main() {
    float f = 1.0000000596046447755;
    println(f);               // expect: 1.0000001
}
