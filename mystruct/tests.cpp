#include <array>
#include <cstdlib>

#include <gtest/gtest.h>

//my library
#include <mystruct.hpp>

TEST(ListTests, simple)
{
    List<int> list;
    for(i64 i = 0; i < 10; i++)
    {
        list.insertBefore(list.end(), i);
    }

    auto nd = list.begin()->next->next;

    for(i64 i = 0; i < 3; i++)
    {
        list.erase(nd->next);
    }
    
    auto list_i = list.begin();
    std::array arr{0,1,2,6,7,8,9};

    for(const auto i : arr)
    {
        EXPECT_EQ(list_i->val, i);
        list_i = list_i->next;
    }
}

TEST(StringTests, simple)
{
    String s;
    s.append("stringi");
    EXPECT_STREQ(s.getConstData(), "stringi");
    s.append("\njoo");
    EXPECT_STREQ(s.getConstData(), "stringi\njoo");
}

TEST(ListOfStringsTests, simple)
{
    List<String> list;

    for(i64 i = 0; i < 10; i++)
    {
        list.insertBefore(list.end(), std::to_string(i).c_str());
    }

    auto nd = list.begin()->next->next;

    for(i64 i = 0; i < 3; i++)
    {
        list.erase(nd->next);
    }
    
    auto list_i = list.begin();
    std::array arr{"0", "1", "2","6","7","8","9"};

    for(const auto i : arr)
    {
        EXPECT_STREQ(list_i->val.getConstData(), i);
        list_i = list_i->next;
    }

    EXPECT_EQ(list.getSize(), arr.size());
}

TEST(StackTests, simple)
{
    Stack<int> st;

    std::array arr {1,2,3,5,6,78,9};

    for(const auto i : arr)
    {
        st.push(i);
    }

    EXPECT_EQ(st.top(), 9);
    st.pop();
    EXPECT_EQ(st.top(), 78);

    EXPECT_EQ(st.getSize(), 6);

    for(const auto i : std::array{78, 6, 5,3,2,1})
    {
        EXPECT_EQ(i, st.top());
        st.pop();
    }
}

TEST(QueueTests, simple)
{
    Queue<int> q;
    std::array arr {1,2,3,5,6,78,9};

    for(const auto i : arr)
    {
        q.push(i);
    }

    EXPECT_EQ(arr.size(), q.getSize());

    for(const auto i : arr)
    {
        EXPECT_EQ(q.front(), i);
        q.pop();
    }
}
