//test program
#include <iostream>
#include <fstream>
#include "point.h"

using namespace std;

int main ()
{
	//testing file read
	cout << "TESTING FILE:\n";
	string line;
	ifstream myfile("data/sample1.txt");
	if(myfile.is_open())
	{
		while( getline(myfile,line) )
		{
			cout << line << '\n';
		}
		myfile.close();
		
	}
	else
	{
		cout << "Unable to open file";
	}
	
	
	//testing point class
	cout << "TESTING POINT:\n";
	double n[2] = {1.2, 4};
	Point p (n, 8.3);
	p.pp();
	
	double *m = p.get_coords();
	cout << "in main, m: " << m[0] << ", " << m[1] << "\n";
	cout << "in main, birth: " << p.get_birth() << "\n";
}
