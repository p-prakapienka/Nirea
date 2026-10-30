// String.of creates text like print; toInt, toInt64 and toFloat64 parse it (§6.5).

enum Color { RED, GREEN }

void main() {
    println(String.of(42));              // expect: 42
    println(String.of(-7));              // expect: -7
    println(String.of(2.5));             // expect: 2.5
    float small = 0.1;
    println(String.of(small));           // expect: 0.1
    println(String.of(true));            // expect: true
    println(String.of('z'));             // expect: z
    println(String.of(Color.GREEN));     // expect: GREEN
    uint64 big = 18446744073709551615;
    println(String.of(big));             // expect: 18446744073709551615

    String number = "-123";
    println(number.toInt() + 1);         // expect: -122
    String wide = "9000000000";
    println(wide.toInt64());             // expect: 9000000000
    String real = "1.5e3";
    println(real.toFloat64());           // expect: 1500.0
    String text = String.of(12345);
    text.append("6");
    println(text.toInt());               // expect: 123456
}
