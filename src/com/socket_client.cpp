//
// Created by iclab on 1/26/21.
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <pthread.h>
//#include "client_tool.h"
#include "settings.h"
#include "tracer.h"

struct timeval starttime;
unsigned long runtime;

unsigned long getRunTime(struct timeval begTime) {
    struct timeval endTime;
    gettimeofday(&endTime, NULL);
    unsigned long duration = (endTime.tv_sec - begTime.tv_sec) * 1000000 + endTime.tv_usec - begTime.tv_usec;
    return duration;
}

void send_data(int threadid, socklen_t fd) {
    Tracer tracer;
    char send_buf[batch_size];
    char read_buf[batch_size];
    memset(send_buf, 0, sizeof(send_buf));
    for (int i = 0; i < batch_size / sizeof(int); i++) {
        *((int *) &send_buf[i * sizeof(int)]) = i;
    }
    uint64_t total_sent = 0, total_count = 0, total_read = 0;
    tracer.startTime();
    for (int i = 0; i < total_round; i++) {
        *((int *) send_buf) = i;
        total_sent += write(fd, send_buf, batch_size);
        total_count++;
        if (using_dummy > 0) {
            total_read += read(fd, read_buf, using_dummy);
        }
    }
    printf("thread %d \tsend %lld, \t%lld, \t%lld, \t%lld\n", threadid, total_count, total_sent, total_read,
           tracer.getRunTime());
}

void *thread_func(void *threadid) {
    long tid;
    tid = (long) threadid;
    //printf("AF_INET\n");
    //create unix socket
    socklen_t connect_fd;
    connect_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_fd < 0) {
        perror("cannot create communication socket");
        return NULL;
    }

    struct sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(server_port_base + tid);
    // srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_addr.s_addr = inet_addr(ip_addr);
    //connect server
    socklen_t ret;
    ret = connect(connect_fd, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
    if (ret == -1) {
        perror("cannot connect to the server");
        close(connect_fd);
        return NULL;
    }

    send_data(tid, connect_fd);
}

int main(int argc, char **argv) {
    sender_num = NUM_THREADS;
    if (argc == 6) {
        ip_addr = argv[1];
        batch_size = std::atoi(argv[2]);
        total_round = std::atoi(argv[3]);
        sender_num = std::atoi(argv[4]);
        using_dummy = std::atoi(argv[5]);
    }
    printf("Address: %s, bs: %d, round: %d, sender: %d, dummy: %d\n", ip_addr, batch_size, total_round, sender_num,
           using_dummy);
    pthread_t threads[sender_num];
    int rc;
    int i;
    for (i = 0; i < sender_num; i++) {
        printf("main() : creating thread %d\n ", i);
        rc = pthread_create(&threads[i], NULL, thread_func, (void *) i);
        if (rc) {
            printf("Error:unable to create thread, %d\n", rc);
            return 1;
        }
    }
    pthread_exit(NULL);
}
