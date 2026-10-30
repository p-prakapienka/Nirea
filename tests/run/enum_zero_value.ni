// The zero value of an enum is its first value (§6.8).

enum Mode {
    OFF,
    STANDBY,
    ON
}

struct Device {
    Mode mode;
}

void main() {
    Mode mode;
    println(mode);                  // expect: OFF
    Device device;
    println(device.mode);           // expect: OFF
    Mode[2] modes;
    println(modes[1]);              // expect: OFF
    if (mode == Mode.OFF) {
        println("first value");     // expect: first value
    }
}
