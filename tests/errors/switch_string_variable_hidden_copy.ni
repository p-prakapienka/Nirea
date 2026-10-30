// A String switch expression that picks an existing String would act on a copy (§10.1).

void main() {
    String a = "a";
    String b = "b";
    boolean pick = true;
    println(switch (pick) { case true -> a; default -> b; }); // error
}
