import lib/Shapes.ni;
import app/feature/Feature.ni;

void main() {
    Square square = makeBlueSquare(4);
    describeSquare(square);
    println(featureTotal());
}
