package edu.cs300;
import CtCILibrary.*;
import java.util.concurrent.*;
import java.util.ArrayList;
import java.io.*;
import java.util.Scanner;
import java.util.*;

public class PassageProcessor {
    
    public static void main(String[] args) throws IOException {

        try {

            ArrayList<String> passages = new ArrayList<String>();
            BufferedReader reader = new BufferedReader(new FileReader("passages.txt"));
            int passageCount = 0;
            String cur;
            while ((cur = reader.readLine()) != null) {
                passages.add(cur);
                passageCount++;
            }
            reader.close();

            ArrayBlockingQueue[] workers = new ArrayBlockingQueue[passageCount];
            ArrayBlockingQueue resultsOutputArray=new ArrayBlockingQueue(passageCount*10);

            for (int i = 0; i < passageCount; i++) {
                workers[i] = new ArrayBlockingQueue<>(10);
                new Worker(passages.get(i), i, workers[i], resultsOutputArray).start();
            }

            try {
                for (int i = 0; i < passageCount; i++) workers[i].put(args[0]);
            } catch (Exception e) {}


            int counter=0;
            while (counter<passageCount) {
                try {
                    String results = (String)resultsOutputArray.take();
                    System.out.println("results:"+results);
                    counter++;
                } catch (InterruptedException e) {};
            }

        } catch(FileNotFoundException e) {} 
          catch(IOException e) {}
    }
}