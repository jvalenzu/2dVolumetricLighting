// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include <new>
#include <stdio.h>

template <typename T> T LinkyListNext(T itr)
{
    if (itr)
        return itr->m_Next;
    return nullptr;
}

template <typename T> T LinkyListPrev(T itr)
{
    return itr->m_Prev;
}

template <typename T> void LinkyListAdd(T& head, T itr)
{
    if (head == nullptr)
    {
        head = itr;
        return;
    }
    
    itr->m_Next = head;
    head->m_Prev = itr;
    head = itr;
}

template <typename T> void LinkyListRemove(T& head, T node)
{
    T itr = head;
    
    while (itr)
    {
        if (itr == node)
        {
            if (itr->m_Prev)
            {
                itr->m_Prev->m_Next = itr->m_Next;
                return;
            }
            
            head = head->m_Next;
            if (head)
                head->m_Prev = nullptr;
            return;
        }
        
        itr = itr->m_Next;
    }
}

template <typename T> int LinkyListCount(T head)
{
    if (head == nullptr)
        return 1;
    return 1 + LinkyListNext(head);
}

struct TestObj
{
    TestObj* m_Prev;
    TestObj* m_Next;
};
