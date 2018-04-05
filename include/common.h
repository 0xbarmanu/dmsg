#ifndef COMMON_H
#define COMMON_H

#define OK      0
#define DEBUG   1
#define d_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0);

// A small struct to hold a UDP endpoint. 
// Use this to hold each client's endpoint.
#include <stdbool.h>

extern char* config_file;
extern char tracer[64];
extern char checker[64];
extern int  checker_port;
extern int  tracer_port;

typedef enum state {
        FREE      = 0, 
        VERIFIED  = 1,
        CHECKING  = 2
} state_e;

typedef enum command {
        HELLO_PUNCHING  = 0, 
        HELLO_UPNP      = 1,
        CHECK           = 2,
        CHECK_OK        = 3,
        ACK_HELLO       = 4,
        LOCATION
} command_e;

typedef struct client
{
    int         host;
    int         port; 
    state_e     state; 
    short       tries;
    // location
} client_t;

typedef struct msg {
        command_e       cmd;
        int             idx; // index of client in the database
        client_t        client_info;
} msg_t;



void init();
void diep(char *s);
// read the IP address and port of Tracer
int readTracerInfo(char* tracer, int* tracer_port, char* checker, int* checker_port);

#endif // COMMON_H
