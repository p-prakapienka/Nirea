// A method that returns `this` as a reference lets calls be chained; each call
// changes the same object (§9.2).

struct Builder {
    int width;
    int height;
    String label;

    Builder* setWidth(int width) {
        this.width = width;
        return this;
    }

    Builder* setHeight(int height) {
        this.height = height;
        return this;
    }

    Builder* setLabel(String label) {
        this.label = label;
        return this;
    }
}

void main() {
    Builder builder = Builder();
    builder.setWidth(10).setHeight(20).setLabel("box");
    println(builder.width * builder.height);  // expect: 200
    println(builder.label);                   // expect: box

    Builder* same = builder.setWidth(30);
    same.height = 2;
    println(builder.width * builder.height);  // expect: 60

    Builder copy = builder.setWidth(4).copy();
    copy.width = 5;
    println(builder.width);                   // expect: 4
    println(copy.width);                      // expect: 5

    Builder made = Builder().setWidth(7).setLabel("temp").copy();
    println(made.width);                      // expect: 7
    println(made.label);                      // expect: temp
}
