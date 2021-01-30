//
// Created by root on 1/28/21.
//

/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/queue.h>

#include <rte_common.h>
#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_ring.h>
#include <rte_log.h>
#include <rte_mempool.h>
#include <cmdline_rdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>
#include <cmdline_socket.h>
#include <cmdline.h>

#define RTE_LOGTYPE_APP RTE_LOGTYPE_USER1

extern struct rte_ring *send_ring;
extern struct rte_mempool *message_pool;
extern volatile int quit;

struct cmd_send_result {
    cmdline_fixed_string_t action;
    cmdline_fixed_string_t message;
};

static void cmd_send_parsed(void *parsed_result,
                            __rte_unused struct cmdline *cl,
                            __rte_unused void *data) {
    void *msg = NULL;
    struct cmd_send_result *res = parsed_result;

    if (rte_mempool_get(message_pool, &msg) < 0)
        rte_panic("Failed to get message buffer\n");
    strlcpy((char *) msg, res->message, STR_TOKEN_SIZE);
    if (rte_ring_enqueue(send_ring, msg) < 0) {
        printf("Failed to send message - message discarded\n");
        rte_mempool_put(message_pool, msg);
    }
}

cmdline_parse_token_string_t cmd_send_action =
        TOKEN_STRING_INITIALIZER(struct cmd_send_result, action, "send");
cmdline_parse_token_string_t cmd_send_message =
        TOKEN_STRING_INITIALIZER(struct cmd_send_result, message, NULL);

cmdline_parse_inst_t cmd_send = {
        .f = cmd_send_parsed,  /* function to call */
        .data = NULL,      /* 2nd arg of func */
        .help_str = "send a string to another process",
        .tokens = {        /* token list, NULL terminated */
                (void *) &cmd_send_action,
                (void *) &cmd_send_message,
                NULL,
        },
};

/**********************************************************/

struct cmd_quit_result {
    cmdline_fixed_string_t quit;
};

static void cmd_quit_parsed(__rte_unused void *parsed_result,
                            struct cmdline *cl,
                            __rte_unused void *data) {
    quit = 1;
    cmdline_quit(cl);
}

cmdline_parse_token_string_t cmd_quit_quit =
        TOKEN_STRING_INITIALIZER(struct cmd_quit_result, quit, "quit");

cmdline_parse_inst_t cmd_quit = {
        .f = cmd_quit_parsed,  /* function to call */
        .data = NULL,      /* 2nd arg of func */
        .help_str = "close the application",
        .tokens = {        /* token list, NULL terminated */
                (void *) &cmd_quit_quit,
                NULL,
        },
};

/**********************************************************/

struct cmd_help_result {
    cmdline_fixed_string_t help;
};

static void cmd_help_parsed(__rte_unused void *parsed_result,
                            struct cmdline *cl,
                            __rte_unused void *data) {
    cmdline_printf(cl, "Simple demo example of multi-process in RTE\n\n"
                       "This is a readline-like interface that can be used to\n"
                       "send commands to the simple app. Commands supported are:\n\n"
                       "- send [string]\n" "- help\n" "- quit\n\n");
}

cmdline_parse_token_string_t cmd_help_help =
        TOKEN_STRING_INITIALIZER(struct cmd_help_result, help, "help");

cmdline_parse_inst_t cmd_help = {
        .f = cmd_help_parsed,  /* function to call */
        .data = NULL,      /* 2nd arg of func */
        .help_str = "show help",
        .tokens = {        /* token list, NULL terminated */
                (void *) &cmd_help_help,
                NULL,
        },
};

/****** CONTEXT (list of instruction) */
cmdline_parse_ctx_t simple_mp_ctx[] = {
        (cmdline_parse_inst_t *) &cmd_send,
        (cmdline_parse_inst_t *) &cmd_quit,
        (cmdline_parse_inst_t *) &cmd_help,
        NULL,
};

extern cmdline_parse_ctx_t simple_mp_ctx[];

static const char *_MSG_POOL = "MSG_POOL";
static const char *_SEC_2_PRI = "SEC_2_PRI";
static const char *_PRI_2_SEC = "PRI_2_SEC";

struct rte_ring *send_ring, *recv_ring;
struct rte_mempool *message_pool;
volatile int quit = 0;

static int lcore_recv(__rte_unused void *arg) {
    unsigned lcore_id = rte_lcore_id();

    printf("Starting core %u\n", lcore_id);
    while (!quit) {
        void *msg;
        if (rte_ring_dequeue(recv_ring, &msg) < 0) {
            usleep(5);
            continue;
        }
        printf("core %u: Received '%s'\n", lcore_id, (char *) msg);
        rte_mempool_put(message_pool, msg);
    }

    return 0;
}

int main(int argc, char **argv) {
    const unsigned flags = 0;
    const unsigned ring_size = 64;
    const unsigned pool_size = 1024;
    const unsigned pool_cache = 32;
    const unsigned priv_data_sz = 0;

    int ret;
    unsigned lcore_id;

    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Cannot init EAL\n");

    if (rte_eal_process_type() == RTE_PROC_PRIMARY) {
        send_ring = rte_ring_create(_PRI_2_SEC, ring_size, rte_socket_id(), flags);
        recv_ring = rte_ring_create(_SEC_2_PRI, ring_size, rte_socket_id(), flags);
        uint32_t id = rte_socket_id();
        message_pool = rte_mempool_create(_MSG_POOL, pool_size,
                                          STR_TOKEN_SIZE, pool_cache, priv_data_sz,
                                          NULL, NULL, NULL, NULL,
                                          rte_socket_id(), flags);
    } else {
        recv_ring = rte_ring_lookup(_PRI_2_SEC);
        send_ring = rte_ring_lookup(_SEC_2_PRI);
        message_pool = rte_mempool_lookup(_MSG_POOL);
    }
    if (send_ring == NULL)
        rte_exit(EXIT_FAILURE, "Problem getting sending ring\n");
    if (recv_ring == NULL)
        rte_exit(EXIT_FAILURE, "Problem getting receiving ring\n");
    if (message_pool == NULL)
        rte_exit(EXIT_FAILURE, "Problem getting message pool\n");

    RTE_LOG(INFO, APP, "Finished Process Init.\n");

    /* call lcore_recv() on every worker lcore */
    RTE_LCORE_FOREACH_WORKER(lcore_id) {
        rte_eal_remote_launch(lcore_recv, NULL, lcore_id);
    }

    /* call cmd prompt on main lcore */
    struct cmdline *cl = cmdline_stdin_new(simple_mp_ctx, "\nsimple_mp > ");
    if (cl == NULL)
        rte_exit(EXIT_FAILURE, "Cannot create cmdline instance\n");
    cmdline_interact(cl);
    cmdline_stdin_exit(cl);

    rte_eal_mp_wait_lcore();
    return 0;
}
