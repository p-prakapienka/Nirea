// The result of a call that returns a reference can't be given to a new
// owner; copy() gives an independent object (§9.2).

struct Builder {
    int width;

    Builder* setWidth(int width) {
        this.width = width;
        return this;
    }
}

void main() {
    Builder builder;
    Builder copy = builder.setWidth(1).copy();
    Builder other = builder.setWidth(2); // error: returns a reference
}
