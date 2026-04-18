#include <iostream>
#include "BigIntMath.h"
#include "PrimeGenerator.h" // Обязательно подключаем новый файл

using namespace std;
using namespace boost::multiprecision;

int main() {
    cout << "--- Testing BigIntMath ---" << endl;
    cpp_int a = 48, b = 18;
    cout << "GCD(" << a << ", " << b << ") = " << BigIntMath::gcd(a, b) << " (Expected: 6)" << endl;
    
    cpp_int base = 2, exp = 10, mod = 1000;
    cout << base << "^" << exp << " mod " << mod << " = " << BigIntMath::modExp(base, exp, mod) << " (Expected: 24)" << endl;

    cpp_int e = 3, phi = 11;
    cout << "Modular inverse of " << e << " mod " << phi << " = " << BigIntMath::modInverse(e, phi) << " (Expected: 4)" << endl;
    cout << "-------------------------------" << endl;

    // ТЕСТИРУЕМ ГЕНЕРАЦИЮ ПРОСТЫХ ЧИСЕЛ
    cout << "\n--- Testing Prime Generation ---" << endl;
    cout << "Generating a 512-bit prime number. Please wait..." << endl;
    
    cpp_int p = PrimeGenerator::generatePrime(512);
    
    cout << "Generated Prime (p): " << endl;
    cout << p << endl;
    cout << "-------------------------------" << endl;

    return 0;
}