javac edu/cs300/*java
javac CtCILibrary/*.java
javac -h . edu/cs300/MessageJNI.java
export JAVA_HOME=/usr/java/latest
gcc -c -fPIC -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux system5_msg.c -o edu_cs300_MessageJNI.o
gcc -shared -o libsystem5msg.so edu_cs300_MessageJNI.o -lc
gcc -std=c99 -D_GNU_SOURCE searchmanager.c -o searchmanager
./searchmanager
java -cp . -Djava.library.path=. edu.cs300.MessageJNI
java edu.cs300.PassageProcessor
