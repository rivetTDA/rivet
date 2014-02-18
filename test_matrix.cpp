#include <iostream>
#include "map_matrix.h"

using namespace std;

int main()
{
	cout << "Testing MapMatrix class\n";
	
	MapMatrix m(9,4);
	
	//cout << "  entry (4,3): " << m.entry(4,3) << "\n";
	
	cout << "  setting entry (2,3)\n";
	m.set(2,3);
	
	cout << "  setting entry (8,3)\n";
	m.set(8,3);
	
	cout << "  setting entry (4,3)\n";
	m.set(4,3);
	
	cout << "  setting entry (1,3)\n";
	m.set(1,3);
	
	cout << "  setting entry (1,3)\n";
	m.set(1,3);	
	
	cout << "  setting entry (7,3)\n";
	m.set(7,3);
	
	cout << "  setting entry (2,3)\n";
	m.set(2,3);
	
	cout << "  setting entry (4,3)\n";
	m.set(4,3);	
	
	cout << "  setting entry (2,2)\n";
	m.set(2,2);
	
	cout << "  setting entry (3,2)\n";
	m.set(3,2);
	
	cout << "  setting entry (4,2)\n";
	m.set(4,2);
	
	cout << "  setting entry (5,2)\n";
	m.set(5,2);
	
	m.print();
	
	cout << "  adding column 3 to column 4\n";
	m.add_column(3,4);
	
/*	cout << "  entry (1,3): " << m.entry(1,3) << "\n";
	cout << "  entry (2,3): " << m.entry(2,3) << "\n";
	cout << "  entry (3,3): " << m.entry(3,3) << "\n";
	cout << "  entry (4,3): " << m.entry(4,3) << "\n";
	cout << "  entry (5,3): " << m.entry(5,3) << "\n";
	cout << "  entry (6,3): " << m.entry(6,3) << "\n";
	cout << "  entry (7,3): " << m.entry(7,3) << "\n";
	cout << "  entry (8,3): " << m.entry(8,3) << "\n";
	cout << "  entry (9,3): " << m.entry(9,3) << "\n";
	cout << "  entry (1,4): " << m.entry(1,4) << "\n";
	cout << "  entry (2,4): " << m.entry(2,4) << "\n";
	cout << "  entry (3,4): " << m.entry(3,4) << "\n";
	cout << "  entry (4,4): " << m.entry(4,4) << "\n";
	cout << "  entry (5,4): " << m.entry(5,4) << "\n";
	cout << "  entry (6,4): " << m.entry(6,4) << "\n";
	cout << "  entry (7,4): " << m.entry(7,4) << "\n";
	cout << "  entry (8,4): " << m.entry(8,4) << "\n";
	cout << "  entry (9,4): " << m.entry(9,4) << "\n";
*/	
	cout << "  low(2): " << m.low(2) << "\n";
	cout << "  low(3): " << m.low(3) << "\n";
	cout << "  low(4): " << m.low(4) << "\n";
	cout << "  low(9): " << m.low(9) << "\n";
	
	m.print();
	
	cout << "  adding column 3 to column 2\n";
	m.add_column(3,2);
	
	m.print();
	
	cout << "  adding column 1 to column 2\n";
	m.add_column(1,2);
	
	m.print();
	
	cout << "  adding column 2 to column 1\n";
	m.add_column(2,1);
	
	m.print();
	
	cout << "  adding column 3 to column 4\n";
	m.add_column(3,4);
	
	m.print();
	
	cout << "  adding column 4 to column 3\n";
	m.add_column(4,3);
	
	m.print();
}
