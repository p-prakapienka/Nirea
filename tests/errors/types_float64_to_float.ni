// float64 converts implicitly to nothing else; float64 to float needs a cast (§6.2).

void main() {
    float64 precise = 2.5;
    float single = precise; // error
}
