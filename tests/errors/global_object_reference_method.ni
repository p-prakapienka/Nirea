// A method that returns a reference can't be called on a global object, so
// no reference into it can be held outside its methods (§5.6).

struct Store {
    private int[] counts;

    int[]* all() {
        return counts;
    }
}

Store STORE = Store();

void main() {
    STORE.all(); // error: returns a reference
}
