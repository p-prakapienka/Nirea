# Arrays

Arrays work as in Java. The type `T[]` has no size: the length is fixed when the array is created, with `T[n]` or from a list of values. Elements start at their zero value, `a.length` gives the length, and an index outside the array is a runtime error. Arrays are objects, like structs and strings: assignment transfers them and `.copy()` copies them.

```nirea
struct Team {
    String[] players;

    Team(int size) {
        players = String[size];              // array fields start empty, so create them
    }
}

int total(int[]* values) {                   // a reference: the array isn't copied
    int sum = 0;
    for (int i = 0; i < values.length; i++) {
        sum += values[i];
    }
    return sum;
}

void main() {
    int[] scores = int[5];                   // 5 elements, all 0
    scores[0] = 10;
    scores[4] = 7;
    println(scores.length);                  // 5
    println(total(scores));                  // 17

    String[] names = {"Ann", "Bob"};         // created from values
    println(names[1]);                       // Bob

    int size = names.length * 2;
    float[] samples = float[size];           // the length can be computed
    println(samples.length);                 // 4

    int[][] grid = int[2][3];                // 2 rows of 3 ints
    grid[1][2] = 9;
    grid[0] = int[1];                        // rows can have different lengths
    println(grid[0].length);                 // 1

    Team team = Team(3);
    team.players[0] = "Ann";
    println(team.players.length);            // 3

    int[] backup = scores.copy();            // explicit copy: both stay usable
    int[] moved = scores;                    // assignment transfers ownership
    println(backup[0] + moved[4]);           // 17
}
```

## Rules

- `T[n]` creates `n` elements, each at its zero value, even for a struct with a constructor. `n` can be any integer expression. A negative `n` is a runtime error.
- `{...}` creates an array from values. It can only initialise a variable declaration.
- `T[a][b]` creates `a` rows of `b` elements. The rows are separate arrays, so they can be replaced.
- An array field starts as an empty array, with length `0`.
- A struct can hold an array of itself: `struct Node { Node[] children; }`.
- A reference to an array is written `int[]* values` ([References](references.md)).
- Arrays can't grow. Growable lists are still an open question.

---

[← Strings](strings.md) · [Contents](README.md) · [Structs and interfaces →](structs.md)
