// An interface declared in a namespace can be implemented by a struct outside
// it, using the qualified name (§5.3, §11).

namespace Graphics {
    interface Drawable {
        void draw();
    }

    void render(Drawable* shape) {
        shape.draw();
    }
}

struct Circle : Graphics.Drawable {
    int radius;

    void draw() {
        print("circle ");
        println(radius);
    }
}

void main() {
    Circle circle;
    circle.radius = 2;
    Graphics.render(circle);        // expect: circle 2
}
