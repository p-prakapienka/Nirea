// A file sees only its own declarations and those of the files it imports
// directly (§12.3). Main.ni doesn't import Other.ni.

void main() {
    println(otherValue());
}
