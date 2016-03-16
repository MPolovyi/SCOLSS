//
// Created by mpolovyi on 10/03/16.
//

#ifndef PROJECT_CMYEXCEPTION_H
#define PROJECT_CMYEXCEPTION_H

#include <string>

class MyException : public std::exception {
    std::string s;
public:
    MyException(std::string _s) : s(_s){}
    ~MyException() throw () {}
    const char* what() const throw() { return s.c_str(); }
};

#endif //PROJECT_CMYEXCEPTION_H
