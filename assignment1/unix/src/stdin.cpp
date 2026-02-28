#include <stdio.h>
#include <stdlib.h>

#include <unix.hpp>

int main()
{
    UnixPath solver;

    char path[UnixPath::MY_PATH_MAX];
    String ans;
    // u64 n;


    while(fgets(path, UnixPath::MY_PATH_MAX, stdin) != nullptr)
    {
        //убираю  /n в конце
        path[strcspn(path, "\n")] = '\0';

        ans = solver.solve(path);

        if(ans.getConstData() != nullptr)
        {
            printf("%s\n", ans.getConstData());
        }
    }
}