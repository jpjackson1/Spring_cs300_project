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

            // Create worker and input arrayblockingQueue's and store them in arrays
            /* 
            *  Instead of parsing each file sequentially in PassageProcessor,
            *  I sent each thread the file name it is associated with and
            *  the worker threads parsed the respective file to maximize concurrency
            */
            Worker[] wArray = new Worker[passageCount];
            for (int i = 0; i < passageCount; i++) {
                workers[i] = new ArrayBlockingQueue<>(10); // Prefix input queue
                wArray[i] = new Worker(passages.get(i), i, workers[i], resultsOutputArray); // Worker array
                wArray[i].start();
            }

            // Get prefix requests, send requests to threads, send results back to system V queue
            SearchRequest search;
            while(true) {
                search = MessageJNI.readPrefixRequestMsg(); // get prefix request
                String prefix = search.getPrefix();
                int prefixId = search.getId();

                System.out.println("**prefix(" + prefixId + ") " + prefix + " recieved");

                // Send prefix to each thread
                try {
                    for (int i = 0; i < passageCount; i++) workers[i].put(search);
                } catch (Exception e) {}

                // End passage processor if 0 id is found
                if (prefixId == 0) break;

                // Get each return value from threads output queue and send to system V queue
                int passageIndex = 0;
                while (passageIndex < passageCount) {
                    try {
                        LongestWord results = (LongestWord)resultsOutputArray.take();
                        int passageId = results.getId();
                        String longestWord = results.getWord();
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

            // Join threads
            for (int i = 0; i < passageCount; i++) wArray[i].join();

            System.out.println("Terminating ...");
            

        } catch(Exception e) {}
    }
}