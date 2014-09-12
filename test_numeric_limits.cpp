// numeric_limits example
#include <iostream>     // std::cout
#include <limits>       // std::numeric_limits

int main () {
  std::cout << std::boolalpha;
  
  std::cout << "Minimum value for double: " << std::numeric_limits<double>::min() << '\n';
  std::cout << "Maximum value for double: " << std::numeric_limits<double>::max() << '\n';
  std::cout << "double is signed: " << std::numeric_limits<double>::is_signed << '\n';
  std::cout << "double has infinity: " << std::numeric_limits<double>::has_infinity << '\n';
  
  return 0;
}
