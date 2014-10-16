// numeric_limits example
#include <iostream>     // std::cout
#include <limits>       // std::numeric_limits

int main () {
  std::cout << std::boolalpha;
  
  std::cout << "Minimum value for unsigned: " << std::numeric_limits<unsigned>::min() << '\n';
  std::cout << "Maximum value for unsigned: " << std::numeric_limits<unsigned>::max() << '\n';
//  std::cout << "double is signed: " << std::numeric_limits<double>::is_signed << '\n';
  std::cout << "unsigned has infinity: " << std::numeric_limits<unsigned>::has_infinity << '\n';
  
  unsigned minus = -1;
  std::cout << "unsigned -1 is " << minus << "\n";
  
  return 0;
}
