// Arrays can be struct fields, stored inline (§6.6).

struct Buffer {
    int[4] data;
    int count;

    void push(int value) {
        data[count] = value;
        count++;
    }

    int sum() {
        int total = 0;
        for (int i = 0; i < count; i++) {
            total += data[i];
        }
        return total;
    }
}

void main() {
    Buffer buffer = Buffer();
    buffer.push(3);
    buffer.push(4);
    println(buffer.sum());          // expect: 7
    println(buffer.data[1]);        // expect: 4
    println(buffer.data[2]);        // expect: 0
    println(buffer.data.length);    // expect: 4
}
