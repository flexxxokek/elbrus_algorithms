#include <stdio.h>
#include <stdlib.h>

#include <browser.hpp>

constexpr u64 MAX_ILEN = 16;
constexpr u64 MAX_ALEN = 1028;

constexpr u64 MAX_LLEN = MAX_ALEN + MAX_ILEN;


int main()
{
    Browser browser;

    char line[MAX_LLEN] = {};
    char instruction[MAX_ILEN];


    u64 numArg = 0;
    char strArg[MAX_ALEN] = {};

    int argOffset = 0;

    while(fgets(line, MAX_LLEN, stdin) != nullptr)
    {
        line[strcspn(line, "\n")] = '\0';

        //reading the instruction
        if(sscanf(line, "%15s", instruction) == EOF)
        {
            break;
        }

        argOffset = strlen(instruction);

        // printf("ARGUMENT: %s\n", line + argOffset);

        if(!strcmp(instruction, "visit"))
        {
            if(sscanf(line + argOffset, "%s", strArg) < 1)
            {
                printf("No argument\n");
                strArg[0] = '\0';
            }

            browser.visit(strArg);
        }
        else if(!strcmp(instruction, "forward"))
        {
            if(sscanf(line + argOffset, "%lu", &numArg) < 1)
            {
                printf("No argument\n");
                numArg = 0;
            }

            browser.forward(numArg);
        }
        else if(!strcmp(instruction, "back"))
        {
            if(sscanf(line + argOffset, "%lu", &numArg) < 1)
            {
                printf("No argument\n");
                numArg = 0;
            }

            browser.back(numArg);
        }
        else if(!strcmp(instruction, "end"))
        {
            break;
        }
    }

    auto out = browser.dumpHistory();

    if(out.getConstData() != nullptr) printf("%s\n", out.getConstData());
}