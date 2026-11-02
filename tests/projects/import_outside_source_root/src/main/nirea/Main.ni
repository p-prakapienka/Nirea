// A relative import can reach a file outside the source root, such as one
// shared by several projects. Such files are part of the program only when
// imported: shared/Unused.ni would clash with this main otherwise (§12.3).

import ../../../shared/Text.ni;

void main() {
    println(Text.centimetres(40));
}
