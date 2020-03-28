#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
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


int main(int argc, char **argv) {
    
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    prefix_buf sbuf;
    response_buf rbuf;
    size_t buf_length;

    // Return error if no prefixes given
    if (argc <= 1) {
        printf("Error: please provide prefix of at least two characters for search\n");
        printf("Usage: %s <prefix>\n",argv[0]);
        exit(-1);
    }

    // Return error if any of the prefix arguments are less than 3 characters
    for (int i = 2; i < argc; i++) {
        if (strlen(argv[i]) <= 2) {
            printf("Error: please provide prefix of at least two characters for search\n");
            printf("Usage: %s <prefix>\n",argv[0]);
            exit(-1);
        }
    }

    // Send message for each prefix with a delay of the given wait time in between each message
    response_buf* rbuf_array;
    int wait_time = atoi(argv[1]);
    int message_id = 1;
    for (int i = 2; i < argc; i++) {
        // Message Send
        key = ftok(CRIMSON_ID,QUEUE_NUMBER);
        if ((msqid = msgget(key, msgflg)) < 0) {
            int errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            perror("(msgget)");
            fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
        }
        else
            fprintf(stderr, "msgget: msgget succeeded: msgqid = %d\n", msqid);

        // We'll send message type 1
        sbuf.mtype = 1;
        strlcpy(sbuf.prefix,argv[i],WORD_LENGTH);
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
            fprintf(stderr,"Message(%d): \"%s\" Sent (%d bytes)\n", sbuf.id, sbuf.prefix,(int)buf_length);


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

        int ret_count = rbuf.count;

        // Create array that can fit each response buf in it
        rbuf_array = malloc(sizeof(response_buf) * ret_count);

        if (rbuf.index >= 0 && rbuf.index < ret_count)
            rbuf_array[rbuf.index] = rbuf;
        else
            printf("Array indexing problem.\n");

        
        for (int i = 1; i < ret_count; i++) {
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
        }

        for (int i = 0; i < ret_count; i++) {
            if (rbuf_array[i].present == 1)
                fprintf(stderr,"%ld, %d of %d, %s, size=%d\n", rbuf_array[i].mtype, rbuf_array[i].index,rbuf_array[i].count,rbuf_array[i].longest_word, ret);
            else
                fprintf(stderr,"%ld, %d of %d, not found, size=%d\n", rbuf_array[i].mtype, rbuf_array[i].index,rbuf_array[i].count, ret);
        }

        message_id++;
        sleep(wait_time);
    }

    // We'll send message type 1
    sbuf.mtype = 1;
    strlcpy(sbuf.prefix, "", WORD_LENGTH);
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
        fprintf(stderr,"Message(%d): \"%s\" Sent (%d bytes)\n", sbuf.id, sbuf.prefix,(int)buf_length);

    return 0;
}