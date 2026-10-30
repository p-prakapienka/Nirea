// Namespaces can't be nested (§11). The declaration is on one line so the
// diagnostic location is unambiguous.

namespace Outer { namespace Inner { void helper() { println("inner"); } } } // error

void main() {
    println("nested");
}
