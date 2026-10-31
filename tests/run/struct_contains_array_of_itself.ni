// An array's elements are stored outside the struct that owns it, so a struct
// can contain an array of itself (§5.2, §6.6).

struct Node {
    int value;
    Node[] children;
}

int total(Node* node) {
    int sum = node.value;
    for (int i = 0; i < node.children.length; i++) {
        sum += total(node.children[i]);
    }
    return sum;
}

void main() {
    Node root = Node();
    root.value = 1;
    root.children = Node[2];
    root.children[0].value = 2;
    root.children[1].value = 3;
    root.children[1].children = Node[1];
    root.children[1].children[0].value = 4;
    Node copy = root.copy();
    root.children[1].children[0].value = 40;
    println(total(root));            // expect: 46
    println(total(copy));            // expect: 10
    println(root == copy);           // expect: false
}
