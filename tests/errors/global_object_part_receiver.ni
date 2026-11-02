// A call on a part of a global object's state can't run code that can reach
// the object again (§9.3).

struct Entry {
    String name;

    void rename(String name) {
        LOG.write("rename");
        this.name = name;
    }
}

struct Store {
    private Entry[] entries;

    void grow() {
        entries = Entry[entries.length + 1];
    }

    void rename(int i, String name) {
        entries[i].rename(name); // error: can reach global object 'STORE'
    }
}

struct Log {
    void write(String text) {
        STORE.grow();
    }
}

Store STORE = Store();
Log LOG = Log();

void main() {
    STORE.rename(0, "Ann");
}
