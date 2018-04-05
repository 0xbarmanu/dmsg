#include "../include/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* tracer_file = "./config/tracer.ini";

char tracer[64];
char checker[64];
int  tracer_port;
int  checker_port;

// Just a function to kill the program when something goes wrong.
void diep(char *s)
{
    perror(s);
    exit(1);
}

void init()
{
    char tracer[64];


    if (readTracerInfo(tracer, &tracer_port, checker, &checker_port) != OK)
    {
        diep("Failed to read ini file!");
    }    
}

// read the IP address and port of Tracer
int readTracerInfo(char* tracer, int* tracer_port, char* checker, int* checker_port)
{
        FILE* fp;
        char buf[128];

        if ((fp = fopen(tracer_file, "r")) == NULL)
        {       /* Open source file. */
                perror(tracer_file);
                return 1;
        }
        
        // read Tracer IP address
        if (fgets(tracer, 256, fp) != NULL)
        {
                tracer[strlen(tracer) - 1] = '\0'; // eat the newline fgets() stores
                // d_print("%s\n", tracer);
        }

        // read tracer port
        if (fgets(buf, sizeof(buf), fp) != NULL)
        {
                buf[strlen(buf)] = '\0'; // eat the newline fgets() stores
                *tracer_port = atoi(buf);
                // d_print("%d\n", *tracer_port);
        }

        // read checker IP address
        if (fgets(checker, 256, fp) != NULL)
        {
                checker[strlen(checker)-1] = '\0'; // eat the newline fgets() stores
                // d_print("%s\n", checker);
        }

        // read checker port
        if (fgets(buf, sizeof(buf), fp) != NULL)
        {
                buf[strlen(buf)] = '\0'; // eat the newline fgets() stores
                *checker_port = atoi(buf);
                // d_print("%d\n", *checker_port);
        }

        fclose(fp);

        return OK;
}