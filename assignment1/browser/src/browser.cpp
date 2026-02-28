#include<browser.hpp>

#include <stdio.h>


Browser& Browser::visit(const String& page)
{
    backStack.push(page);
    //replace with clear
    while(!forwardStack.empty())
    {
        forwardStack.pop();
    }

    history.push(page);

    return *this;
}

Browser& Browser::visit(String&& page)
{
    backStack.push(std::move(page));
    //replace with clear
    while(!forwardStack.empty())
    {
        forwardStack.pop();
    }

    history.push(page);

    return *this;
}

Browser& Browser::back(u64 n)
{
    u64 count = 0;
    while(count < n && backStack.getSize() > 1)
    {
        count++;
        auto tmp = backStack.top();
        backStack.pop();
        forwardStack.push(tmp);
    }

    history.push(backStack.top());

    return *this;
}

Browser& Browser::forward(u64 n)
{
    u64 count = 0;
    while(count < n && !forwardStack.empty())
    {
        count++;
        auto tmp = forwardStack.top();
        forwardStack.pop();
        backStack.push(tmp);
    }

    history.push(backStack.top());

    return *this;
}

String Browser::dumpHistory()
{
    String str;

    str.append(history.front().getConstData());
    history.pop();

    while(!history.empty())
    {
        str.append("\n");
        str.append(history.front().getConstData());
        history.pop();
    }

    return str;
}
