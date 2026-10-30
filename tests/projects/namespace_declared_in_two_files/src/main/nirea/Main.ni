// Each namespace is declared once in a program (§11): Tools is declared in
// both First.ni and Second.ni.
import First.ni;
import Second.ni;

void main() {
    Tools.first();
    Tools.second();
}
