# Structs and interfaces

Nirea has no classes. Data lives in structs, which have fields and methods. Interfaces describe what a struct can do.

## Structs

```nirea
struct Account {
    private String owner;
    private int balance;

    Account(String owner) {                  // the constructor
        this.owner = owner;
    }

    void deposit(int amount) {
        balance += amount;                   // the same as this.balance
    }

    int getBalance() {
        return balance;
    }

    String describe() {
        return owner + ": " + String.of(balance);
    }
}

void main() {
    Account account = Account("Ann");
    account.deposit(50);
    account.deposit(25);
    println(account.getBalance());           // 75
    println(account.describe());             // Ann: 75
}
```

- Inside a method, fields and methods can be used by name or through `this`.
- A struct can have one constructor, named after the struct. There is no `new`: `Account("Ann")` creates the value.
- A struct without a constructor is created with `T()`, which sets every field to zero.
- Fields start at their zero value, even before the constructor runs.
- Members are public by default. A `private` member can only be used in the struct's own methods and constructor. There are no properties or generated getters: write the methods you need.

```nirea
account.balance = 1000000;               // error: 'balance' is private to struct 'Account'
```

A struct can hold other structs. It owns them, and they are destroyed with it:

```nirea
struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
    int speed;
}

void main() {
    Vehicle vehicle = Vehicle();
    vehicle.engine.power = 120;
    println(vehicle.engine.power);           // 120
}
```

## Equality

`==` compares values by their contents: structs field by field, arrays element by element, and strings byte by byte.

```nirea
struct Point {
    int x;
    int y;
}

boolean same(Point* a, Point* b) {
    return a.equals(b);                      // compares the points the references refer to
}

void main() {
    Point a = Point();
    a.x = 1;
    Point b = a.copy();
    println(a == b);                         // true
    b.y = 2;
    println(a == b);                         // false
    println(same(a, b));                     // false
}
```

`==` can't be used on references, so that nobody wonders whether it compares addresses or contents. Use `a.equals(b)`, which compares contents, exactly like `==` on values.

## Interfaces

A struct implements an interface by listing it after `:` and declaring every method of the interface. An interface is used through a reference, written `Shape*`. Calls through it go to the struct's own method.

```nirea
interface Shape {
    int area();
    String name();
}

struct Rectangle : Shape {
    int width;
    int height;

    Rectangle(int width, int height) {
        this.width = width;
        this.height = height;
    }

    int area() {
        return width * height;
    }

    String name() {
        return "rectangle";
    }
}

struct Square : Shape {
    int side;

    Square(int side) {
        this.side = side;
    }

    int area() {
        return side * side;
    }

    String name() {
        return "square";
    }
}

void describe(Shape* shape) {
    println(shape.name() + ": " + String.of(shape.area()));
}

void main() {
    Rectangle rectangle = Rectangle(2, 3);
    Square square = Square(4);
    describe(rectangle);                     // rectangle: 6
    describe(square);                        // square: 16

    Shape* shape = square;                   // a reference variable of an interface type
    println(shape.area());                   // 16
}
```

- A struct can implement several interfaces: `struct Circle : Shape, Printable`.
- Interfaces have no fields, and there is no inheritance between structs.
- Interface types are always references: parameters, reference variables and returned references. Variables, fields and arrays that own an interface value aren't supported yet.

---

[← Arrays](arrays.md) · [Contents](README.md) · [Enums →](enums.md)
