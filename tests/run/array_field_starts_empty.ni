// An array field starts as an empty array, its zero value, until an array is
// assigned to it (§6.6, §6.8).

struct Playlist {
    String[] songs;
}

void main() {
    Playlist playlist = Playlist();
    println(playlist.songs.length);  // expect: 0
    String[] songs = {"one", "two"};
    playlist.songs = songs;
    println(playlist.songs.length);  // expect: 2
    println(playlist.songs[1]);      // expect: two
}
