#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <set>


#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/math/constants/constants.hpp>

typedef boost::multiprecision::cpp_dec_float_100 highprecision;


int main(int argc, char* argv[])
{	
	highprecision a = 1;
	
	std::cout << "high precision digits: " << std::numeric_limits<highprecision>::digits << "\n\n";
	
	
	
	std::cout << std::setprecision(std::numeric_limits<highprecision>::max_digits10) << "1 stored in a: " << a << "\n\n";
	
	
	highprecision b = 3;
	highprecision c = a/b;
	
	std::cout << "highprecision integer division 1/3: " << c << "\n\n";
	
	double t = 1.3;
	highprecision h = t;
	std::cout << "highprecision 1.3 from a double: " << h << "\n\n";
	
	highprecision d (highprecision(13)/10);
	std::cout << "highprecision 1.3 from highprecision integer division: " << d << "\n\n";
	
	highprecision e("1.3");
	std::cout << "highprecision 1.3 from a string: " << e << "\n\n";
	
	std::cout << "COMPARING highprecision AND double:\n";
	std::cout << "   highprecision 1 > double 0 : " << (a > 0) << "\n";
	std::cout << "   highprecision 1.3 > double 1.3 : " << (d > 1.3) << "\n\n";
	
	std::cout << "SQUARE ROOT 2:\n";
	highprecision z = 2;
	std::cout << sqrt(z) << "\n";
	//std::cout << "   Boost math: " << boost::math::sqrt(z) << "\n";
	
	std::cout << "TESTING EPSILON:\n";
	//highprecision ten = 10;
	highprecision big = pow(highprecision(10), 90);
	std::cout << "  big: " << big << "\n";
	highprecision epsilon = 1/pow(highprecision(10), 90);
	std::cout << "  epsilon: " << epsilon << "\n";
	highprecision neg = -1*epsilon;
	std::cout << "  negative epsilon: " << neg << ", " << abs(neg) << "\n\n";
	
	std::cout << "TESTING PI:\n";
	highprecision pi = boost::math::constants::pi<highprecision>();
	std::cout << "PI: " << pi << "\n";
	std::cout << "HALF_PI: " << pi/2 << "\n\n";
	
}
