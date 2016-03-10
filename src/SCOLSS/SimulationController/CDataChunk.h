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
    DataClass *m_Data;
    size_t m_size;

    bool doCleanUp;
    DataClass* linkToNext;
    DataClass* linkToPrev;
public:

    int ProcId;

    size_t size() const { return m_size; };
    size_t size_in_bytes() const { return DataSizeInBytes * m_size; }

    size_t size_of_data() const { return DataSizeInBytes; }

    CDataChunk() { doCleanUp = false; }

    void Init(DataClass* data, size_t size, int procId,
              DataClass* linkToNext = nullptr,
              DataClass* linkToPrev = nullptr)
    {
        m_Data = data;
        m_size = size;

        doCleanUp = false;

        ProcId = procId;

        if (linkToNext == nullptr){linkToNext = end();}
        if (linkToPrev == nullptr){linkToPrev = begin() - 1;}
    }

    void Init(size_t size, int procId){
        ProcId = procId;
        m_size = size;
        m_Data = new DataClass[size];

        doCleanUp = true;
    }

    ~CDataChunk(){
        if (doCleanUp)
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

    DataClass* linkNext() {

    }

    DataClass* linkPrev() {

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

    size_t firstIndex(){
        return ProcId*m_size;
    }
    size_t endIndex(){
        return (ProcId + 1)*m_size;
    }
};


#endif //PROJECT_CDATACHUNK_H
