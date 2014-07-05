#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <set>

#include "interface/input_manager.h"
#include "math/st_node.h"
#include "math/simplex_tree.h"
#include "math/map_matrix.h"
#include "math/multi_betti.h"


// RECURSIVELY PRINT TREE
void print_subtree(STNode &node, int indent)
{
	//print current node
	for(int i=0; i<indent; i++)
		std::cout << "  ";
	node.print();
	
	//print children nodes
    std::vector<STNode*> kids = node.get_children();
	for(int i=0; i<kids.size(); i++)
		print_subtree(*kids[i], indent+1);
}




// TESTING SIMPLEX TREE
int main(int argc, char* argv[])
{	
    //check for name of data file
    if(argc == 1)
    {
        std::cout << "USAGE: run <filename> [dimension of homology]\n";
        return 1;
    }

    //set dimension of homology
    int dim = 1;		//default
    if(argc >= 3)
        dim = std::atoi(argv[2]);
    std::cout << "Homology dimension set to" << dim << ".\n";

    //start the input manager
    int verbosity = 8;
    InputManager im(dim, verbosity);
    im.start(argv[1]);

    //get the bifiltration from the input manager
    SimplexTree* bifiltration = im.get_bifiltration();

    //test dimension indexes
    bifiltration->update_dim_indexes();

    //print simplex tree
    if(verbosity >= 2)
    {
        std::cout << "SIMPLEX TREE:\n";
        bifiltration->print();
    }


    //get index matrices
    IndexMatrix* index_dim = bifiltration->get_index_mx(dim);
    std::cout << "INDEX MATRIX FOR DIMENSION " << dim << ":\n";
    index_dim->print();

    IndexMatrix* index_high = bifiltration->get_index_mx(dim+1);
    std::cout << "INDEX MATRIX FOR DIMENSION " << (dim+1) << ":\n";
    index_high->print();

    //get boundary matrices
    MapMatrix* boundary1 = bifiltration->get_boundary_mx(dim);
    std::cout << "BOUNDARY MATRIX FOR DIMENSION " << dim << ":\n";
    boundary1->print();

//    MapMatrix* boundary2 = bifiltration->get_boundary_mx(dim+1);
//    std::cout << "BOUNDARY MATRIX FOR DIMENSION " << (dim+1) << ":\n";
//    boundary2->print();

//    //get merge matrix
//    MapMatrix* merge = bifiltration->get_merge_mx();
//    std::cout << "MERGE MATRIX:\n";
//    merge->print();

//    //get split matrix
//    MapMatrix* split = bifiltration->get_split_mx();
//    std::cout << "SPLIT MATRIX:\n";
//    split->print();


    //do column reduction
    MultiBetti mb(bifiltration, dim, verbosity);
    mb.compute_fast();


    //print
    std::cout << "COMPUTATION FINISHED:\n";

    //build column labels for output
    std::string col_labels = "         x = ";
    std::string hline = "    --------";
    for(int j=0; j<bifiltration->num_x_grades(); j++)
    {
        std::ostringstream oss;
        oss << j;
        col_labels += oss.str() + "  ";
        hline += "---";
    }
    col_labels = hline + "\n" + col_labels + "\n";

    //output xi_0
    std::cout << "  VALUES OF xi_0 for dimension " << dim << ":\n";
    for(int i=bifiltration->num_y_grades()-1; i>=0; i--)
    {
        std::cout << "     y = " << i << " | ";
        for(int j=0; j<bifiltration->num_x_grades(); j++)
        {
            std::cout << mb.xi0(j,i) << "  ";
        }
        std::cout << "\n";
    }
    std::cout << col_labels;

    //output xi_1
    std::cout << "\n  VALUES OF xi_1 for dimension " << dim << ":\n";
    for(int i=bifiltration->num_y_grades()-1; i>=0; i--)
    {
        std::cout << "     y = " << i << " | ";
        for(int j=0; j<bifiltration->num_x_grades(); j++)
        {
            std::cout << mb.xi1(j,i) << "  ";
        }
        std::cout << "\n";
    }
    std::cout << col_labels << "\n";


}

/* OLD TEST CODE
//create root
STNode root;	//calls empty constructor

//create level 1 children
STNode n1(1, &root, 2, 3, -1);
root.append_child(&n1);
STNode n2(2, &root, 2, 4, -1);
root.append_child(&n2);
STNode n3(3, &root, 3, 2, -1);
root.append_child(&n3);

//create level 2 children
STNode n11(11, &n1, 2, 3, -1);
n1.append_child(&n11);
STNode n12(12, &n1, 5, 9, -1);
n1.append_child(&n12);
STNode n21(21, &n2, 4, 9, -1);
n2.append_child(&n21);
n21.add_child(211, 5, 2);
//    STNode n211(211, &n21, 5, 2, -1);


//print entire tree
print_subtree(root,0);


//print node data


//test traversal
std::cout << "\nTESTING TRAVERSAL\n";
//STNode p = n11.get_parent();
std::cout << "  Parent of " << n11.get_vertex() << " has vertex " << n11.get_parent().get_vertex() << ".\n";
std::cout << "  Grandparent of " << n11.get_vertex() << " has vertex " << n11.get_parent().get_parent().get_vertex() << ".\n";


//done
std::cout << "\n";
*/
