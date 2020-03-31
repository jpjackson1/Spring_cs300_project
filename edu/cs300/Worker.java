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

    // Parse passage file into individual words
    ArrayList<String> wordList = new ArrayList<>();
    File f = new File(file);
    Scanner reader;
    try {
      reader = new Scanner(f);

      while (reader.hasNext()) {
        String token = reader.next();
        token = token.toLowerCase();

        if (token.contains("-") || token.contains("\'")) continue;

        for (String word : token.split("\\P{Alpha}+")) {
          wordList.add(word);
        }

      }
      reader.close();

    } catch (FileNotFoundException e) {}

    // Convert wordList to array of strings
    String[] words = new String[wordList.size()];
    words = wordList.toArray(words);

    // Create Trie with the words and initialize variables
    this.textTrieTree=new Trie(words);
    this.prefixRequestArray=prefix;
    this.resultsOutputArray=results;
    this.id=id;
    this.passageName=file;//put name of passage here
  }

  public void run() {
    
    System.out.println("Worker-"+this.id+" ("+this.passageName+") thread started ...");
    while(true) {
      try {
        // Take prefix from the array blocking queue and initialize values of search
        SearchRequest search = (SearchRequest)this.prefixRequestArray.take();
        String prefix = search.prefix;
        int requestId = search.requestID;

        if (requestId == 0) break;

        // Find longest word in trie starting with given prefix
        String lword = this.textTrieTree.findLongestWord(prefix);
        LongestWord longest = new LongestWord(this.id, lword);
        
        // Print results
        if (lword == "") {
          System.out.println("Worker-"+this.id+" "+requestId+":"+ prefix+" ==> not found ");
        } else{
          System.out.println("Worker-"+this.id+" "+requestId+":"+ prefix+" ==> "+longest.word);
        }

        // Send message back to queue
        resultsOutputArray.put(longest);


      } catch(InterruptedException e){
        //System.out.println(e.getMessage());
      }
    }
  }

}
