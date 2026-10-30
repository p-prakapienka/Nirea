// Line comments and block comments may appear between tokens (Appendix A).

/* A block comment
   spanning several lines. */

void main() {
    println("start"); // a trailing line comment
    /* a block comment before a statement */ println("middle");
    int total = 1 /* between tokens */ + 2;
    println(total);
    println("// inside a string is not a comment");
    println("/* neither is this */");
}

// expect: start
// expect: middle
// expect: 3
// expect: // inside a string is not a comment
// expect: /* neither is this */
