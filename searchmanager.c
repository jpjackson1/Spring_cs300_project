#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include "longest_word_search.h"
#include "queue_ids.h"

size_t                  /* O - Length of string */
strlcpy(char       *dst,        /* O - Destination string */
        const char *src,      /* I - Source string */
        size_t      size)     /* I - Size of destination string buffer */
{
    size_t    srclen;         /* Length of source string */


    /*
     * Figure out how much room is needed...
     */

    size --;

    srclen = strlen(src);

    /*
     * Copy the appropriate amount...
     */

    if (srclen > size)
        srclen = size;

    memcpy(dst, src, srclen);
    dst[srclen] = '\0';

    return (srclen);
}

sem_t *progress;
int prefix_count; // will not change from initial value set in main
int total_response; // Will not change from initial value set in main
char **prefixes; // Will not change from initial value set in main


void sigintHandler(int signalNumber) {

    int *complete = malloc(sizeof(int));
    printf("\n");
    for (int i = 0; i < prefix_count; i++) {
        sem_getvalue(&progress[i], complete);

        if (*complete < 1)
            printf("%s - pending\n", prefixes[i]);
        else if (*complete < total_response)
            printf("%s - %d of %d\n", prefixes[i], *complete, total_response);
        else
            printf("%s - done\n", prefixes[i]);
    }
    printf("\n");

    free(complete);
}

