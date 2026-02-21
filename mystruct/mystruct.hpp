#pragma once

#include <utility>
#include <limits>

#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef uint64_t u64;
typedef int64_t i64;

template <typename ListT>
struct List
{
    public:
    struct Node
    {
        ListT val;
        Node *prev = nullptr, *next = nullptr;

        Node(const ListT& val) : val(val) {};
        Node(ListT&& val) : val(std::move(val)) {};

        Node() = default;
    };

    private:
    u64 size = 0;

    Node *head, *tail;

    public:

    Node* begin() const
    {
        return head->next;
    }
    Node* end() const
    {
        return tail;
    }

    List()
    {
        head = new Node();
        tail = new Node();
        head->next = tail;
        tail->prev = head;
    }

    List(const List& list) : size(list.size)
    {
        head = new Node();
        tail = new Node();
        Node *prevNode = head;



        for(Node *iter = list.begin(); iter != list.end(); iter = iter->next)
        {
            Node *newNode = new Node(iter->val);
            prevNode->next = newNode;
            newNode->prev = prevNode;
            prevNode = newNode;
        }

        prevNode->next = tail;
        tail->prev = prevNode;
    }
    
    List(List&& list) : head(list.head), tail(list.tail), size(list.size)
    {
        list.head = nullptr;
        list.tail = nullptr;
        list.size = 0;
    }

    ~List()
    {
        Node *next = nullptr;
        for(Node *iter = head; iter != nullptr; iter = next)
        {
            next = iter->next;
            delete iter;
        }
    }
    
    inline u64 getSize() const noexcept
    {
        return size;
    }

    ListT erase(Node* node)
    {   
        if(node == nullptr || node == head || node == tail) return {};
        
        node->prev->next = node->next;
        node->next->prev = node->prev;

        size--;

        ListT val(std::move(node->val));
        
        delete node;

        return val;
    }

    Node* insertAfter(Node* node, ListT val)
    {
        if(node == nullptr || node == tail) return nullptr;

        Node* newNode = new Node(std::move(val));
        
        newNode->prev = node;
        newNode->next = node->next;

        node->next->prev = newNode;
        node->next = newNode; 

        size++;

        return newNode;
    }

    Node* insertBefore(Node* node, ListT val)
    {
        if(node == nullptr || node == head) return nullptr;

        Node* newNode = new Node(std::move(val));
        
        newNode->prev = node->prev;
        newNode->next = node;

        node->prev->next = newNode;
        node->prev = newNode;

        size++;

        return newNode;
    }
};

template<typename StackT>
struct Stack
{
    private:

    StackT *data = nullptr;
    u64 size = 0;
    u64 capacity;

    public:     //////////////////////////////////////
    
    Stack() : capacity(0) {};
    Stack(u64 capacity) : capacity(capacity) {}
    
    void setCapacity(u64 cap)
    {
        if(cap == 0)
        {
            return;
        }
        else if(cap > std::numeric_limits<u64>().max() / 20)
        {
            printf("too big capacity\n");
            return;
        }

        capacity = cap;
        StackT* newData = new StackT[capacity];
        for(int i = 0; i < size; i++)
        {
            newData[i] = std::move(data[i]);
        }
        
        delete[] data;
        data = newData;
    }

    inline u64 getCapacity() const noexcept
    {
        return capacity;
    }

    inline u64 getSize() const noexcept
    {
        return size;
    }

    inline const StackT* getData() const noexcept
    {
        return data;
    }
    
    void push(const StackT& val)
    {
        if(capacity == 0)
        {
            setCapacity(10);
        }
        // для лучшего избегания переполнения
        u64 usagePercentage = size * 10 / capacity * 10;

        if(usagePercentage > 100)
        {
            printf("usage > 100\n");
            return;
        }
        else if(capacity * 10 < size)
        {
            printf("capacity overflow\n");
            return;
        }
        else if(usagePercentage >= 80)
        {
            setCapacity(capacity * 2);
        }

        data[size] = val;
        size++;
    }

    void pop()
    {
        u64 usagePercentage = size * 10 / capacity * 10;
        if(usagePercentage > 100)
        {
            printf("usage > 100 \n");
            return;
        }
        else if(capacity * 10 < size)
        {
            printf("capacity overflow\n");
            return;
        }
        else if(usagePercentage < 20)
        {
            setCapacity(capacity / 2);
        }
        
        data[size] = {};
        size--;
    }

    StackT& top()
    {
        return data[size - 1];
    }

    inline bool empty() const noexcept
    {
        return size == 0;
    }
};


#warning List realization
template<typename QueueT>
struct Queue
{
    private:
    List<QueueT> list;

    public:
    using NodeT = typename List<Queue>::Node;

    Queue() : list() {};

    void push(QueueT val)
    {
        list.insertBefore(list.end(), val);
    }

    void pop()
    {
        list.erase(list.begin());
    }

    NodeT* begin() const
    {
        return list.begin();
    }

    NodeT* end() const
    {
        return list.end();
    }

    u64 getSize() const
    {
        return list.getSize();
    }

    QueueT front()
    {
        return list.begin()->val;
    }

    QueueT back()
    {
        return list.end()->prev->val;
    }

    inline bool empty() const noexcept
    {
        return list.getSize() == 0;
    }
};

struct String
{
    private:
    u64 size;
    char *data;

    public:
    String() : size(0), data(nullptr) {};

    String(const char *str)
    {
        size = strlen(str);
        data = new char[size + 1];

        strcpy(data, str);
    }

    String(const char *begin, const char *end)
    {
        if(begin > end)
        {
            return;
        }

        size = end - begin;
        data = new char[size + 1];

        strncpy(data, begin, size);
        data[size] = '\0';
    }

    //copy constructor because segfault
    String(const String& str) : size(str.size)
    {
        data = new char[size + 1];
        strcpy(data, str.data);
    }

    //move constructor (rule of 5)
    String(String&& str) : data(str.data), size(str.size)
    {
        str.data = nullptr;
        str.size = 0;
    }

    String& operator=(const String& str)
    {
        size = str.size;
        
        delete[] data;
        data = new char[size + 1];
        
        strcpy(data, str.data);

        return *(this);
    }

    String& operator=(String&& str)
    {
        size = str.size;

        delete[] data;
        data = str.data;
        str.data = nullptr;

        return *(this);
    }

    ~String()
    {
        delete[] data;
    }

    inline char* getData() const noexcept
    {
        return data;
    }

    inline const char* getConstData() const noexcept
    {
        return data;
    }

    inline const u64 getSize() const noexcept
    {
        return size;
    }

    void append(const char* str)
    {
        u64 strSize = strlen(str);

        char* newStr = new char[size + strSize + 1];

        memcpy(newStr, data, size);

        delete[] data;

        memcpy(newStr + size, str, strSize);
        newStr[size + strSize] = '\0';

        size += strSize;
        data = newStr;
    }

    void append(const char* str, u64 n)
    {
        char* newStr = new char[size + n + 1];

        memcpy(newStr, data, size);

        delete[] data;
        
        u64 i = 0;
        while(i < n && str[i] != '\0')
        {
            newStr[size + i] = str[i];
            i++;
        }

        newStr[size + i] = '\0';

        size += i;
        data = newStr;
    }

};