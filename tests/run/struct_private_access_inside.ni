// A private member can be used inside the methods and constructor of its
// struct, on any value of that struct type (§8).

struct Wallet {
    private int coins;

    Wallet(int coins) {
        this.coins = coins;
    }

    private int doubled() {
        return coins * 2;
    }

    boolean richerThan(Wallet* other) {
        return coins > other.coins;
    }

    int doubledTotal(Wallet* other) {
        return doubled() + other.doubled();
    }

    void takeAll(Wallet* other) {
        coins += other.coins;
        other.coins = 0;
    }

    int getCoins() {
        return coins;
    }
}

void main() {
    Wallet a = Wallet(5);
    Wallet b = Wallet(3);
    if (a.richerThan(b)) {
        println("a is richer");     // expect: a is richer
    }
    println(a.doubledTotal(b));     // expect: 16
    a.takeAll(b);
    println(a.getCoins());          // expect: 8
    println(b.getCoins());          // expect: 0
}
