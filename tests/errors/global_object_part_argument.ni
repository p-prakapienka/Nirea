// A call that can reach a global object again can't receive a reference into
// its state as an argument; a value argument receives a copy (§9.3).

struct Entry {
    String name;
}

struct Store {
    private Entry[] entries;

    void grow() {
        entries = Entry[entries.length + 1];
    }

    void archive(int i) {
        LOG.keep(entries[i]);
        LOG.inspect(entries[i]); // error: can reach global object 'STORE'
    }
}

struct Log {
    void keep(Entry entry) {
        STORE.grow();
    }

    void inspect(Entry* entry) {
        STORE.grow();
        println(entry.name);
    }
}

Store STORE = Store();
Log LOG = Log();

void main() {
    STORE.archive(0);
}
