// A default branch handles the enum values not listed (§10.2).

enum Weekday {
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    SUNDAY
}

boolean isWeekend(Weekday day) {
    return switch (day) {
        case SATURDAY, SUNDAY -> true;
        default -> false;
    };
}

void announce(Weekday day) {
    switch (day) {
        case MONDAY -> println("start");
        default -> println("not monday");
    }
}

void main() {
    if (isWeekend(Weekday.SUNDAY)) {
        println("weekend");         // expect: weekend
    }
    if (!isWeekend(Weekday.WEDNESDAY)) {
        println("weekday");         // expect: weekday
    }
    announce(Weekday.MONDAY);       // expect: start
    announce(Weekday.FRIDAY);       // expect: not monday
}
