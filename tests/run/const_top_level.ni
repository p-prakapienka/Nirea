// Top-level constants of primitive, enum and String types, initialised from a
// literal, a negated numeric literal or an enum value (§5.5).

enum Mode {
    OFF,
    ON
}

const int MAX_BUFFER_SIZE = 1024;
const int MIN_OFFSET = -40;
const uint8 BYTE_MAX = 255;
const int64 BIG_COUNT = 9000000000;
const int MASK = 0xFF;
const float64 RATIO = 2.5;
const float NEGATIVE = -3.0;
const char SEPARATOR = ',';
const boolean ENABLED = true;
const String APP_NAME = "Nirea";
const Mode DEFAULT_MODE = Mode.ON;

void main() {
    println(MAX_BUFFER_SIZE);       // expect: 1024
    println(MIN_OFFSET);            // expect: -40
    println(BYTE_MAX);              // expect: 255
    println(BIG_COUNT);             // expect: 9000000000
    println(MASK);                  // expect: 255
    println(RATIO);                 // expect: 2.5
    println(NEGATIVE);              // expect: -3.0
    if (SEPARATOR == ',') {
        println("separator");       // expect: separator
    }
    if (ENABLED) {
        println("enabled");         // expect: enabled
    }
    println(APP_NAME);              // expect: Nirea
    println(DEFAULT_MODE);          // expect: ON
    int half = MAX_BUFFER_SIZE / 2;
    println(half);                  // expect: 512
}
