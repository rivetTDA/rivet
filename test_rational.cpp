#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <set>


#include <boost/multiprecision/cpp_int.hpp>
//#include <boost/math/constants/constants.hpp>

typedef boost::multiprecision::cpp_rational rational;

//finding a rational approximation of a floating-point
// precondition: x > 0
rational approx(double x)
{
    int d = 3;	//desired significant digits
    int log = (int) floor( log10(x) ) + 1;
    
    if(log >= d)
    	return rational( floor(x) );
    
    int denom = pow(10, d-log);
    return rational( (int) floor(x*denom), denom);
}


int main(int argc, char* argv[])
{	
    rational a = 1;
	
//	std::cout << "high precision digits: " << std::numeric_limits<highprecision>::digits << "\n\n";
//	std::cout << std::setprecision(std::numeric_limits<highprecision>::max_digits10) << "1 stored in a: " << a << "\n\n";
	
/*    std::cout << "1 stored in a: " << a << "\n";

    for(unsigned i=1; i<=100; i++)
        a *= i;

    std::cout << "100! = " << a << "\n";

    a /= 10;

    std::cout << "100!/10 = " << a << "\n";
    std::cout << "  numerator: " << numerator(a) << "\n";
    std::cout << "  denominator: " << denominator(a) << "\n";
*/
    rational b(1, 3);
    std::cout << "1/3: " << b << "\n";
    bool nonzero = b > 0;
    std::cout << "1/3 greater than zero: " << nonzero << "\n";

    rational c("2/4");
    std::cout << "2/4: " << c << "\n";
    
    //testing conversion of (finite) decimal string to rational
    std::string str("20.625");		//WARNING: STRING NOT ALLOWED TO CONTAIN WHITE SPACE!!!
    int dec = str.find(".");

    rational r;
    
    if(dec == std::string::npos)	//then decimal point not found
    {
    	r = rational(str);
    	std::cout << "r = " << r << "\n";
    }
    else	//then decimal point found
    {
      	std::string whole = str.substr(0,dec);
   	std::string frac = str.substr(dec+1);
    	unsigned exp = frac.length();
    
    	std::istringstream s(whole + frac);
    	int num;
    	s >> num;
    	int denom = pow(10,exp);
    	r = rational(num, denom);
    	std::cout << "r = " << r << "\n";
    }
    
    //finding a rational approximation of a floating-point
    std::vector<double> nums;
    nums.push_back(12345.8);
    nums.push_back(2233.1234);
    nums.push_back(.12345678);
    nums.push_back(12.345678);
    nums.push_back(12345.678);
    nums.push_back(12);
    nums.push_back(0.000047);
    nums.push_back(0.0003355224);
    
    for(unsigned i=0; i<nums.size(); i++)
    {
    	std::cout << "approximation of " << nums[i] << ": " << approx(nums[i]) << "\n";
    }
    
    

}
