namespace Text {
    String levelName(int level) {
        return switch (level) {
            case 0 -> "silent";
            case 3 -> "loud";
            default -> "normal";
        };
    }
}
