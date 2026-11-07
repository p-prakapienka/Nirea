// A call that can reach a global object again is allowed while no reference
// into its state is held, and references survive calls that can't reach it
// (§9.3).

struct Entry {
    String name;

    void rename(String name) {
        this.name = name;
    }
}

struct Store {
    private Entry[] entries;

    Store() {
        entries = Entry[1];
    }

    void grow() {
        Entry[] bigger = Entry[entries.length + 1];
        for (int i = 0; i < entries.length; i++) {
            bigger[i] = entries[i];
        }
        entries = bigger;
    }

    void rename(int i, String name) {
        LOG.write("rename");           // may call back into STORE.grow()
        entries[i].name = name; // reaches whatever array is there now
        Entry* entry = entries[i];
        TEXT.measure(name);            // can't reach STORE: entry stays usable
        entry.rename(name);            // Entry.rename can't reach STORE either
    }

    int size() {
        return entries.length;
    }

    String nameAt(int i) {
        return entries[i].name;
    }
}

struct Log {
    void write(String text) {
        STORE.grow();
    }
}

struct Text {
    int measure(String text) {
        return text.length();
    }
}

Store STORE = Store();
Log LOG = Log();
Text TEXT = Text();

void main() {
    STORE.rename(0, "Ann");
    println(STORE.size());          // expect: 2
    println(STORE.nameAt(0));       // expect: Ann
}
