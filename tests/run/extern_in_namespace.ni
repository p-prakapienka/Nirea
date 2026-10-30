// An extern "C" block can appear inside a namespace. Its functions are called
// unqualified inside the namespace and qualified outside it (§11, §14).
// printf is used only at the end so its output order is unambiguous.

namespace CMath {
    extern "C" {
        float64 sqrt(float64 x);
    }

    float64 hypotenuse(float64 a, float64 b) {
        return sqrt(a * a + b * b);
    }
}

namespace Console {
    extern "C" {
        int printf(String format, ...);
    }

    void show(int value) {
        printf("%d\n", value);
    }
}

void main() {
    println(CMath.sqrt(6.25));              // expect: 2.5
    println(CMath.hypotenuse(1.5, 2.0));    // expect: 2.5
    Console.show(42);                       // expect: 42
}
