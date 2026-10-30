// Colors.ni is in the same directory as this file (§12.2).
import Colors.ni;

struct Square {
    int side;
    Color color;
}

Square makeBlueSquare(int side) {
    Square square;
    square.side = side;
    square.color = Color.BLUE;
    return square;
}

void describeSquare(Square* square) {
    print(square.side);
    print(" ");
    println(square.color);
}
