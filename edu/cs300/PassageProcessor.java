package edu.cs300;
import CtCILibrary.*;
import java.util.concurrent.*;
import java.util.ArrayList;
import java.io.*;
import java.util.Scanner;
import java.util.*;

public class PassageProcessor {
    public static void main(String args[]) throws IOException {
            
        try {

            // Create an array for the names of the passage files to send to the workers
            ArrayList<String> passages = new ArrayList<String>();
            BufferedReader reader = new BufferedReader(new FileReader("passages.txt"));
            int passageCount = 0;
            String cur;
            while ((cur = reader.readLine()) != null) {
                passages.add(cur);
                passageCount++;
            }
            reader.close();

            // Allocate space for workers and queue
            ArrayBlockingQueue[] workers = new ArrayBlockingQueue[passageCount];
            ArrayBlockingQueue resultsOutputArray=new ArrayBlockingQueue(passageCount*10);

            for (int i = 0; i < passageCount; i++) {
                workers[i] = new ArrayBlockingQueue<>(10);
                new Worker(passages.get(i), i, workers[i], resultsOutputArray).start();
            }

            while(true) {
                SearchRequest search = MessageJNI.readPrefixRequestMsg();
                String prefix = search.prefix;
                int prefixId = search.requestID;

                if (prefixId == 0) break;

                try {
                    for (int i = 0; i < passageCount; i++) workers[i].put(search);
                } catch (Exception e) {}

                int passageIndex = 0;
                while (passageIndex < passageCount) {
                    try {
                        LongestWord results = (LongestWord)resultsOutputArray.take();
                        int passageId = results.passageid;
                        String longestWord = results.word;
                        String passageName = passages.get(passageId);

                        if (results.word == "") {
                            MessageJNI.writeLongestWordResponseMsg(prefixId, prefix, passageId+1, passageName, longestWord, passageCount, 0);
                        }
                        else {
                            MessageJNI.writeLongestWordResponseMsg(prefixId, prefix, passageId+1, passageName, longestWord, passageCount, 1);
                        }

                        passageIndex++;
                    } catch (InterruptedException e) {};
                }
            }

            System.out.println("Done");

        } catch(FileNotFoundException e) {} 
        catch(IOException e) {}
    }
}