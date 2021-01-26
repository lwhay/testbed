//
// Created by iclab on 1/26/21.
//

#ifndef TESTBED_SETTINGS_H
#define TESTBED_SETTINGS_H

char *ip_addr = "127.0.0.1";

int batch_size = 4;

int total_round = (1llu << 20);

int sender_num = 4;

int port_num;

int using_dummy = 0;

#define DUMMY_LENGTH 4

#define  NUM_THREADS 4

int server_port_base = 8033;

#define PORT_NUM port_num

#define THREAD_NUM port_num

#define PORT_BASE 8033

#define INIT_READ_BUF_SIZE 5000
#define INIT_RET_BUF_SIZE 1000
#define BATCH_BUF_SIZE 1000000

#define MAX_CONN 65535

#define ROUND_NUM 20

#define TEST_WORK_STEP_SIZE 28

#endif //TESTBED_SETTINGS_H
