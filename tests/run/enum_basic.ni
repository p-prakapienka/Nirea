// Enum values are written qualified, support == and !=, and are copied like
// primitive values (§6.7).

enum Direction {
    NORTH,
    EAST,
    SOUTH,
    WEST,
}

struct Compass {
    Direction heading;
}

Direction turnRight(Direction direction) {
    return switch (direction) {
        case NORTH -> Direction.EAST;
        case EAST -> Direction.SOUTH;
        case SOUTH -> Direction.WEST;
        case WEST -> Direction.NORTH;
    };
}

void main() {
    Direction heading = Direction.NORTH;
    Direction saved = heading;
    heading = turnRight(heading);
    println(saved);                 // expect: NORTH
    println(heading);               // expect: EAST
    if (heading != saved) {
        println("changed");         // expect: changed
    }
    if (turnRight(Direction.WEST) == Direction.NORTH) {
        println("wrapped");         // expect: wrapped
    }

    Compass compass = Compass();
    compass.heading = heading;
    Direction fromField = compass.heading;
    println(fromField);             // expect: EAST
    println(compass.heading);       // expect: EAST
}
