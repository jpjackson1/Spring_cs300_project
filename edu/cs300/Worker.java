package edu.cs300;
import CtCILibrary.*;
import java.util.concurrent.*;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Scanner;

class Worker extends Thread {

  Trie textTrieTree;
  ArrayBlockingQueue<Object> prefixRequestArray;
  ArrayBlockingQueue<Object> resultsOutputArray;
  int id;
  String passageName;

  public Worker(String file, int id, ArrayBlockingQueue prefix, ArrayBlockingQueue results) {

    ArrayList<String> wordList = new ArrayList<>();

    File f = new File(file);

    Scanner reader;
    try {
      reader = new Scanner(f);

      while (reader.hasNext()) {
        String word = reader.next();
        word = word.toLowerCase();
        if (word.matches("[a-z]+")) {
          wordList.add(word);
        }
        else if ((word.substring(0, word.length()-1)).matches("[a-z]+")) {
          wordList.add(word.substring(0, word.length() - 1));
        }
      }
      reader.close();

    } catch (FileNotFoundException e) {}

    String[] words = new String[wordList.size()];
    words = wordList.toArray(words);

    this.textTrieTree=new Trie(words);
    this.prefixRequestArray=prefix;
    this.resultsOutputArray=results;
    this.id=id;
    this.passageName=file;//put name of passage here
  }

  public void run() {
    System.out.println("Worker-"+this.id+" ("+this.passageName+") thread started ...");
    //while (true){
      try {
        String prefix=(String)this.prefixRequestArray.take();
        //boolean found = this.textTrieTree.contains(prefix);
        String longest = this.textTrieTree.findLongestWord(prefix);
        
        if (longest == "") {
          //System.out.println("Worker-"+this.id+" "+req.requestID+":"+ prefix+" ==> not found ");
          resultsOutputArray.put(passageName+":"+prefix+" not found");
        } else{
          //System.out.println("Worker-"+this.id+" "+req.requestID+":"+ prefix+" ==> "+word);
          resultsOutputArray.put("Worker-"+this.id+" "+);
        }
      } catch(InterruptedException e){
        System.out.println(e.getMessage());
      }
    //}
  }

}
