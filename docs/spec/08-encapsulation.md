# 8. Struct encapsulation

Nirea uses explicit getters and setters.

There are no implicit properties, generated accessors or property syntactic sugar.

```nirea
struct Engine {
    private int power;

    int getPower() {
        return power;
    }

    void setPower(int power) {
        this.power = power;
    }
}
```

Member visibility is public by default, with `private` available for encapsulation.

`private` applies to fields and methods. A private member can be used only inside the methods and constructor of its struct, on any value of that struct type.

Operations remain explicit and visible in source code.

---

[← 7. Ownership and memory safety](07-ownership.md) · [Contents](README.md) · [9. References →](09-references.md)
