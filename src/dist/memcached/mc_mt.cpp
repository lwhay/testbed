//
// Created by iclab on 1/3/21.
//
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tracer.h"

#define BATCH_SIZE 4096
#define SET_HEAD_SIZE 32
#define GET_HEAD_SIZE 24

using namespace ycsb;

typedef struct SendBatch {
    char *buf;
    std::size_t size;//real data size
} sendbatch;

char *target_ip = "172.168.204.75";

int THREAD_NUM = 4;

unsigned long *runtimelist;

std::vector<YCSB_request *> loads;

std::vector<sendbatch> database;

void con_database(int begin_index = 0, int end_index = loads.size());

void con_package(char *buf, YCSB_request *req);

int get_package_size(YCSB_request *req);

void send_thread(int tid);

int main(int argc, char **argv) {
    char *path;
    if (argc == 3) {
        THREAD_NUM = std::atol(argv[1]);
        runtimelist = (unsigned long *) malloc(THREAD_NUM * sizeof(unsigned long));
        path = argv[2];
    } else if (argc < 3) {
        printf("please input filename\n");
        return 0;
    }
    if (argc == 4) target_ip = argv[3];

    int key_range = 200;
    YCSBLoader loader(path);
    loads = loader.load();
    int size = loader.size();
    con_database();

    std::vector<std::thread> threads;
    int send_batch_num = database.size() / THREAD_NUM;
    for (int i = 0; i < THREAD_NUM; i++) {
        printf("creating thread %d\n", i);
        threads.push_back(std::thread(send_thread, i));
    }
    for (int i = 0; i < THREAD_NUM; i++) {
        printf("stoping %d\n", i);
        threads[i].join();
    }

    //calculate running time

    unsigned long runtime = 0;
    for (int i = 0; i < THREAD_NUM; i++) {
        runtime += runtimelist[i];
    }
    runtime /= (THREAD_NUM);
    printf("\n____\nruntime:%lu\n", runtime);

    return 0;
}

void send_thread(int tid) {
    int send_num = database.size() / THREAD_NUM;
    int start_index = tid * send_num;

    unsigned int connect_fd;
    static struct sockaddr_in srv_addr;
    //create  socket
    connect_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_fd < 0) {
        perror("cannot create communication socket");
        return;
    }

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(11211);
    srv_addr.sin_addr.s_addr = inet_addr(target_ip);//htonl(INADDR_ANY);

    //connect server;
    if (connect(connect_fd, (struct sockaddr *) &srv_addr, sizeof(srv_addr)) == -1) {
        perror("cannot connect to the server");
        close(connect_fd);
        return;
    }
//    char tmpbuf[30]="set foo 0 0 3 noreply\r\naaa\r\n";
//    for(int i=0;i<100000;i++) write(connect_fd,tmpbuf,strlen(tmpbuf));

    Tracer tracer;
    tracer.startTime();
    unsigned long writenbytes = 0;
    for (int i = 0; i < send_num; i++) {
        writenbytes += write(connect_fd, database[start_index + i].buf, \
                            database[start_index + i].size);
        //writenbytes+=write(connect_fd,tmpbuf,strlen(tmpbuf));
    }
    runtimelist[tid] += tracer.getRunTime();
    printf("thread %d write bytes:%ld\n", tid, writenbytes);
    close(connect_fd);
}

void con_database(int begin_index, int end_index) {
    int offset = 0; //package offset
    sendbatch batch;
    batch.buf = (char *) malloc(BATCH_SIZE);
    memset(batch.buf, 0, BATCH_SIZE);
    for (int i = begin_index; i < end_index; i++) {
        YCSB_request *req = loads[i];
        if (offset + get_package_size(req) > BATCH_SIZE) {
            batch.size = offset;
            database.push_back(batch);

            sendbatch newbatch;
            newbatch.buf = (char *) malloc(BATCH_SIZE);
            memset(newbatch.buf, 0, BATCH_SIZE);
            batch = newbatch;

            offset = 0;
            i--;
            continue;
        } else {
            con_package(batch.buf + offset, req);
            offset += get_package_size(req);
        }
    }
    batch.size = offset;
    database.push_back(batch);

}

int get_package_size(YCSB_request *req) {
    if (req->getOp() == lookup) {
        return 24 + req->keyLength();
    } else if (req->getOp() == insert || req->getOp() == update) {
        return 32 + req->keyLength() + req->valLength();
    } else {
        return BATCH_SIZE;
    }
}

void con_package(char *buf, YCSB_request *req) {
    if (req->getOp() == lookup) {
        //GETQ
        buf[0] = 0x80;
        buf[1] = 0x09;
        size_t ks = req->keyLength();
        *(unsigned short int *) (buf + 2) = htons((unsigned short int) ks);
        *(unsigned int *) (buf + 8) = htonl((unsigned int) ks);
        memcpy(buf + 24, req->getKey(), ks);
    } else if (req->getOp() == insert || req->getOp() == update) {
        //SET
        buf[0] = 0x80;
        buf[1] = 0x11;
        size_t ks = req->keyLength();
        size_t vs = req->valLength();
        *(unsigned short int *) (buf + 2) = htons((unsigned short int) ks);
        buf[4] = 0x08;
        *(unsigned int *) (buf + 8) = htonl((unsigned int) (ks + vs + 8));

        memcpy(buf + 32, req->getKey(), ks);
        mempcpy(buf + 32 + ks, req->getVal(), vs);
    }
}