int main(int argc, char **argv) {
    
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    prefix_buf sbuf;
    response_buf rbuf;
    size_t buf_length;
    signal(SIGINT, sigintHandler);

    // Return error if no prefixes given
    if (argc <= 1) {
        printf("Error: please provide prefix of at least two characters for search\n");
        printf("Usage: %s <delay> <prefix>\n",argv[0]);
        exit(-1);
    }

    // Only count prefixes of > 2 and < 20 in length
    prefix_count = 0;
    for (int i = 2; i < argc; i++) {
        if ((strlen(argv[i]) >= 3) && (strlen(argv[i])) <= 20) {
            prefix_count++;
        }
    }

    progress = malloc(sizeof(sem_t) * prefix_count);
    prefixes = malloc(sizeof(char*) * prefix_count);
    for( int i = 0; i < prefix_count; i++) prefixes[i] = malloc(sizeof(char) * 21);
    for (int i = 0, j = 0; j < prefix_count; i++) {
        if ((strlen(argv[i+2])) < 3 || (strlen(argv[i+2]) > 20)) continue;
        strcpy(prefixes[j], argv[i+2]);
        sem_init(&progress[j], 0, 0);
        j++;
    }

    // Send message for each prefix with a delay of the given wait time in between each message
    response_buf* rbuf_array;
    int wait_time = atoi(argv[1]);
    int message_id = 1;


    // Message Send
    key = ftok(CRIMSON_ID,QUEUE_NUMBER);
    if ((msqid = msgget(key, msgflg)) < 0) {
        int errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("(msgget)");
        fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
    }
    //else
        //fprintf(stderr, "msgget: msgget succeeded: msgqid = %d\n", msqid);

    // We'll send message type 1
    sbuf.mtype = 1;
    strlcpy(sbuf.prefix,prefixes[0],WORD_LENGTH);
    sbuf.id=message_id;
    buf_length = strlen(sbuf.prefix) + sizeof(int)+1;//struct size without long int type

    // Send a message.
    if((msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT)) < 0) {
        int errnum = errno;
        fprintf(stderr,"%d, %ld, %s, %d\n", msqid, sbuf.mtype, sbuf.prefix, (int)buf_length);
        perror("(msgsnd)");
        fprintf(stderr, "Error sending msg: %s\n", strerror( errnum ));
        exit(1);
    }
    else
        fprintf(stderr,"\nMessage(%d): \"%s\" Sent (%d bytes)\n\n", sbuf.id, sbuf.prefix,(int)buf_length);


    // Recieve first message and get passage count
    int ret;
    do {
        ret = msgrcv(msqid, &rbuf, sizeof(response_buf), 2, 0);//receive type 2 message
        int errnum = errno;
        if (ret < 0 && errno != EINTR){
            fprintf(stderr, "Value of errno: %d\n", errno);
            perror("Error printed by perror");
            fprintf(stderr, "Error receiving msg: %s\n", strerror( errnum ));
        }
    } while ((ret < 0 ) && (errno == 4));

    total_response = rbuf.count;
    sem_post(&progress[0]);

    // Create array that can fit each response buf in it
    rbuf_array = malloc(sizeof(response_buf) * total_response);

    if (rbuf.index >= 0 && rbuf.index < total_response)
        rbuf_array[rbuf.index] = rbuf;
    else
        printf("Array indexing problem.\n");

    
    for (int i = 1; i < total_response; i++) {
        // Message Recieve
        int ret;
        do {
            ret = msgrcv(msqid, &rbuf, sizeof(response_buf), 2, 0);//receive type 2 message
            int errnum = errno;
            if (ret < 0 && errno != EINTR){
                fprintf(stderr, "Value of errno: %d\n", errno);
                perror("Error printed by perror");
                fprintf(stderr, "Error receiving msg: %s\n", strerror( errnum ));
            }
        } while ((ret < 0 ) && (errno == 4));

        rbuf_array[rbuf.index] = rbuf;
        sem_post(&progress[0]);
    }

    printf("Report \"%s\"\n", sbuf.prefix);
    for (int i = 0; i < total_response; i++) {
        if (rbuf_array[i].present == 1)
            fprintf(stderr,"%ld, %d of %d, %s, size=%d\n", rbuf_array[i].mtype, rbuf_array[i].index+1,rbuf_array[i].count,rbuf_array[i].longest_word, ret);
        else
            fprintf(stderr,"%ld, %d of %d, not found, size=%d\n", rbuf_array[i].mtype, rbuf_array[i].index+1,rbuf_array[i].count, ret);
    }

    message_id++;
    sleep(wait_time);



    for (int i = 1; i < prefix_count; i++) {
        // Message Send
        key = ftok(CRIMSON_ID,QUEUE_NUMBER);
        if ((msqid = msgget(key, msgflg)) < 0) {
            int errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            perror("(msgget)");
            fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
        }
        //else
            //fprintf(stderr, "msgget: msgget succeeded: msgqid = %d\n", msqid);

        // We'll send message type 1
        sbuf.mtype = 1;
        strlcpy(sbuf.prefix,prefixes[i],WORD_LENGTH);
        sbuf.id=message_id;
        buf_length = strlen(sbuf.prefix) + sizeof(int)+1;//struct size without long int type

        // Send a message.
        if((msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT)) < 0) {
            int errnum = errno;
            fprintf(stderr,"%d, %ld, %s, %d\n", msqid, sbuf.mtype, sbuf.prefix, (int)buf_length);
            perror("(msgsnd)");
            fprintf(stderr, "Error sending msg: %s\n", strerror( errnum ));
            exit(1);
        }
        else
            fprintf(stderr,"\nMessage(%d): \"%s\" Sent (%d bytes)\n\n", sbuf.id, sbuf.prefix,(int)buf_length);

        // Get return messages
        for (int j = 0; j < total_response; j++) {
            // Message Recieve
            int ret;
            do {
                ret = msgrcv(msqid, &rbuf, sizeof(response_buf), 2, 0);//receive type 2 message
                int errnum = errno;
                if (ret < 0 && errno != EINTR){
                    fprintf(stderr, "Value of errno: %d\n", errno);
                    perror("Error printed by perror");
                    fprintf(stderr, "Error receiving msg: %s\n", strerror( errnum ));
                }
            } while ((ret < 0 ) && (errno == 4));

            rbuf_array[rbuf.index] = rbuf;
            sem_post(&progress[i]);
        }

        printf("Report \"%s\"\n", sbuf.prefix);
        for (int j = 0; j < total_response; j++) {
            if (rbuf_array[j].present == 1)
                fprintf(stderr,"%ld, %d of %d, %s, size=%d\n", rbuf_array[j].mtype, rbuf_array[j].index+1,rbuf_array[j].count,rbuf_array[j].longest_word, ret);
            else
                fprintf(stderr,"%ld, %d of %d, not found, size=%d\n", rbuf_array[j].mtype, rbuf_array[j].index+1,rbuf_array[j].count, ret);
        }

        message_id++;
        sleep(wait_time);
    }

    // We'll send message type 1
    sbuf.mtype = 1;
    strlcpy(sbuf.prefix, "   ", WORD_LENGTH);
    sbuf.id=0;
    buf_length = strlen(sbuf.prefix) + sizeof(int)+1;//struct size without long int type

    // Send a message.
    if((msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT)) < 0) {
        int errnum = errno;
        fprintf(stderr,"%d, %ld, %s, %d\n", msqid, sbuf.mtype, sbuf.prefix, (int)buf_length);
        perror("(msgsnd)");
        fprintf(stderr, "Error sending msg: %s\n", strerror( errnum ));
        exit(1);
    }
    else
        fprintf(stderr,"\nMessage(%d): \"%s\" Sent (%d bytes)\n\n", sbuf.id, sbuf.prefix,(int)buf_length);

    printf("Exiting ...\n");

    free(rbuf_array);

    return 0;
}