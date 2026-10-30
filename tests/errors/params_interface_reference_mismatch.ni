// An implementing method must match the interface, including value versus reference (§5.3).

struct Item {
    int weight;
}

interface Store {
    void put(Item* item);
}

struct Box : Store {
    Item content;

    void put(Item item) { // error
        content = item;
    }
}

void main() {
    Box box;
    println(box.content.weight);
}
