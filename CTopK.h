//
//  CTopK.h
//  PingCAP
//
//  Created by john-y on 2019/3/3.
//  Copyright © 2019 john-y. All rights reserved.
//
#include <iostream>
#include <string>
#include <unordered_map>

#ifndef CTopK_h
#define CTopK_h
template<class T>
class CTopK
{
public:
    CTopK();
    ~CTopK();
    T*  m_Data;
    int GetTopK(std::unordered_map<std::string, int>& unorder_map, int& nTop);
private:
    void Clear();
    void HeapAdjust(int nStart, int nLen);
    void BuildHeap(int nLen);
    bool CompareGreater(const T &v1, const T &v2);
};

template <class T>
bool CTopK<T>::CompareGreater(const T &v1, const T &v2) {
    return v1->second > v2->second;
}

template<class T>
CTopK<T>::CTopK() {
    m_Data = NULL;
}

template<class T>
CTopK<T>::~CTopK() {
    Clear();
}

template<class T>
void CTopK<T>::Clear() {
    if (NULL != m_Data) {
        delete[] m_Data;
        m_Data = NULL;
    }
}

// get top k
template<class T>
int CTopK<T>::GetTopK(std::unordered_map<std::string, int>& unorder_map, int& nTop) {
    int i = 0;
    
    // parameter check
    if (nTop <= 0)
    {
        return -1;
    }
    
    // clear
    Clear();
    
    // alloc space
    m_Data = new T[nTop];
    if (NULL == m_Data)
    {
        return -1;
    }
    
    //read topk date
    std::unordered_map<std::string, int>::iterator unorder_map_it = unorder_map.begin();
    for (i = 0; i < nTop; i++) {
        if (unorder_map_it != unorder_map.end()) {
            m_Data[i] = unorder_map_it++;
        }
        else {
            break;
        }
    }
    
    // deal i <  nTop
    if (i < nTop) {
        nTop = i;
    } else {
        BuildHeap(nTop);
        
        while (unorder_map_it != unorder_map.end()) {
            if (CompareGreater(unorder_map_it, m_Data[0])) {
                //swap and adjust heap
                m_Data[0] = unorder_map_it;
                HeapAdjust(0, nTop);
            }
            ++unorder_map_it;
        }
    }
    
    return 0;
}

//adjust minimum heap
template<class T>
void CTopK<T>::HeapAdjust(int nStart, int nLen) {
    int nMinChild = 0;
    T fTemp;
    
    while ((2 * nStart + 1) < nLen) {
        nMinChild = 2 * nStart + 1;
        if ( (2 * nStart + 2) < nLen) {
            //compare left and right node, record nMinChild Index
            if (CompareGreater(m_Data[2 * nStart + 1], m_Data[2 * nStart + 2])) {
                nMinChild = 2 * nStart + 2;
            }
        }
        
        //change data
        if (CompareGreater(m_Data[nStart], m_Data[nMinChild])) {
            //swap nStart and nMaxChild data
            fTemp = m_Data[nStart];
            m_Data[nStart] = m_Data[nMinChild];
            m_Data[nMinChild] = fTemp;
            
            //heap destroyed and continue adjust
            nStart = nMinChild;
        } else {
            // not adjust
            break;
        }
    }
}

// build heap
template<class T>
void CTopK<T>::BuildHeap(int nLen) {
    int i = 0;
    //m_Data[0, Len-1] is a min heap, not order
    for (i = nLen / 2  - 1; i >= 0; i--) {
        HeapAdjust(i, nLen);
    }
}
#endif /* CTopK_h */
