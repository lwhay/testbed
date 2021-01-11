//
// Created by iclab on 1/11/21.
//

#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unordered_set>
#include "tracer.h"

#include <cassert>
#include <cstring>

#include "mica/processor/partitions.h"
#include "mica/processor/request_accessor.h"
#include "mica/util/hash.h"
#include "mica/util/tsc.h"

struct LTableConfig : public ::mica::table::BasicLossyLTableConfig {
    // struct LTableConfig : public ::mica::table::BasicLosslessLTableConfig {
    static constexpr bool kVerbose = false;
    static constexpr bool kCollectStats = true;
};

typedef ::mica::table::LTable<LTableConfig> Table;
typedef ::mica::table::Result Result;

template<typename T>
static uint64_t mica_hash(const T *key, size_t key_length) {
    return ::mica::util::hash(key, key_length);
}

void dummy() {
    ::mica::util::lcore.pin_thread(0);

    auto config = ::mica::util::Config::load_file("conf.json");

    LTableConfig::Alloc alloc(config.get("alloc"));
    LTableConfig::Pool pool(config.get("pool"), &alloc);
    Table table(config.get("table"), &alloc, &pool);

    uint64_t key_hash;
    uint64_t key_i;
    uint64_t value_i;
    const char *key = reinterpret_cast<const char *>(&key_i);
    char *value = reinterpret_cast<char *>(&value_i);
    size_t key_length = sizeof(key_i);
    size_t value_length = sizeof(value_i);

    Result out_result;
    size_t out_value_length;

    for (int i = 0; i < 10000; i++) {
        key_i = i;
        key_hash = mica_hash(&key_i, sizeof(key_i));
        value_i = i;
        out_result =
                table.set(key_hash, key, key_length, value, value_length, true);
        assert(out_result == Result::kSuccess);
        // table.print_buckets();
    }
    table.print_stats();

    for (int i = 0; i < 10000; i++) {
        key_i = i;
        key_hash = mica_hash(&key_i, sizeof(key_i));
        value_i = 0;
        out_value_length = 0;
        out_result = table.get(key_hash, key, key_length, value, value_length,
                               &out_value_length, false);
        if (out_result != Result::kSuccess) std::cout << "error" << i << std::endl;
        assert(out_result == Result::kSuccess);
        assert(out_value_length == value_length);
        assert(value_i == i);
    }
    // table.print_stats();

    for (int i = 0; i < 10000; i++) {
        key_i = i;
        key_hash = mica_hash(&key_i, sizeof(key_i));
        value_i = i + 1;
        out_result =
                table.set(key_hash, key, key_length, value, value_length, true);
        assert(out_result == Result::kSuccess);
        // table.print_buckets();
    }
    // table.print_stats();

    for (int i = 0; i < 10000; i++) {
        key_i = i;
        key_hash = mica_hash(&key_i, sizeof(key_i));
        value_i = 0;
        out_result = table.get(key_hash, key, key_length, value, value_length,
                               &out_value_length, false);
        assert(out_result == Result::kSuccess);
        assert(out_value_length == value_length);
        assert(value == 300);
    }
    // table.print_stats();

    for (int i = 0; i < 10000; i++) {
        key_i = i;
        key_hash = mica_hash(&key_i, sizeof(key_i));
        out_result = table.del(key_hash, key, key_length);
        assert(out_result == Result::kSuccess);
    }
    // table.print_stats();

    (void) out_result;
}

#define DEFAULT_THREAD_NUM (8)
#define DEFAULT_KEYS_COUNT (1 << 20)
#define DEFAULT_KEYS_RANGE (1 << 20)

#define ENABLE_INITIALIZATION 1

#define DEFAULT_KEY_LENGTH (1 << 6)
using namespace ycsb;

char *host_ip = "127.0.0.1";

Table *store;

std::vector<YCSB_request *> loads;

std::vector<YCSB_request *> runs;

long total_time;

uint64_t exists = 0;

uint64_t read_success = 0, modify_success = 0;

uint64_t read_failure = 0, modify_failure = 0;

uint64_t total_count = DEFAULT_KEYS_COUNT;

uint64_t timer_range = default_timer_range;

int thread_number = DEFAULT_THREAD_NUM;

int key_range = DEFAULT_KEYS_RANGE;

double skew = 0.0;

stringstream *output;

atomic<int> stopMeasure(0);

int updatePercentage = 10;

int erasePercentage = 0;

int totalPercentage = 100;

int readPercentage = (totalPercentage - updatePercentage - erasePercentage);

struct target {
    int tid;
    Table *store;
};

pthread_t *workers;

struct target *parms;

void simpleInsert() {
    Tracer tracer;
    tracer.startTime();
    int inserted = 0, success = 0;
    for (int i = 0; i < key_range; i++, inserted++) {
        uint64_t key_hash = mica_hash(loads[i]->getKey(), loads[i]->keyLength());
        Result ret = store->set(key_hash, loads[i]->getKey(), loads[i]->keyLength(), loads[i]->getVal(),
                                loads[i]->valLength(), true);
        if (ret == Result::kSuccess) success++;
    }
    cout << inserted << " " << success << " " << tracer.getRunTime() << endl;
    store->print_stats();
    std::cout << "---------------------------------------------------------------------------------------------------"
              << std::endl;
}

