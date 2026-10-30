// Interface methods can take struct values and references (§5.3, §7.3).

struct Item {
    int weight;
}

interface Store {
    void put(Item item);
    int weightOf(Item* item);
}

struct Box : Store {
    Item content;

    void put(Item item) {
        content = item;
    }

    int weightOf(Item* item) {
        return item.weight * 2;
    }
}

void fill(Store* store) {
    Item item = Item();
    item.weight = 4;
    println(store.weightOf(item));    // expect: 8
    store.put(item);
}

void main() {
    Box box = Box();
    fill(box);
    println(box.content.weight);      // expect: 4
}
