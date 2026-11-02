// A local variable declared without an initialiser has no value until it is
// assigned; assigning to a field of it before then is a use (§6.8).

struct Circle {
    float radius;
}

void main() {
    Circle circle;
    circle.radius = 2.0; // error: it has no value yet
}