void *measureWorker(void *args) {
    Tracer tracer;
    tracer.startTime();
    struct target *work = (struct target *) args;
    uint64_t mhit = 0, rhit = 0;
    uint64_t mfail = 0, rfail = 0;
    std::string dummyVal;

    try {
        while (stopMeasure.load(memory_order_relaxed) == 0) {
            for (int i = work->tid * total_count / thread_number;
                 i < (work->tid + 1) * total_count / thread_number; i++) {
                uint64_t key_hash = mica_hash(runs[i]->getKey(), runs[i]->keyLength());
                switch (static_cast<int>(runs[i]->getOp())) {
                    case 0: {
                        size_t value_length;
                        char value[255];
                        char *key = runs[i]->getKey();
                        Result ret = work->store->get(key_hash, runs[i]->getKey(), runs[i]->keyLength(), value,
                                                      19, &value_length, false);
                        /*std::cout << runs[i]->valLength() << ":" << runs[i]->keyLength() << ":" << value_length
                                  << std::endl;*/
                        if (ret == Result::kSuccess) rhit++;
                        else rfail++;
                        break;
                    }
                    case 1:
                    case 3: {
                        Result ret = work->store->set(key_hash, runs[i]->getKey(), runs[i]->keyLength(),
                                                      runs[i]->getVal(), runs[i]->valLength(), true);
                        if (ret == Result::kSuccess) mhit++;
                        else mfail++;
                        break;
                    }
                    case 2: {
                        Result ret = work->store->del(key_hash, runs[i]->getKey(), runs[i]->keyLength());
                        if (ret == Result::kSuccess) mhit++;
                        else mfail++;
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    } catch (exception e) {
        cout << work->tid << endl;
    }

    long elipsed = tracer.getRunTime();
    output[work->tid] << work->tid << " " << elipsed << " " << mhit << " " << rhit << endl;
    __sync_fetch_and_add(&total_time, elipsed);
    __sync_fetch_and_add(&read_success, rhit);
    __sync_fetch_and_add(&read_failure, rfail);
    __sync_fetch_and_add(&modify_success, mhit);
    __sync_fetch_and_add(&modify_failure, mfail);
}

void prepare() {
    cout << "prepare" << endl;
    workers = new pthread_t[thread_number];
    parms = new struct target[thread_number];
    output = new stringstream[thread_number];
    for (int i = 0; i < thread_number; i++) {
        parms[i].tid = i;
        parms[i].store = store;
    }
}

void finish() {
    cout << "finish" << endl;
    delete[] parms;
    delete[] workers;
    delete[] output;
}

void multiWorkers() {
    output = new stringstream[thread_number];
    Tracer tracer;
    tracer.startTime();
    cout << "Insert " << exists << " " << tracer.getRunTime() << endl;
    Timer timer;
    timer.start();
    for (int i = 0; i < thread_number; i++) {
        pthread_create(&workers[i], nullptr, measureWorker, &parms[i]);
    }
    while (timer.elapsedSeconds() < timer_range) {
        sleep(1);
    }
    stopMeasure.store(1, memory_order_relaxed);
    for (int i = 0; i < thread_number; i++) {
        pthread_join(workers[i], nullptr);
        string outstr = output[i].str();
        cout << outstr;
    }
    cout << "Gathering ..." << endl;
}

int main(int argc, char **argv) {
    // dummy();
    if (argc > 7) {
        thread_number = std::atol(argv[1]);
        key_range = std::atol(argv[2]);
        total_count = std::atol(argv[3]);
        timer_range = std::atol(argv[4]);
        skew = std::stof(argv[5]);
        updatePercentage = std::atoi(argv[6]);
        erasePercentage = std::atoi(argv[7]);
        readPercentage = totalPercentage - updatePercentage - erasePercentage;
    }
    if (argc > 8) host_ip = argv[8];
    char host[DEFAULT_KEY_LENGTH];
    std::memset(host, 0, DEFAULT_KEY_LENGTH);
    std::strcpy(host, host_ip);
    ::mica::util::lcore.pin_thread(0);
    auto config = ::mica::util::Config::load_file("conf.json");
    LTableConfig::Alloc alloc(config.get("alloc"));
    LTableConfig::Pool pool(config.get("pool"), &alloc);

    store = new Table(config.get("table"), &alloc, &pool);
#if ENABLE_INITIALIZATION
    YCSBLoader loader(loadpath, key_range);
    loads = loader.load();
    key_range = loader.size();
    cout << "simple" << endl;
    simpleInsert();
#endif
    prepare();
    YCSBLoader runner(runpath, total_count);
    runs = runner.load();
    total_count = runner.size();
    cout << " threads: " << thread_number << " range: " << key_range << " count: " << total_count << " timer: "
         << timer_range << " skew: " << skew << " u:e:r = " << updatePercentage << ":" << erasePercentage << ":"
         << readPercentage << endl;
    cout << "multiinsert" << endl;
    multiWorkers();
    cout << "read operations: " << read_success << " read failure: " << read_failure << " modify operations: "
         << modify_success << " modify failure: " << modify_failure << " throughput: "
         << (double) (read_success + read_failure + modify_success + modify_failure) * thread_number / total_time
         << endl;
    std::cout << "---------------------------------------------------------------------------------------------------"
              << std::endl;
    //store->print_stats();
    loads.clear();
    runs.clear();
    finish();
    delete store;
    return EXIT_SUCCESS;
}