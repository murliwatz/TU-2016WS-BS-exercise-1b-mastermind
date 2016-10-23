/*
 * @brief main c file for the implementation of stegit
 * @author Paul Pröll, 1525669
 * @date 2016-10-08
 *
 * Copyright (c) 2012-2015 OSUE Team <osue-team@vmars.tuwien.ac.at>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * gcc -std=c99 -Wall -g -pedantic -DENDEBUG \
 *      -D_BSD_SOURCE -D_XOPEN_SOURCE=500 -o server server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include "client.h"

/* === Macros === */

#ifdef ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif

/* Length of an array */
#define COUNT_OF(x) (sizeof(x)/sizeof(x[0]))

/* === Global Variables === */

/* Name of the program */
static const char *progname = "client"; /* default name */

/* File descriptor for server socket */
static int sockfd = -1;

/* File descriptor for connection socket */
static int connfd = -1;

/* This variable is set upon receipt of a signal */
volatile sig_atomic_t quit = 0;

/* === Implementations === */

static uint8_t *read_from_server(int fd, uint8_t *buffer, size_t n)
{
    /* loop, as packet can arrive in several partial reads */
    size_t bytes_recv = 0;
    do {
        ssize_t r;
        r = recv(fd, buffer + bytes_recv, n - bytes_recv, 0);
        if (r <= 0) {
            return NULL;
        }
        bytes_recv += r;
    } while (bytes_recv < n);

    if (bytes_recv < n) {
        return NULL;
    }
    return buffer;
}

static uint8_t *send_to_server(int fd, uint8_t *buffer, size_t n)
{
    /* loop, as packet can arrive in several partial reads */
    size_t bytes_sent = 0;
    do {
        ssize_t r;
        r = send(fd, buffer + bytes_sent, n - bytes_sent, 0);
        if (r <= 0) {
            return NULL;
        }
        bytes_sent += r;
    } while (bytes_sent < n);

    if (bytes_sent < n) {
        return NULL;
    }
    return buffer;
}

static int compute_answer(uint8_t req)
{
    /** TODO **/
    int red = req & 0x7;
    req >>= 0x7;
    int white = req & 0x7;
    req >>= 0x1;
    int parity_fault = req & 0x1;
    req >>= 0x1;
    int game_lost = req & 0x1;
    fprintf(stderr, "Red: %d White: %d\n", red, white);
    return -1;
}

static void bail_out(int exitcode, const char *fmt, ...)
{
    va_list ap;

    (void) fprintf(stderr, "%s: ", progname);
    if (fmt != NULL) {
        va_start(ap, fmt);
        (void) vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    if (errno != 0) {
        (void) fprintf(stderr, ": %s", strerror(errno));
    }
    (void) fprintf(stderr, "\n");

    free_resources();
    exit(exitcode);
}

static void free_resources(void)
{
    /* clean up resources */
    DEBUG("Shutting down server\n");
    if(connfd >= 0) {
        (void) close(connfd);
    }
    if(sockfd >= 0) {
        (void) close(sockfd);
    }
}

static void signal_handler(int sig)
{
    quit = 1;
}

static void gen_message(uint16_t *buffer) {
    int colors[SLOTS] = {
        rand() % COLORS,
        rand() % COLORS,
        rand() % COLORS,
        rand() % COLORS,
        rand() % COLORS
    };

    int parity_calc = 0;
    for(int i = 0; i < SLOTS; i++) {
        *buffer |= colors[i] & 0x7;

        parity_calc ^= colors[i] ^ (colors[i] >> 1) ^ (colors[i] >> 2);
        
        if((i+1) < SLOTS)
            *buffer = *buffer << 3;

        //fprintf(stderr, "%d\n", colors[i]);
    }

    *buffer = *buffer | (parity_calc << 15);
    //fprintf(stderr, "%d ", parity_calc & 0x1);
}

/**
 * @brief Program entry point
 * @param argc The argument counter
 * @param argv The argument vector
 * @return EXIT_SUCCESS on success, EXIT_PARITY_ERROR in case of an parity
 * error, EXIT_GAME_LOST in case client needed to many guesses,
 * EXIT_MULTIPLE_ERRORS in case multiple errors occured in one round
 */
int main(int argc, char *argv[])
{

    struct opts options;
    int round;
    int ret;

    parse_args(argc, argv, &options);

    /* setup signal handlers */
    const int signals[] = {SIGINT, SIGTERM};
    struct sigaction s;

    s.sa_handler = signal_handler;
    s.sa_flags   = 0;
    if(sigfillset(&s.sa_mask) < 0) {
        bail_out(EXIT_FAILURE, "sigfillset");
    }
    for(int i = 0; i < COUNT_OF(signals); i++) {
        if (sigaction(signals[i], &s, NULL) < 0) {
            bail_out(EXIT_FAILURE, "sigaction");
        }
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        bail_out(EXIT_FAILURE, "creating socket");
    }

    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(options.portno);
    serv_addr.sin_addr.s_addr = options.hname.s_addr;

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        bail_out(EXIT_FAILURE, "connecting to server");
    }

    /* connection established */
    ret = EXIT_SUCCESS;
    
    /** TODO **/
    static uint16_t buffer;
    static uint8_t buffer_answer;

    srand(time(NULL));

    for (round = 1; round <= MAX_TRIES; ++round) {
        buffer = 0;
        gen_message(&buffer);
        if (send_to_server(sockfd, &buffer, WRITE_BYTES) == NULL) {
            if (quit) break; /* caught signal */
            bail_out(EXIT_FAILURE, "send_to_server");
        }
        if (read_from_server(sockfd, &buffer_answer, READ_BYTES) == NULL) {
            if (quit) break; /* caught signal */
            bail_out(EXIT_FAILURE, "read_from_server");
        }
        compute_answer(buffer_answer);
        //fprintf(stderr, "%d", buffer);

        sleep(1);
    }

    /* we are done */
    free_resources();
    return ret;
}

static void parse_args(int argc, char **argv, struct opts *options)
{
    int i;
    char *port_arg;
    char *hname_arg;
    char *endptr;
    enum { beige, darkblue, green, orange, red, black, violet, white };

    if(argc > 0) {
        progname = argv[0];
    }
    if (argc != 3) {
        bail_out(EXIT_FAILURE,
            "Usage: %s <server-hostname> <server-port>", progname);
    }
    port_arg = argv[2];
    hname_arg = argv[1];

    errno = 0;
    options->portno = strtol(port_arg, &endptr, 10);

    if ((errno == ERANGE &&
          (options->portno == LONG_MAX || options->portno == LONG_MIN))
        || (errno != 0 && options->portno == 0)) {
        bail_out(EXIT_FAILURE, "strtol");
    }

    if (endptr == port_arg) {
        bail_out(EXIT_FAILURE, "No digits were found");
    }

    /* If we got here, strtol() successfully parsed a number */

    if (*endptr != '\0') { /* In principle not necessarily an error... */
        bail_out(EXIT_FAILURE,
            "Further characters after <server-port>: %s", endptr);
    }

    /* check for valid port range */
    if (options->portno < 1 || options->portno > 65535)
    {
        bail_out(EXIT_FAILURE, "Use a valid TCP/IP port range (1-65535)");
    }

    struct in_addr ip;
    if(inet_aton(hname_arg, &ip) < 0) {
        bail_out(EXIT_FAILURE, "<server-hostname> can't be resolved");
    }

    options->hname = ip;
}
