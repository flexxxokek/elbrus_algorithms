#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//in unix.hpp my librar
#include <unix.hpp>

String UnixPath::solve(const char* inputPath) const
{
    List<String> pathList;
    using PathNode = decltype(pathList)::Node;

    u64 nameLength = 0;
    const char* iter = inputPath;
    String name;

    for(;*iter != '\0'; iter++)
    {
        if(*iter == '/')
        {
            if(nameLength != 0)
            {
                name = String(iter - nameLength, iter);
                pathList.insertBefore(pathList.end(), std::move(name));
            }
            nameLength = 0;
        }
        else
        {
            nameLength++;
        }
    }

    if(nameLength != 0)
    {
        pathList.insertBefore(pathList.end(), String(iter - nameLength, iter));
    }

    // for(const PathNode *iter = pathList.begin(); iter != pathList.end(); iter = iter->next)
    // {
    //     printf("Node: %s\n", iter->val.getConstData());
    // }
    
    
    PathNode *next = nullptr; 
    for(PathNode *iter = pathList.begin(); iter != pathList.end(); iter = next)
    {
        next = iter->next;
        
        if(!strcmp(iter->val.getConstData(), ".."))
        {
            #warning !!!!!!!!!
            if(iter == pathList.begin())
            {
                return ERROR_STRING;
                break;
            }
            // printf("found .. directory\n");
            pathList.erase(iter->prev);
            pathList.erase(iter);
        }
        else if(!strcmp(iter->val.getConstData(), "."))
        {
            pathList.erase(iter);
            // printf("found . directory\n");
        }
    }

    String canonPath;
    //root dir
    canonPath.append("/"); 

    for(const PathNode* iter = pathList.begin(); iter != pathList.end(); iter = iter->next)
    {
        canonPath.append(iter->val.getConstData());
        if(iter != pathList.end()->prev) canonPath.append("/");
    }


    // puts(path.data);
    
    return canonPath;
}