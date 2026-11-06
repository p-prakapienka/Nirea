// A global object can be declared inside a namespace and is used through the
// namespace name (§5.6, §11).

namespace Shop {
    struct Basket {
        private int items;

        void add() {
            items++;
        }

        int count() {
            return items;
        }
    }

    Basket BASKET = Basket();

    void buy() {
        BASKET.add();
    }
}

void main() {
    Shop.buy();
    Shop.BASKET.add();
    println(Shop.BASKET.count());   // expect: 2
}
