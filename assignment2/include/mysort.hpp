#pragma once

#include <vector>
#include <cstdlib>
#include <iterator>
#include <concepts>

namespace MySort
{

template <typename IteratorT, typename CompT>
    requires
    std::bidirectional_iterator<IteratorT> &&
    std::strict_weak_order<CompT, std::iter_value_t<IteratorT>, std::iter_value_t<IteratorT>>

void BubbleSort(IteratorT begin, IteratorT end, CompT comp)
{
    auto iterationEnd = end;
    while(iterationEnd != begin)
    {
        auto iter = begin;
        auto next = std::next(iter);

        while(next != iterationEnd)
        {
            if(comp(*next, *iter) == true)
            {
                std::swap(*next, *iter);
            }

            iter++;
            next++;
        }

        iterationEnd--;
    }
}

template <typename IteratorT, typename CompT>
    requires
    std::bidirectional_iterator<IteratorT> &&
    std::strict_weak_order<CompT, std::iter_value_t<IteratorT>, std::iter_value_t<IteratorT>>
    
void BubbleSortWithCondition(IteratorT begin, IteratorT end, CompT comp)
{
    auto iterationEnd = end;
    while(iterationEnd != begin)
    {
        bool isSorted = true;

        auto iter = begin;
        auto next = std::next(iter);

        while(next != iterationEnd)
        {
            if(comp(*next, *iter) == true)
            {
                std::swap(*next, *iter);
                isSorted = false;
            }

            iter++;
            next++;
        }
        
        if(isSorted) break;

        iterationEnd--;
    }
}

template <typename IteratorT, typename CompT>
    requires
    std::bidirectional_iterator<IteratorT> &&
    std::strict_weak_order<CompT, std::iter_value_t<IteratorT>, std::iter_value_t<IteratorT>>
    
void ShakerSort(IteratorT begin, IteratorT end, CompT comp)
{    
    auto iterationEnd = end;
    auto iterationBegin = begin;

    auto iter = iterationBegin;

    constexpr bool forward = false, backward = true;
    bool direction = forward;

    bool isSorted = true; 
    
    while(std::distance(iterationBegin, iterationEnd) > 0)
    {
        if(direction == forward)
        {
            auto next = std::next(iter);

            while(next != iterationEnd)
            {
                if(comp(*next, *iter) == true)
                {
                    std::swap(*next, *iter);
                    isSorted = false;
                }

                iter++;
                next++;
            }

            iterationEnd = iter; // <=> iterationEnd--;
        }
        else if(direction == backward)
        {
            auto prev = std::prev(iter);

            while (iter != iterationBegin)
            {
                if(!comp(*prev, *iter))
                {
                    std::swap(*iter, *prev);
                    isSorted = false;
                }

                iter--;
                prev--;
            }

            iterationBegin = std::next(iter); // <=> iteraionBegin++;
        }
        if(isSorted) break;
    
        isSorted = false;
        direction = !direction;
    }
}

using StepT = uint32_t;

template <typename IteratorT, typename CompT>
    requires
    std::random_access_iterator<IteratorT> &&
    std::strict_weak_order<CompT, std::iter_value_t<IteratorT>, std::iter_value_t<IteratorT>>
    
void CombSort(IteratorT begin, IteratorT end, CompT comp)
{
    auto GetNextStep = [](StepT step) -> StepT { return step * 4 / 5; };
    StepT step = std::distance(begin, end);

    bool isAnySwap = false;

    while(step > 1 || isAnySwap)
    {
        if(step > 1)
        {
            step = GetNextStep(step);
        }
        
        auto leftIter = begin;
        auto rightIter = begin + step;

        isAnySwap = false;

        while(rightIter < end)
        {
            if(comp(*rightIter, *leftIter) == true)
            {
                std::swap(*rightIter, *leftIter);
                isAnySwap = true;
            }

            leftIter++;
            rightIter++;
        }
    }
}

namespace Detail
{
using DigitT = uint16_t;
using BaseT = DigitT;

template <BaseT base = 10, typename NumberT>
constexpr uint16_t MaxDigitCount()
{
    uint64_t max = uint64_t(std::numeric_limits<NumberT>::max()) + 1; // = 2^N
    uint16_t digits = 0;
    while (max > 0) { max /= base; ++digits; }
    return digits;
}

template <BaseT base = 10, typename NumberT>
DigitT GetDigit(NumberT num, uint16_t rank)
{
    NumberT divResult = num;

    while(rank > 0)
    {
        if(divResult == 0) return 0;

        divResult = divResult / base;
        rank--;
    }

    return static_cast<DigitT>(divResult % base);
}
}

template <typename T>
concept UnsignedInteger =
    std::same_as<T, uint8_t>  ||
    std::same_as<T, uint16_t> ||
    std::same_as<T, uint32_t> ||
    std::same_as<T, uint64_t>;


#warning RadixSort is implemented using std::vector
template <Detail::BaseT base = 10, typename UIntIteratorT>
    requires
    std::random_access_iterator<UIntIteratorT> &&
    UnsignedInteger<std::iter_value_t<UIntIteratorT>>

void RadixSort(UIntIteratorT begin, UIntIteratorT end)
{
    // resolved during compile time
    using ValT = std::iter_value_t<UIntIteratorT>;
    constexpr uint16_t digits = Detail::MaxDigitCount<base, ValT>();  
    //
    
    std::vector <std::vector<ValT>> radixVector;
    radixVector.resize(base);

    auto size = std::distance(begin, end);

    std::vector <ValT> radixBuffer;
    radixBuffer.reserve(size);

    auto cpyIter = begin;
    auto cpyEnd = end;
    while(cpyIter != cpyEnd)
    {
        radixBuffer.push_back(*cpyIter);
        cpyIter++;
    }

    // сразу аллоцирую всю память, если ее нужно не так много
    if constexpr (digits * base < 9999)
        for(auto& vec : radixVector) vec.reserve(size);

    for(uint16_t rank = 0; rank < digits; rank++)
    {
        for(decltype(size) offset = 0; offset < size; offset++)
        {
            radixVector[Detail::GetDigit<base>(radixBuffer[offset], rank)].push_back(radixBuffer[offset]);
        }
        
        radixBuffer.clear();

        for(auto& vec : radixVector)
        {
            for(auto& num : vec)
            {
                radixBuffer.push_back(num);
            }
            vec.clear();
        }
    }

    auto sortedIter = radixBuffer.begin();
    auto unsortedIter = begin;
    while(sortedIter != radixBuffer.end())
    {
        *unsortedIter = *sortedIter; 

        sortedIter++;
        unsortedIter++;
    }
}

}