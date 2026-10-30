// Inside a namespace, its own members can be used without the namespace
// prefix: functions, structs, enums and constants (§11).

namespace Geometry {
    const int SQUARE_SIDES = 4;

    enum Kind {
        SQUARE,
        TRIANGLE
    }

    struct Polygon {
        Kind kind;
        int sides;
    }

    Polygon makeSquare() {
        Polygon polygon = Polygon();
        polygon.kind = Kind.SQUARE;
        polygon.sides = SQUARE_SIDES;
        return polygon;
    }

    int perimeter(Polygon* polygon, int length) {
        return polygon.sides * length;
    }

    int squarePerimeter(int length) {
        Polygon square = makeSquare();
        return perimeter(square, length);
    }
}

void main() {
    Geometry.Polygon square = Geometry.makeSquare();
    println(square.sides);                      // expect: 4
    println(Geometry.squarePerimeter(3));       // expect: 12
    println(Geometry.perimeter(square, 5));     // expect: 20
    println(Geometry.SQUARE_SIDES);             // expect: 4
    if (square.kind == Geometry.Kind.SQUARE) {
        println("square");                      // expect: square
    }
}
