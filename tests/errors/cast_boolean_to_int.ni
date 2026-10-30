// Casts are allowed between numeric types and between char and integer
// types; boolean can't be cast (§6.2).

void main() {
    boolean flag = true;
    int value = (int) flag; // error
}
