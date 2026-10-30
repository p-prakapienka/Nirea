// A struct can implement several interfaces, and can have methods beyond
// those the interfaces require (§5.3).

interface Named {
    String name();
}

interface Measured {
    int size(int scale);
}

struct Box : Named, Measured {
    int side;

    String name() {
        return "box";
    }

    int size(int scale) {
        return side * scale;
    }

    int volume() {
        return side * side * side;
    }
}

void showName(Named* named) {
    println(named.name());
}

void showSize(Measured* measured) {
    println(measured.size(10));
}

void main() {
    Box box;
    box.side = 4;
    showName(box);                  // expect: box
    showSize(box);                  // expect: 40
    println(box.volume());          // expect: 64
}
