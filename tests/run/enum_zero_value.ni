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
    Device device = Device();
    println(device.mode);           // expect: OFF
    Mode[2] modes = Mode[2]();
    println(modes[1]);              // expect: OFF
    if (device.mode == Mode.OFF) {
        println("first value");     // expect: first value
    }
}
