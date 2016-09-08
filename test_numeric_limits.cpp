// numeric_limits example
#include <iostream> // std::cout
#include <limits> // std::numeric_limits

int main()
{
    std::cout << std::boolalpha;

    std::cout << "Minimum value for unsigned: " << std::numeric_limits<unsigned>::min() << '\n';
    std::cout << "Maximum value for unsigned: " << std::numeric_limits<unsigned>::max() << '\n';
    //  std::cout << "double is signed: " << std::numeric_limits<double>::is_signed << '\n';
    std::cout << "unsigned has infinity: " << std::numeric_limits<unsigned>::has_infinity << '\n';

    unsigned minus = -1;
    std::cout << "unsigned -1 is " << minus << "\n";

    minus++;
    std::cout << "  0: " << minus << "\n";
    minus++;
    std::cout << "  1: " << minus << "\n";
    minus -= 3;
    std::cout << "  -2: " << minus << "\n";

    std::cout << "Maximum value for int: " << std::numeric_limits<int>::max() << '\n';
    std::cout << "Maximum value for long int: " << std::numeric_limits<long int>::max() << '\n';
    std::cout << "Maximum value for unsigned long: " << std::numeric_limits<unsigned long>::max() << '\n';

    int a = 100000000;
    int b = 200000000;
    int prod = a * b;
    std::cout << "int a = " << a << "; b = " << b << "; ab = " << prod << "\n";
    unsigned long lprod = static_cast<unsigned long>(a) * static_cast<unsigned long>(b); // (long int) a*(long int) b;
    std::cout << "long product " << lprod << "\n";

    return 0;
}
