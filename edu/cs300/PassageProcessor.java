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
                int id = search.requestID;

                if (id == 0) break;

                try {
                    for (int i = 0; i < passageCount; i++) workers[i].put(prefix);
                } catch (Exception e) {}

                int passageIndex = 0;
                while (passageIndex < passageCount) {
                    try {
                        String results = (String)resultsOutputArray.take();
                        System.out.println("results:"+results);
                        MessageJNI.writeLongestWordResponseMsg(id, prefix, passageIndex, passageName, longestWord, passageCount, present);
                        passageIndex++;
                    } catch (InterruptedException e) {};
                }
            }

        } catch(FileNotFoundException e) {} 
        catch(IOException e) {}
    }
}