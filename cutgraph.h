/**********************************************************************
Copyright 2014-2016 The RIVET Devlopers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

// NOTE:  this file will eventually contain more functions necessary for the graph cutting procedures that are in progress
// for now, it just contains the adjacency list sorting functions

#include <stack>
#include <utility> // for make_pair()
#include <vector>

// function to sort the children of each node by the weight of the subtree of which each child is the root
// input:
//    adjList: list of all adjacencies in the tree; adjList[a] = b iff nodes a and b are adjacent
//    distances: list of all distances between adjacent nodes in the tree
//    start: index of the node in the tree that will be regarded as root
// output: children[i] is a vector of indexes of the children of node i, in decreasing order of branch weight
//    (branch weight is total weight of all edges below a given node, plus weight of edge to parent node)
void sortAdjacencies(std::vector<std::vector<unsigned>>& adjList, std::vector<std::vector<unsigned>>& distances,
    unsigned start, std::vector<std::vector<unsigned>>& children)
{
    bool* discovered = new bool[adjList.size()]; // boolean array for keeping track of which nodes have been visited
    unsigned* branchWeight = new unsigned[adjList.size()]; // this will contain the weight of the edges "hanging" from the node represented by its index in branchWeight
    // populate the boolean array with false and the branchWeight array with 0
    for (unsigned i = 0; i < adjList.size(); ++i) {
        discovered[i] = false;
        branchWeight[i] = 0;
    }

    std::stack<unsigned> nodes; // stack for nodes as we do DFS
    nodes.push(start); // push start node onto the node stack
    discovered[start] = true; // mark start node as discovered
    std::vector<std::pair<unsigned, unsigned>> toBeSorted; // vector of pairs to contain the children of a given node

    while (!nodes.empty()) // while we have not traversed the whole tree
    {
        unsigned node = nodes.top(); // the current node that we are considering

        // find the next undiscovered child of node
        bool found_new_child = false;
        for (unsigned i = 0; i < adjList[node].size(); ++i) // look for an undiscovered node
        {
            if (!discovered[adjList[node][i]]) // found a node
            {
                discovered[adjList[node][i]] = true; // discover the next node
                nodes.push(adjList[node][i]); // push the next node onto the stack
                found_new_child = true;
                break;
            }
        }

        if (!found_new_child)
        // we have found all of node's children, so we can sort them and compute branch weight for node
        {
            nodes.pop(); // pop node off of the node stack

            unsigned running_sum = 0; // reset runningSum
            toBeSorted.clear(); // reset toBeSorted

            for (unsigned i = 0; i < adjList[node].size(); i++) // loop over all children of node
            {
                if (!nodes.empty() && nodes.top() == adjList[node][i]) // then this adjacency is the parent node
                    continue;

                //add this child to the toBeSorted vector
                unsigned child = adjList[node][i];
                unsigned cur_branch_weight = branchWeight[child] + distances[node][child];
                toBeSorted.push_back(std::make_pair(cur_branch_weight, child));

                //add weight of this child's branch to runningSum
                running_sum += cur_branch_weight;
            }

            branchWeight[node] = running_sum; // assign running_sum to branchWeight at the current node

            //sort the children of current node (sorts in increasing order by branch weight)
            std::sort(toBeSorted.begin(), toBeSorted.end());

            // copy the children indexes to the children vector in reverse branch-weight order
            for (std::vector<std::pair<unsigned, unsigned>>::reverse_iterator rit = toBeSorted.rbegin();
                 rit != toBeSorted.rend(); ++rit) {
                children[node].push_back(rit->second);
            }
        }
    } // end while
    delete[] branchWeight;
    delete[] discovered;
} // end sortAdjacencies()
