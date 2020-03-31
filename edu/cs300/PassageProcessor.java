package edu.cs300;
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

            Worker[] wArray = new Worker[passageCount];
            for (int i = 0; i < passageCount; i++) {
                workers[i] = new ArrayBlockingQueue<>(10);
                wArray[i] = new Worker(passages.get(i), i, workers[i], resultsOutputArray);
                wArray[i].start();
            }

            SearchRequest search;
            while(true) {
                search = MessageJNI.readPrefixRequestMsg();
                String prefix = search.prefix;
                int prefixId = search.requestID;

                System.out.println("**prefix(" + prefixId + ") " + prefix + " recieved");

                try {
                    for (int i = 0; i < passageCount; i++) workers[i].put(search);
                } catch (Exception e) {}

                if (prefixId == 0) break;

                int passageIndex = 0;
                while (passageIndex < passageCount) {
                    try {
                        LongestWord results = (LongestWord)resultsOutputArray.take();
                        int passageId = results.passageid;
                        String longestWord = results.word;
                        String passageName = passages.get(passageId);

                        if (results.word == "") {
                            MessageJNI.writeLongestWordResponseMsg(prefixId, prefix, passageId, passageName, "----", passageCount, 0);
                        }
                        else {
                            MessageJNI.writeLongestWordResponseMsg(prefixId, prefix, passageId, passageName, longestWord, passageCount, 1);
                        }

                        passageIndex++;
                    } catch (InterruptedException e) {};
                }
            }

            for (int i = 0; i < passageCount; i++) wArray[i].join();

            System.out.println("Terminating ...");
            

        } catch(Exception e) {}
    }
}