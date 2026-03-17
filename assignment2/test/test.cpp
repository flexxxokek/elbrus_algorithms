#include <gtest/gtest.h>

#include <mysort.hpp>

#include <vector>
#include <algorithm>

TEST(BubbleSortTests, SimpleCases)
{
    std::vector vec {1, 2, 3, 5, 6, 99, 1, 9, 9};
    std::vector ansVec = vec;

    std::sort(ansVec.begin(), ansVec.end());

    MySort::BubbleSort(vec.begin(), vec.end(), std::less<int>{});

    EXPECT_EQ(vec, ansVec);

    vec = {2, 1};
    ansVec = vec;

    std::sort(ansVec.begin(), ansVec.end());

    MySort::BubbleSort(vec.begin(), vec.end(), std::less<int>{});

    EXPECT_EQ(vec, ansVec);
}

TEST(ShakerSortTests, SimpleCases)
{
    std::vector vec {2, 2, 1};
    std::vector ansVec = vec;

    std::sort(ansVec.begin(), ansVec.end());

    MySort::ShakerSort(vec.begin(), vec.end(), std::less<int>{});

    EXPECT_EQ(vec, ansVec);

    vec = {3, 2, 1, 99, 67, 2, 2, 1};
    ansVec = vec;

    std::sort(ansVec.begin(), ansVec.end());

    MySort::ShakerSort(vec.begin(), vec.end(), std::less<int>{});

    EXPECT_EQ(vec, ansVec);

    vec.resize(static_cast<size_t>(1e4));

    for(auto& i : vec) i = rand();

    ansVec=vec;

    std::sort(ansVec.begin(), ansVec.end());

    MySort::ShakerSort(vec.begin(), vec.end(), std::less<int>{});

    EXPECT_EQ(vec, ansVec);
}

TEST(CombSortTests, SimpleCases)
{
    std::vector vec {2, 2, 1};
    std::vector ansVec = vec;

    std::sort(ansVec.begin(), ansVec.end());

    MySort::CombSort(vec.begin(), vec.end(), std::less<int>{});

    EXPECT_EQ(vec, ansVec);

    vec = {3, 2, 1, 99, 67, 2, 2, 1};
    ansVec = vec;

    std::sort(ansVec.begin(), ansVec.end());

    MySort::CombSort(vec.begin(), vec.end(), std::less<int>{});

    EXPECT_EQ(vec, ansVec);

    vec.resize(static_cast<size_t>(1e4));

    for(auto& i : vec) i = rand();

    ansVec=vec;

    std::sort(ansVec.begin(), ansVec.end());

    MySort::ShakerSort(vec.begin(), vec.end(), std::less<int>{});

    EXPECT_EQ(vec, ansVec);
}

TEST(RadixSortTests, SimpleCases)
{
    std::vector <unsigned> vec {1, 2, 3, 5, 6, 99, 1, 9, 9};
    std::vector ansVec = vec;

    std::sort(ansVec.begin(), ansVec.end());

    MySort::RadixSort(vec.begin(), vec.end());

    EXPECT_EQ(vec, ansVec);

    vec.resize(static_cast<size_t>(1e4));

    for(auto& i : vec) i = rand();

    ansVec=vec;

    std::sort(ansVec.begin(), ansVec.end());

    MySort::ShakerSort(vec.begin(), vec.end(), std::less<int>{});

    EXPECT_EQ(vec, ansVec);
}