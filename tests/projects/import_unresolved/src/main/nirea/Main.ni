// The compiler detects unresolved imports (§12.3): no such file exists.
import missing/Nowhere.ni;

void main() {
    println("unresolved");
}
