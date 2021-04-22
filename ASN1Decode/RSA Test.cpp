#include <iostream>
#include <iomanip>
#include "BigUInt/BigUInt.hpp"
#include <math.h>

using namespace std;

void RSATest(const uint8_t* ab, const uint8_t* eb, const uint8_t* mb)
{
    BigUInt a, e, m;
    //a = 787878787878787878;
    //e = 345643456;
    //m = 35644564654645646;

    
    a.loadFromByteReversArray(ab, 2048 / 8);
    e.loadFromByteReversArray(eb, 3);
    m.loadFromByteReversArray(mb, 257);

    cout << "a: " << a << endl;
    cout << "e: " << e << endl;
    cout << "m: " << m << endl;

    //cout << std::hex;
    //cout << "R0: " << metod(123456789, 65537, 67) << endl;
    BigUInt message = a.modPow(e, m);
    cout << "R1: " << message << endl;

    //BigUInt sha256;
    //sha256 = 256;
    //cout << "sha256: " << sha256.pow(2048/8) << endl;
} 