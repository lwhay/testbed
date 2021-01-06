//
// Created by iclab on 1/6/21.
//

#include <iostream>
#include <cstring>
#include <pthread.h>
#include <libmemcached/memcached.h>
#include "tracer.h"

#define DEFAULT_KEY_LENGTH (1 << 6)

uint64_t total_count = (1 << 24);

int thread_number = 2;

char *host_ip = "localhost";

memcached_server_st *servers;
memcached_st **memc;

size_t *runtimes, *msucc, *mfail;
std::atomic<size_t> modification_success{0};
std::atomic<size_t> modification_failure{0};

void *measureWorker(void *args) {
    int tid = *(int *) args;
    char key[DEFAULT_KEY_LENGTH];
    char value[DEFAULT_KEY_LENGTH];
    Tracer tracer;
    tracer.startTime();
    //std::cout << "\tTid" << tid << ": " << tracer.getRunTime() << std::endl;
    for (int i = tid; i < total_count; i += thread_number) {
        std::memset(key, 0, DEFAULT_KEY_LENGTH);
        std::memset(value, 0, DEFAULT_KEY_LENGTH);
        std::sprintf(key, "%d", i);
        std::memcpy(value, "hello", 5);
        std::sprintf(value + 5, "%d", i);
        memcached_return_t ret = memcached_set(memc[tid], key, std::strlen(key), value, std::strlen(value), 0, 0);
        //if (tid == 0 && (i % 100000 == 0)) cout << "\t" << tid << " " << i << " " << ret << endl;
        if (ret == memcached_return_t::MEMCACHED_SUCCESS) msucc[tid]++;
        else mfail[tid]++;
    }
    modification_success.fetch_add(msucc[tid]);
    modification_failure.fetch_add(mfail[tid]);
    runtimes[tid] += tracer.getRunTime();
}

int main(int argc, char **argv) {
    if (argc > 3) {
        thread_number = std::atoi(argv[1]);
        total_count = std::atol(argv[2]);
        host_ip = argv[3];
    }
    cout << "thread: " << thread_number << " " << total_count << " " << host_ip << endl;
    servers = memcached_servers_parse(host_ip);
    runtimes = new size_t[thread_number];
    msucc = new size_t[thread_number];
    mfail = new size_t[thread_number];
    //memcached_server_list_free(servers);
    memc = new memcached_st *[thread_number];
    Tracer tracer;
    tracer.startTime();
    pthread_t threads[thread_number];
    int tids[thread_number];
    for (int i = 0; i < thread_number; i++) {
        memc[i] = memcached_create(NULL);
        memcached_server_push(memc[i], servers);
        memcached_behavior_set(memc[i], MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, i);
        tids[i] = i;
        msucc[i] = 0;
        mfail[i] = 0;
        pthread_create(&threads[i], nullptr, measureWorker, tids + i);
    }
    for (int i = 0; i < thread_number; i++) {
        pthread_join(threads[i], nullptr);
        memcached_free(memc[i]);
    }

    size_t total_runtime = 0;
    for (int i = 0; i < thread_number; i++) {
        cout << "\t" << i << " " << runtimes[i] << " " << msucc[i] << " " << mfail[i] << endl;
        total_runtime += runtimes[i];
    }
    cout << total_count << "<->" << tracer.getRunTime() << " " << modification_success << " " << modification_failure
         << " " << ((double) modification_success + modification_failure) * thread_number / total_runtime << endl;
    delete[] memc;
    delete[] runtimes;
    delete[] msucc;
    delete[] mfail;
    return 0;
}