#pragma once

#include <mystruct.hpp>

#include <stdio.h>


struct Browser
{
    Stack<String> backStack;
    Stack<String> forwardStack;

    Queue<String> history;

    Browser()
    {
        backStack.push("homepage");
        history.push("homepage");
    }

    Browser& visit(const String& page);

    Browser& visit(String&& page);

    Browser& back(u64 n);

    Browser& forward(u64 n);

    String dumpHistory();
};