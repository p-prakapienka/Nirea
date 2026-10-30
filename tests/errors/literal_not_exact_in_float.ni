// An integer literal can be used as a floating-point type only if that type
// represents its value exactly; 16777217 isn't exact in float (§6.3).

void main() {
    float value = 16777217; // error
}
