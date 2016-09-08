#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;

struct Face {
    int a;
    int b;
};

int main(int argc, char* argv[])
{
    Face* face1 = new Face();
    Face* face2 = new Face();
    Face* face3 = new Face();

    std::cout << "Size of pointer: " << sizeof(void*) << "; size of unsigned: " << sizeof(unsigned) << "\n";

    std::cout << "Pointers:" << face1 << ", " << face2 << "\n";
    std::cout << "Long:" << (unsigned long)face1 << ", " << (unsigned long)face2 << "\n";

    Graph g;

    boost::add_edge(1, 2, g);
    boost::add_edge(1, 3, g);

    //print the edge set
    typedef boost::graph_traits<Graph>::edge_iterator edge_iterator;
    std::pair<edge_iterator, edge_iterator> ei = boost::edges(g);

    std::cout << "Edges: \n";
    for (edge_iterator it = ei.first; it != ei.second; ++it) {
        std::cout << "  (" << boost::source(*it, g) << ", " << boost::target(*it, g) << ")\n";
    }

    return 0;
}
