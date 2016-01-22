//
// Created by mpolovyi on 22/01/16.
//

#include <iostream>
#include <SCOLSS/MathLibrary/CVector.h>

int main(int argc, char** argv){
    CVector a(1, 2, 3);

    CVector b = a + CVector(1, 2, 3);

    std::cout << b << std::endl;

    std::cout << "Hello world!";
}