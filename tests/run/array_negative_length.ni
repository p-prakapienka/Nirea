// Creating an array with a negative length is a runtime error (§6.4, §6.6).

void main() {
    int count = 2 - 5;
    int[] values = int[count]; // runtime-error: negative array length
}
