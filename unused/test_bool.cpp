#include "f2_field.h"
#include <iostream>

using namespace std;

int main()
{
    cout << "Testing F2 arithmetic\n\n";

    F2 t(true);
    F2 f, a;

    cout << "1: " << t << "\n";
    cout << "0: " << f << "\n";

    cout << "Addition:\n";

    a = t + t;
    cout << "  1+1: " << a << "\n";

    a = f + f;
    cout << "  0+0: " << a << "\n";

    a = t + f;
    cout << "  1+0: " << a << "\n";

    a = f + t;
    cout << "  0+1: " << a << "\n";

    cout << "Subtraction:\n";

    a = t - t;
    cout << "  1-1: " << a << "\n";

    a = f - f;
    cout << "  0-0: " << a << "\n";

    a = t - f;
    cout << "  1-0: " << a << "\n";

    a = f - t;
    cout << "  0-1: " << a << "\n";

    cout << "Multiplication:\n";

    a = t * t;
    cout << "  1*1: " << a << "\n";

    a = f * f;
    cout << "  0*0: " << a << "\n";

    a = t * f;
    cout << "  1*0: " << a << "\n";

    a = f * t;
    cout << "  0*1: " << a << "\n";

    cout << "Division:\n";

    a = t / t;
    cout << "  1/1: " << a << "\n";

    a = f / t;
    cout << "  0/1: " << a << "\n";

    cout << "Comparison:\n";

    bool b = (t == t);
    cout << "  1==1: " << b << "\n";

    b = (f == f);
    cout << "  0==0: " << b << "\n";

    b = (t == f);
    cout << "  1==0: " << b << "\n";

    b = (f == t);
    cout << "  0==1: " << b << "\n";

    b = (f != f);
    cout << "  0!=0: " << b << "\n";

    b = (t != f);
    cout << "  1!=0: " << b << "\n";

    b = (t >= t);
    cout << "  1>=1: " << b << "\n";

    b = (f >= t);
    cout << "  0>=1: " << b << "\n";

    b = (f > f);
    cout << "  0>0: " << b << "\n";

    b = (t > f);
    cout << "  1>0: " << b << "\n";
}
