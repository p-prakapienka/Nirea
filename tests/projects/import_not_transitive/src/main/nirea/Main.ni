// Imports aren't transitive (§12.3): Main.ni imports First.ni, which imports
// Second.ni, so secondValue is not visible here.
import First.ni;

void main() {
    println(firstValue());
    println(secondValue());
}
