#include <iostream>
#include <vector>

#include "st_node.h"


using namespace std;


// RECURSIVELY PRINT TREE
void print_subtree(STNode &node, int indent)
{
	//print current node
	for(int i=0; i<indent; i++)
		std::cout << "  ";
	node.print();
	
	//print children nodes
	vector<STNode*> kids = node.get_children();
	for(int i=0; i<kids.size(); i++)
		print_subtree(*kids[i], indent+1);
}




// TESTING SIMPLEX TREE
int main()
{	
	cout << "Testing simplex tree:\n";
	
	//create root
	STNode root;	//calls empty constructor
	
	//create level 1 children
	STNode n1(1, &root, 2.2, 3.3);
	STNode n2(2, &root, 2.5, 4.78);
	STNode n3(3, &root, 3.6, 2.0);
	
	//create level 2 children
	STNode n11(11, &n1, 2.22, 3.33);
	STNode n12(12, &n1, 5.2, 9.0);
	STNode n21(21, &n2, 4.4, 9.2);
	STNode n211(211, &n21, 5.4, 2.6);
	
	
	//print entire tree
	print_subtree(root,0);	
	
	
	//print node data
	/*cout << "Root node stored at " << &root << ": ";
	root.print();
	
	cout << "Child 1 stored at " << &n1 << ": ";
	n1.print();
	
	cout << "Child 2 stored at " << &n2 << ": ";
	n2.print();
	
	cout << "Child 11 stored at " << &n11 << ": ";
	n11.print();
	
	
	//test traversal
	cout << "\nTESTING TRAVERSAL\n";
	//STNode p = n11.get_parent();
	cout << "  Parent of " << n11.get_vertex() << " has vertex " << n11.get_parent().get_vertex() << ".\n";
	cout << "  Grandparent of " << n11.get_vertex() << " has vertex " << n11.get_parent().get_parent().get_vertex() << ".\n";
	*/
	
	//done
	cout << "\n";
}


