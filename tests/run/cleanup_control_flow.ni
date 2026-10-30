// Locals are destroyed on every exit from their block: break, continue, return (§7.6).

String find(int stop) {
    String result = "none";
    for (int i = 0; i < 10; i++) {
        String label = String.of(i);
        if (i == 1) {
            continue;
        }
        if (i == stop) {
            String found = label.copy();
            found.append("!");
            return found;
        }
        if (i == 7) {
            break;
        }
        while (true) {
            String inner = "inner";
            break;
        }
    }
    return result;
}

void main() {
    println(find(3));                    // expect: 3!
    println(find(20));                   // expect: none
    String kept = "kept";
    String taken = kept;
    if (taken.length() > 0) {
        String other = taken;
        println(other);                  // expect: kept
    }
}
