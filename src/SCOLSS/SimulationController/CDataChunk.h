//
// Created by mpolovyi on 10/03/16.
//

#ifndef PROJECT_CDATACHUNK_H
#define PROJECT_CDATACHUNK_H

#include <utility>
#include <stdlib.h>

#include <SCOLSS/ExecFile/CMyException.h>

template <class DataClass, size_t DataSizeInBytes = sizeof(DataClass)>
class CDataChunk {
    DataClass* m_Data;
    size_t m_size;

    bool m_doCleanUp;
    DataClass* m_linkToNext;
    DataClass* m_linkToPrev;
public:

    int ProcId;

    size_t size() const { return m_size; };
    size_t size_in_bytes() const { return DataSizeInBytes * m_size; }

    size_t size_of_data() const { return DataSizeInBytes; }

    CDataChunk() { m_doCleanUp = false; }

    void Init(DataClass* data, size_t size, int procId,
              DataClass* linkToNext = nullptr,
              DataClass* linkToPrev = nullptr)
    {
        m_Data = data;
        m_size = size;

        m_doCleanUp = false;

        ProcId = procId;

        if (linkToNext == nullptr){ m_linkToNext = end(); } else { m_linkToNext = linkToNext; }
        if (linkToPrev == nullptr){ m_linkToPrev = begin() - 1; } else { m_linkToPrev = linkToPrev; }
    }

    void Init(size_t size, int procId){
        ProcId = procId;
        m_size = size;
        m_Data = new DataClass[size];

        m_doCleanUp = true;
    }

    ~CDataChunk(){
        if (m_doCleanUp)
            delete[] m_Data;
    }

    DataClass& operator[](size_t index){
        if (index < size())
            return m_Data[index];
        else
            throw MyException("Index is bigger than chunk size");
    }

    DataClass* operator&(){
        return m_Data;
    }

    DataClass* linkToNext() {
        return m_linkToNext;
    }

    DataClass* linkToPrev() {
        return m_linkToPrev;
    }

    DataClass* last(){
        return m_Data + size() - 1;
    }

    DataClass* begin(){
        return m_Data;
    }
    DataClass* end(){
        return m_Data + size();
    }

    size_t beginIndex(){
        return ProcId*m_size;
    }
    size_t endIndex(){
        return (ProcId + 1)*m_size;
    }
};


#endif //PROJECT_CDATACHUNK_H
