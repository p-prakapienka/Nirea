// A local isn't moved while something still uses it: a reference variable used
// afterwards, the object a call is made on, or the target of an assignment
// (§7.2).

struct Engine {
    int power;
}

struct Tank {
    int[] levels;

    void absorb(Tank other) {
        println(levels.length + other.levels.length);
    }
}

struct Node {
    int value;
    Node[] children;
}

int sum(Engine a, Engine b) {
    return a.power + b.power;
}

void main() {
    Engine engine = Engine();
    engine.power = 1;
    Engine* reference = engine;
    Engine other = engine;
    other.power = 2;
    println(reference.power);       // expect: 1

    Tank tank = Tank();
    tank.levels = int[3];
    tank.absorb(tank);              // expect: 6
    Tank* same = tank;
    same.absorb(tank);              // expect: 6

    Node root = Node();
    root.value = 5;
    root.children = Node[1];
    root.children[0] = root;
    println(root.children[0].value);            // expect: 5
    println(root.children[0].children.length);  // expect: 1

    Engine twice = Engine();
    twice.power = 4;
    println(sum(twice, twice));     // expect: 8
}
