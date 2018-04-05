#ifndef TRACER_H
#define TRACER_H
#include "common.h"
// purpose: find a free index in client-list
// input: i - search from this index
// return: free index
//         or -1 if no more free index
int findFreeIdx(int i);

// purpose: send CHECK command to the checker to check the new client is: uPnP, Hole punching, PMP, or PCP
// input: s - socket of checker
//        idx - index of client to be checked
//        client_info - info of client to be checked
void sendCheckCmd(int s, int idx, client_t client_info);

#endif