//
// Created by iclab on 1/11/21.
//

#include <iostream>
#include <sstream>
#include <chrono>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_set>
#include "tracer.h"

#include <cassert>
#include <cstring>

#include "mica/processor/partitions.h"
#include "mica/processor/request_accessor.h"
#include "mica/util/hash.h"
#include "mica/util/zipf.h"
#include "mica/util/tsc.h"


int main(int argc, char **argv) {
    auto config = ::mica::util::Config::load_file("conf.json");
    return 0;
}

//#define DEFAULT_THREAD_NUM (8)
//#define DEFAULT_KEYS_COUNT (1 << 20)
//#define DEFAULT_KEYS_RANGE (1 << 20)
//
//#define ENABLE_INITIALIZATION 0
//
//#define DEFAULT_KEY_LENGTH (1 << 6)
//using namespace ycsb;
//
//char *host_ip = "127.0.0.1";
//
//memcached_server_st *store;
//
//memcached_st **memc;
//
//std::vector<YCSB_request *> loads;
//
//std::vector<YCSB_request *> runs;
//
//long total_time;
//
//uint64_t exists = 0;
//
//uint64_t read_success = 0, modify_success = 0;
//
//uint64_t read_failure = 0, modify_failure = 0;
//
//uint64_t total_count = DEFAULT_KEYS_COUNT;
//
//uint64_t timer_range = default_timer_range;
//
//int thread_number = DEFAULT_THREAD_NUM;
//
//int key_range = DEFAULT_KEYS_RANGE;
//
//double skew = 0.0;
//
//stringstream *output;
//
//atomic<int> stopMeasure(0);
//
//int updatePercentage = 10;
//
//int erasePercentage = 0;
//
//int totalPercentage = 100;
//
//int readPercentage = (totalPercentage - updatePercentage - erasePercentage);
//
//struct target {
//    int tid;
//    memcached_server_st *store;
//};
//
//pthread_t *workers;
//
//struct target *parms;
//
//void simpleInsert() {
//    memc[0] = memcached_create(NULL);
//    memcached_server_push(memc[0], store);
//    memcached_behavior_set(memc[0], MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 0);
//    Tracer tracer;
//    tracer.startTime();
//    int inserted = 0, success = 0;
//    for (int i = 0; i < key_range; i++, inserted++) {
//        memcached_return_t ret = memcached_set(memc[0], loads[i]->getKey(), loads[i]->keyLength(), loads[i]->getVal(),
//                                               loads[i]->valLength(), 0, 0);
//        if (ret == memcached_return_t::MEMCACHED_SUCCESS) success++;
//    }
//    cout << inserted << " " << success << " " << tracer.getRunTime() << endl;
//    memcached_free(memc[0]);
//}
//
//void *insertWorker(void *args) {
//    struct target *work = (struct target *) args;
//    uint64_t inserted = 0;
//    for (int i = work->tid * key_range / thread_number; i < (work->tid + 1) * key_range / thread_number; i++) {
//        memcached_return_t ret = memcached_set(memc[0], loads[i]->getKey(), loads[i]->keyLength(), loads[i]->getVal(),
//                                               loads[i]->valLength(), 0, 0);
//        inserted++;
//    }
//    __sync_fetch_and_add(&exists, inserted);
//}
//
//void *measureWorker(void *args) {
//    Tracer tracer;
//    tracer.startTime();
//    struct target *work = (struct target *) args;
//    uint64_t mhit = 0, rhit = 0;
//    uint64_t mfail = 0, rfail = 0;
//    std::string dummyVal;
//
//    try {
//        while (stopMeasure.load(memory_order_relaxed) == 0) {
//            for (int i = work->tid * total_count / thread_number;
//                 i < (work->tid + 1) * total_count / thread_number; i++) {
//                switch (static_cast<int>(runs[i]->getOp())) {
//                    case 0: {
//                        size_t value_length;
//                        uint32_t flag;
//                        memcached_return_t ret;
//                        char *value = memcached_get(memc[work->tid], runs[i]->getKey(), runs[i]->keyLength(),
//                                                    &value_length, &flag, &ret);
//                        if (ret == memcached_return_t::MEMCACHED_SUCCESS) rhit++;
//                        else rfail++;
//                        break;
//                    }
//                    case 1:
//                    case 3: {
//                        memcached_return_t ret = memcached_set(memc[work->tid], runs[i]->getKey(), runs[i]->keyLength(),
//                                                               runs[i]->getVal(), runs[i]->valLength(), 0, 0);
//                        if (ret == memcached_return_t::MEMCACHED_SUCCESS) mhit++;
//                        else mfail++;
//                        break;
//                    }
//                    case 2: {
//                        memcached_return_t ret = memcached_delete(memc[work->tid], runs[i]->getKey(),
//                                                                  runs[i]->keyLength(), 0);
//                        if (ret == memcached_return_t::MEMCACHED_SUCCESS) mhit++;
//                        else mfail++;
//                        break;
//                    }
//                    default:
//                        break;
//                }
//            }
//        }
//    } catch (exception e) {
//        cout << work->tid << endl;
//    }
//
//    long elipsed = tracer.getRunTime();
//    output[work->tid] << work->tid << " " << elipsed << " " << mhit << " " << rhit << endl;
//    __sync_fetch_and_add(&total_time, elipsed);
//    __sync_fetch_and_add(&read_success, rhit);
//    __sync_fetch_and_add(&read_failure, rfail);
//    __sync_fetch_and_add(&modify_success, mhit);
//    __sync_fetch_and_add(&modify_failure, mfail);
//}
//
//void prepare() {
//    cout << "prepare" << endl;
//    workers = new pthread_t[thread_number];
//    parms = new struct target[thread_number];
//    output = new stringstream[thread_number];
//    for (int i = 0; i < thread_number; i++) {
//        parms[i].tid = i;
//        parms[i].store = store;
//    }
//}
//
//void finish() {
//    cout << "finish" << endl;
//    delete[] parms;
//    delete[] workers;
//    delete[] output;
//}
//
//void multiWorkers() {
//    output = new stringstream[thread_number];
//    Tracer tracer;
//    tracer.startTime();
//    cout << "Insert " << exists << " " << tracer.getRunTime() << endl;
//    Timer timer;
//    timer.start();
//    for (int i = 0; i < thread_number; i++) {
//        memc[i] = memcached_create(NULL);
//        memcached_server_push(memc[i], store);
//        memcached_behavior_set(memc[i], MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1);
//        pthread_create(&workers[i], nullptr, measureWorker, &parms[i]);
//    }
//    while (timer.elapsedSeconds() < timer_range) {
//        sleep(1);
//    }
//    stopMeasure.store(1, memory_order_relaxed);
//    for (int i = 0; i < thread_number; i++) {
//        pthread_join(workers[i], nullptr);
//        string outstr = output[i].str();
//        cout << outstr;
//        memcached_free(memc[i]);
//    }
//    cout << "Gathering ..." << endl;
//}
//
//int main(int argc, char **argv) {
//    if (argc > 7) {
//        thread_number = std::atol(argv[1]);
//        key_range = std::atol(argv[2]);
//        total_count = std::atol(argv[3]);
//        timer_range = std::atol(argv[4]);
//        skew = std::stof(argv[5]);
//        updatePercentage = std::atoi(argv[6]);
//        erasePercentage = std::atoi(argv[7]);
//        readPercentage = totalPercentage - updatePercentage - erasePercentage;
//    }
//    if (argc > 8) host_ip = argv[8];
//    char host[DEFAULT_KEY_LENGTH];
//    std::memset(host, 0, DEFAULT_KEY_LENGTH);
//    std::strcpy(host, host_ip);
//    store = memcached_servers_parse(std::strcat(host, ":11211"));
//    memc = new memcached_st *[thread_number];
//#if ENABLE_INITIALIZATION
//    YCSBLoader loader(loadpath, key_range);
//    loads = loader.load();
//    key_range = loader.size();
//    cout << "simple" << endl;
//    simpleInsert();
//#endif
//    prepare();
//    YCSBLoader runner(runpath, total_count);
//    runs = runner.load();
//    total_count = runner.size();
//    cout << " threads: " << thread_number << " range: " << key_range << " count: " << total_count << " timer: "
//         << timer_range << " skew: " << skew << " u:e:r = " << updatePercentage << ":" << erasePercentage << ":"
//         << readPercentage << endl;
//    cout << "multiinsert" << endl;
//    multiWorkers();
//    cout << "read operations: " << read_success << " read failure: " << read_failure << " modify operations: "
//         << modify_success << " modify failure: " << modify_failure << " throughput: "
//         << (double) (read_success + read_failure + modify_success + modify_failure) * thread_number / total_time
//         << endl;
//    loads.clear();
//    runs.clear();
//    finish();
//    delete[] memc;
//    delete store;
//    return 0;
//}