// UDP hole punching example, server code
// Base UDP code stolen from http://www.abc.se/~m6695/udp.html
// By Oscar Rodriguez
// This code is public domain, but you're a complete lunatic
// if you plan to use this code in any real program.
 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "common.h"
#include "tracer.h"

#define NCLIENT 10
#define CLOCK_TICK 5 // tick every 5 seconds
 
char tracer[64];
char checker[64];
int  checker_port;
int  tracer_port;
volatile client_t clients[NCLIENT];

int findFreeIdx(int i)
{
    int temp = i;
    if (i == NCLIENT) i = 0;
    while (clients[i].state != FREE) {
        i ++;
        if (i == NCLIENT) i = 0;
        if (i == temp) 
            return -1; // not found free index
    }

    return i;
}

void sendCheckCmd(int s, int idx, client_t client_info)
{
    struct  sockaddr_in  si_checker;
    msg_t   msg;
    int     slen=sizeof(si_checker);

    si_checker.sin_family = AF_INET;
    si_checker.sin_addr.s_addr = inet_addr(checker);
    si_checker.sin_port = htons(checker_port);

    msg.cmd = CHECK;
    msg.idx = idx;
    msg. client_info = client_info;
    if (sendto(s, &msg, sizeof(msg), 0, (struct sockaddr*)(&si_checker), slen)==-1)
        diep("sendto");

    d_print("Sent CHECK to checker at %s:%d msg.cmd=%d\n", inet_ntoa(si_checker.sin_addr), si_checker.sin_port, msg.cmd);

}

void sigalrm_handler( int sig )
{
    struct sockaddr_in si_temp;
    int i;
    for (i = 0; i < NCLIENT; i++)
    {
        if (clients[i].state == CHECKING)
        {
            clients[i].tries ++;

            if (clients[i].tries >= 2)
            {
                clients[i].state = FREE;
                clients[i].tries = 0;
                si_temp.sin_addr.s_addr = clients[i].host;
                d_print("Timeout for checking client idx %d %s:%d\n", i, inet_ntoa(si_temp.sin_addr), clients[i].port);
            }
            else if (clients[i].tries == 1)
            {
                // TODO: try to send one more time
            }
        }
    }
    alarm(CLOCK_TICK);
}

int start_timer()
{
    struct sigaction sact;
    sigemptyset (&sact.sa_mask);
    sact.sa_flags = 0;
    sact.sa_handler = sigalrm_handler;
    sigaction(SIGALRM, &sact, NULL);

    alarm(CLOCK_TICK);  /* Request SIGALRM in 5 seconds */

    return 0;
}

int main(void)
{
    struct sockaddr_in si_me, si_other, si_reply;
    int s,  i, j, slen=sizeof(si_other);
    msg_t   msg, sendmsg;
    int     idx = -1;
    fd_set  read_flags;
    struct timeval waitd;          // the max wait time for an event
    int     sel;
 
  
    if (readTracerInfo(tracer, &tracer_port, checker, &checker_port) != OK)
    {
        diep("Failed to read ini file!");
    }  

    // Create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");
 
    // init
    memset((char *)&clients, 0, sizeof(client_t)*NCLIENT);
    
    // si_me stores our local endpoint. Remember that this program
    // has to be run in a network with UDP endpoint previously known
    // and directly accessible by all clients. In simpler terms, the
    // server cannot be behind a NAT.
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(tracer_port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr*)(&si_me), sizeof(si_me))==-1)
        diep("bind");

    start_timer();

    while (1)
    {
        // When a new client sends a datagram...
        while(1) {
            waitd.tv_sec  = 10;
            waitd.tv_usec = 0;
            FD_ZERO(&read_flags);
            FD_SET(s, &read_flags);

            sel = select(s+1, &read_flags, NULL, (fd_set*)0, &waitd);
            if(sel < 0)
                continue;

            //socket ready for reading
            if(FD_ISSET(s, &read_flags)) {
                FD_CLR(s, &read_flags);
            
                memset((char*) &msg, 0, sizeof(msg));
                if (recvfrom(s, &msg, sizeof(msg), 0, (struct sockaddr*)(&si_other), (socklen_t*)&slen)==-1)
                    diep("recvfrom");
            
                break;
            }
        }

        d_print("Received packet from %s:%d msg.command=%d\n", inet_ntoa(si_other.sin_addr), si_other.sin_port, msg.cmd);
        
        switch (msg.cmd) {
            case HELLO_PUNCHING:
                // Find a free space to store new client
                idx = findFreeIdx(idx+1);
                if (idx == -1)
                    diep("No more space for new client");

                // store new client info
                clients[idx].host = si_other.sin_addr.s_addr;
                clients[idx].port = si_other.sin_port;

                sendCheckCmd(s, idx, clients[idx]);
                clients[idx].state = CHECKING;

                // reply ACK_HELLO to the new client
                msg.cmd = ACK_HELLO;
                if (sendto(s, &msg, sizeof(msg), 0, (struct sockaddr*)(&si_other), slen)==-1)
                    diep("sendto");

                d_print("Send ACK_HELLO packet to %s:%d msg.command=%d\n", 
                        inet_ntoa(si_other.sin_addr), si_other.sin_port, msg.cmd);

            break;

            case CHECK_OK:
                if (clients[msg.idx].state == CHECKING)
                {
                    clients[msg.idx].state = VERIFIED;
                    si_other.sin_addr.s_addr = msg.client_info.host;
                }
                else{
                    d_print("Client state error %d, expected state %d", clients[msg.idx].state, CHECKING);
                }
                d_print("Received CHECK_OK for client %s:%d idx=%d\n", inet_ntoa(si_other.sin_addr), msg.client_info.port, msg.idx);
            break;

            case HELLO_UPNP:

            break;
            case LOCATION:
            break;

            default:
            break;
        }
        // The client's public UDP endpoint data is now in si_other.
        // Notice that we're completely ignoring the datagram payload.
        // If we want to support multiple clients inside the same NAT,
        // we'd have clients send their own private UDP endpoints
        // encoded in some way inside the payload, and store those as
        // well.
        // Now we add the client's UDP endpoint in our list.

        // And then tell everybody about everybody's public UDP endpoints

        d_print("Now we have %d clients\n", idx);
        // And we go back to listening. Notice that since UDP has no notion
        // of connections, we can use the same socket to listen for data
        // from different clients.
        // break;
    }
 
    // Actually, we never reach this point...
    close(s);
    return 0;
}