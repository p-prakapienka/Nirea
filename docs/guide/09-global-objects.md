# 9. Global objects

A global object is a struct value that belongs to the whole program, like a Kotlin `object` or a singleton service. It is created before `main` runs and destroyed after `main` returns. It is declared at the top level of a file or in a namespace, and its name is written in UPPER_SNAKE_CASE.

```nirea
struct User {
    String name;

    User(String name) {
        this.name = name;
    }
}

struct UserStore {
    private User[] users;
    private int count;

    UserStore() {
        users = User[4];
    }

    int save(User user) {
        users[count] = user;
        count++;
        return count - 1;
    }

    User find(int id) {
        return users[id].copy();
    }

    int size() {
        return count;
    }
}

UserStore USERS = UserStore();

void register(String name) {
    USERS.save(User(name));                  // any function can call its methods
}

void main() {
    register("Ann");
    register("Bob");
    User bob = USERS.find(1);                // a copy
    println(bob.name);                       // Bob
    println(USERS.size());                   // 2
}
```

A global object is used only by calling its methods. It can't be assigned, copied, compared or passed as an argument, and its fields can't be used through its name:

```nirea
UserStore store = USERS;                 // error: global object 'USERS' can only be used to call its methods
println(USERS.count);                    // error: the fields of global object 'USERS' can't be used through its name
```

Its methods that return a reference can't be called on it either, so nothing outside can hold a reference into it. A struct type with a global object can still have ordinary local values.

Global objects are created in the order they need each other: before a global object is created, every global object its initialiser can reach exists. A global object that needs itself, directly or through others, is a compile error. They are destroyed in the reverse order.

## Choosing an implementation

To switch implementations, for example in tests, a global object can forward each call to another one. It implements the same interface:

```nirea
interface Greeter {
    String greet(String name);
}

struct FormalGreeter : Greeter {
    String greet(String name) {
        return "Good day, " + name;
    }
}

struct CasualGreeter : Greeter {
    String greet(String name) {
        return "Hi " + name;
    }
}

struct Settings {
    private boolean casual;

    void setCasual(boolean casual) {
        this.casual = casual;
    }

    boolean isCasual() {
        return casual;
    }
}

struct Greeters : Greeter {
    String greet(String name) {
        if (SETTINGS.isCasual()) {
            return CASUAL.greet(name);
        }
        return FORMAL.greet(name);
    }
}

FormalGreeter FORMAL = FormalGreeter();
CasualGreeter CASUAL = CasualGreeter();
Settings SETTINGS = Settings();
Greeters GREETER = Greeters();

void main() {
    println(GREETER.greet("Ann"));           // Good day, Ann
    SETTINGS.setCasual(true);
    println(GREETER.greet("Ann"));           // Hi Ann
}
```

## References into a global object

Any function can reach a global object by its name. So inside the global object's own methods, a call to another function could come back to the same object and change it, while the method still holds a reference into it. The compiler prevents this: in the methods of a struct type that has a global object, a call that can reach that global object ends every reference into `this`.

```nirea
struct Log {
    void write(String text) {
        println(text + " (" + String.of(USERS.size()) + " users)");
    }
}

struct UserStore {
    private User[] users;
    private int count;

    void rename(int id, String name) {
        User* user = users[id];
        LOG.write("renaming");               // Log.write can reach USERS, so user ends here
        user.name = name;                    // error: 'user' can't be used here
    }

    int size() {
        return count;
    }
}

UserStore USERS = UserStore();
Log LOG = Log();
```

The error message shows the chain of calls that reaches the global object:

```text
Main.ni:14:9: error: 'user' can't be used here: the reference ended when its object was reached another way
Main.ni:13:18: note: 'user' ended here, because this call can reach global object 'USERS': 'Log.write' uses 'USERS'
```

To fix it, make the call before taking the reference, or use `users[id]` directly after the call. For the same reason, such a call can't receive a reference into `this` as an argument; pass a copy instead. Calls that can't reach the global object aren't affected.

Using global objects from several threads is still an open question.

---

[← 8. References](08-references.md) · [Contents](README.md) · [Projects →](10-projects.md)
