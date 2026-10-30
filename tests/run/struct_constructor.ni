// A constructor is called like a function, using the struct's name. All fields
// hold their zero value before the body runs, and the constructor can set
// private fields (§5.4).

struct Account {
    int id;
    String owner;
    private int balance;

    Account(int id, int initial) {
        println(balance);
        this.id = id;
        balance = initial;
    }

    int getBalance() {
        return balance;
    }
}

struct Vec2 {
    float x;
    float y;

    Vec2(float x, float y) {
        this.x = x;
        this.y = y;
    }
}

void main() {
    Account account = Account(7, 100);      // expect: 0
    println(account.id);                    // expect: 7
    println(account.getBalance());          // expect: 100
    if (account.owner == "") {
        println("owner is zero");           // expect: owner is zero
    }

    Vec2 position = Vec2(2.5, -3.0);
    println(position.x);                    // expect: 2.5
    println(position.y);                    // expect: -3.0
    position = Vec2(1.0, 1.0);
    println(position.x);                    // expect: 1.0
}
