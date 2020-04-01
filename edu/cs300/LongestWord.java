package edu.cs300;

public class LongestWord {

    int passageid;
    String word;

    public LongestWord(int id, String lword) {
        this.passageid = id;
        this.word = lword;
    }

    int getId() {
        return passageid;
    }

    String getWord() {
        return word;
    }

}