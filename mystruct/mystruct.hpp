#pragma once

#include <utility>
#include <limits>
#include <iterator>

#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef uint64_t u64;
typedef int64_t i64;

template <typename ValT>
struct List;

template <typename ValT>
struct ListNode;

template <typename ValT>
struct ListIterator;

template <typename ValT>
struct ListNode
{
    using pointer = ListNode<ValT>*;

    ValT val;
    ListNode<ValT> *prev = nullptr, *next = nullptr;

    ListNode(const ValT& val) : val(val) {};
    ListNode(ValT&& val) : val(std::move(val)) {};

    ListNode() = default;
};

template <typename ValT>
struct ListIterator
{
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type        = ValT;
    using difference_type   = std::ptrdiff_t;
    using pointer           = ValT*;
    using reference         = ValT&;

    explicit ListIterator(ListNode<ValT>* node) : node(node) {}

    reference operator*()  const { return node->val; }
    pointer   operator->() const { return &node->val; }

    ListIterator& operator++() { node = node->next; return *this; }
    ListIterator& operator--() { node = node->prev; return *this; }

    ListIterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
    ListIterator operator--(int) { auto tmp = *this; --(*this); return tmp; }

    bool operator==(const ListIterator& o) const { return node == o.node; }
    bool operator!=(const ListIterator& o) const { return node != o.node; }

private:
    ListNode<ValT>* node;
    friend struct List<ValT>;
};

template <typename ValT>
struct List
{
public:
    using Node = ListNode<ValT>;
    using Iterator = ListIterator<ValT>;
    

private:
    u64 size = 0;

    Node *head, *tail;

    public:

    Iterator begin() const
    {
        return Iterator(head->next);
    }
    Iterator end() const
    {
        return Iterator(tail);
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

        for(auto iter = list.begin(); iter != list.end(); iter++)
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

    ValT erase(Iterator iter)
    {   
        Node* node = iter.node;
        if(node == nullptr || node == head || node == tail) return {};
        
        node->prev->next = node->next;
        node->next->prev = node->prev;

        size--;

        ValT val(std::move(node->val));
        
        delete node;

        return val;
    }

    Iterator insertAfter(Iterator iter, ValT val)
    {
        Node* node = iter.node;
        if(node == nullptr || node == tail) return Iterator(nullptr);

        Node* newNode = new Node(std::move(val));
        
        newNode->prev = node;
        newNode->next = node->next;

        node->next->prev = newNode;
        node->next = newNode; 

        size++;

        return Iterator(newNode);
    }

    Iterator insertBefore(Iterator iter, ValT val)
    {
        Node* node = iter.node;
        if(node == nullptr || node == head) return Iterator(nullptr);

        Node* newNode = new Node(std::move(val));
        
        newNode->prev = node->prev;
        newNode->next = node;

        node->prev->next = newNode;
        node->prev = newNode;

        size++;

        return Iterator(newNode);
    }
};

template <typename ValT>
struct VectorIterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = ValT;
    using difference_type   = std::ptrdiff_t;
    using pointer           = ValT*;
    using reference         = ValT&;

    explicit VectorIterator(ValT* node) : node(node) {}

    reference operator*()  const { return *valPtr; }
    pointer   operator->() const { return  valPtr; }

    VectorIterator& operator++() { ++node; return *this; }
    VectorIterator& operator--() { --node; return *this; }

    VectorIterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
    VectorIterator operator--(int) { auto tmp = *this; --(*this); return tmp; }

    bool operator==(const ListIterator& o) const { return valPtr == o.valPtr; }
    bool operator!=(const ListIterator& o) const { return valPtr != o.valPtr; }

    
    VectorIterator& operator+= (i64 offset) { valPtr += offset; return *this; }
    VectorIterator& operator-= (i64 offset) { valPtr -= offset; return *this; }

    reference operator[] (i64 offset) { return node[offset]; }

private:
    ValT* valPtr;
    friend struct Vector<ValT>;
};

template <typename ValT>
struct Vector
{
public:
    using Iterator = VectorIterator;

    ValT *data = nullptr;
    u64 size = 0;
    u64 capacity;

private:
    Vector() : capacity(0) {}
    Vector(u64 capacity) : capacity(capacity) {}
    
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
        ValT* newData = new ValT[capacity];
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

    inline const ValT* getConstData() const noexcept
    {
        return data;
    }
    
    void push_back(const ValT& val)
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

    void pop_back()
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
        
        size--;
        data[size].~ValT();
    }

    ValT& back()
    {
        return data[size - 1];
    }

    VatT& front()
    {
        return data[0];
    }

    inline bool empty() const noexcept
    {
        return size == 0;
    }

    Iterator begin()
    {
        return Iterator(data);
    }

    Iterator end()
    {
        return Iterator(data + size);
    }
};




template<typename ValT>
struct Stack
{
private:

    ValT *data = nullptr;
    u64 size = 0;
    u64 capacity;

public:
    
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
        ValT* newData = new ValT[capacity];
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

    inline const ValT* getConstData() const noexcept
    {
        return data;
    }
    
    void push(const ValT& val)
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
        
        size--;
        data[size].~ValT();
    }

    ValT& top()
    {
        return data[size - 1];
    }

    inline bool empty() const noexcept
    {
        return size == 0;
    }
};


#warning List realization
template<typename ValT>
struct Queue
{
private:
    List<ValT> list;

public:
    using NodeT = typename List<ValT>::Node;

    Queue() : list() {};

    void push(ValT val)
    {
        list.insertBefore(list.end(), val);
    }

    void pop()
    {
        list.erase(list.begin());
    }

    u64 getSize() const
    {
        return list.getSize();
    }

    ValT front()
    {
        return *(list.begin());
    }

    ValT back()
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
        if(str == NULL) return;

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

