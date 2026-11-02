// Inside a global object's methods, a call that can reach the object again
// ends the reference variables into its state (§9.3).

struct Entry {
    String name;
}

struct Store {
    private Entry[] entries;

    void grow() {
        entries = Entry[entries.length + 1];
    }

    void rename(int i, String name) {
        Entry* entry = entries[i];
        LOG.write("rename");
        entry.name = name; // error: the reference ended
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
