// Importing the same file again, under the same or another path, has no
// further effect (§12.3).
import Ping.ni;
import Ping.ni;
import ./Ping.ni;

void main() {
    println(ping(5));
}
