#include "input_manager.h"
#include "simplex_tree.h"

int main(int argc, char* argv[])
{
	std::cout << "Testing InputManager class\n";
	
	
	//check for name of data file
	if(argc == 1)
	{
		std::cout << "USAGE: run <filename>\n";
		return 1;
	}
	
	//start the input manager
	InputManager im;
	im.start(argv[1]);
	
	
	
	//print simplex tree
	std::cout << "TESTING SIMPLEX TREE:\n";
	SimplexTree* bifil = im.get_bifiltration();
	(*bifil).print();	
	
	std::cout << "Done.\n\n";
}
