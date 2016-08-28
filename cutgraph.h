// NOTE:  this file will eventually contain more functions necessary for the graph cutting procedures that are in progress
	// for now, it just contains the adjacency list sorting functions

#include <stack>
#include <vector>


// NOTE:  this function currently sorts in reverse order (most branching first, least branching last)
bool pairCompare(std::pair<unsigned, unsigned> left, std::pair<unsigned, unsigned> right)
{
    return left.first > right.first; // REVERSED sorting
}

void sortAdjacencies(std::vector<std::vector<unsigned> > &adjList, std::vector<std::vector<unsigned> > &distances, unsigned start)
{
	bool discovered[adjList.size()]; // boolean array for keeping track of which nodes have been visited
	unsigned branchWeight[adjList.size()]; // this will contain the weight of the edges "hanging" from the node represented by its index in branchWeight
	// populate the boolean array with false and the branchWeight array with 0
	for (int i = 0; i < adjList.size(); ++i)
	{
		discovered[i] = false;
		branchWeight[i] = 0;
	}
	std::stack<unsigned> nodes; // stack for nodes as we do DFS
    unsigned node = start, edgeIndex = 0, runningSum = 0;
    nodes.push(node); // push node onto the node stack
    discovered[node] = true; // mark node as discovered

    while (!nodes.empty()) // while we have not traversed the whole tree
    {
		node = nodes.top(); // let node be the current node that we are considering

		// find the next undiscovered node
		edgeIndex = adjList.at(node).size(); // set edgeIndex such that we will skip to the else statement if we don't find an undiscovered node
        // look for an undiscovered node
		for (int i = 0; i < adjList.at(node).size(); ++i)
		{
			if (!discovered[ adjList.at(node).at(i) ])
			{
				edgeIndex = i;
				break;
			}
		}

        if (edgeIndex < adjList.at(node).size()) // if we have not traversed all of node's children
        {
        	discovered[ adjList.at(node).at(edgeIndex) ] = true; // discover the next node
            nodes.push( adjList.at(node).at(edgeIndex) ); // push the next node onto the stack
        }

        else // we have traversed all of node's children
        {
            nodes.pop(); // pop node off of the node stack

            runningSum = 0; // reset runningSum
			for (int i = 0; i < adjList.at(node).size(); ++i)
			{
				runningSum += branchWeight[adjList.at(node).at(i)] + distances.at(node).at( adjList.at(node).at(i) );
			}

			if (!nodes.empty()) // if there is a parent node, we know that we overcounted the branchWeight by the weight of the edge between the current node and its parent
			{
				branchWeight[node] = runningSum - distances.at(node).at(nodes.top()); // assign runningSum to branchWeight at the current node
			}
			else // otherwise, we are at the root of the tree and we can assign runningSum to the branchWeight at node
			{
				branchWeight[node] = runningSum; // assign runningSum to branchWeight at the current node
			}
        }
    } // end while


	// TESTING:
	// qDebug() << "start = " << start;
	// qDebug() << "adjList = ";
	// for (int i = 0; i < adjList.size(); ++i)
	// {
	// 	qDebug() << "branchWeight.at(" << i << ") = " << branchWeight[i];
	// 	// qDebug().nospace() << i << " = ";
	// 	for (int j = 0; j < adjList.at(i).size(); ++j)
	// 	{
	// 		qDebug().nospace() << adjList.at(i).at(j) << "  =  " << distances.at(i).at( adjList.at(i).at(j) );
	// 	}
	// }

	// for (int i = 0; i < adjList.size(); ++i)
	// {
	// 	qDebug() << "branchWeight.at(" << i << ") = " << branchWeight[i];
	// }

    std::vector<std::pair<unsigned, unsigned> > toBeSorted; // vector of pairs to contain the children of a given node
    std::pair<unsigned, unsigned> newPair; // pair for containing the necessary info for sorting

    // sort and replace the children of each node
	for (int i = 0; i < adjList.size(); ++i)
	{
		toBeSorted.clear(); // clear toBeSorted so we don't have any extra pairs from previous iterations
		for (int j = 0; j < adjList.at(i).size(); ++j) // for each child of the current node
		{
			// make a pair where the first element is the branchWeight off of that child and the second element is the child
			newPair.first = branchWeight[ adjList.at(i).at(j) ];
			newPair.second = adjList.at(i).at(j);

			// and then push it onto toBeSorted
			toBeSorted.push_back(newPair);
		}

		// now sort the pairs
		// NOTE:  pairCompare currently places the "heaviest" branchWeight first and the "lightest" branchWeight last
		sort(toBeSorted.begin(), toBeSorted.end(), pairCompare);

		// replace the elements in the adjacency list in sorted order
		for (int j = 0; j < adjList.at(i).size(); ++j)
		{
			adjList.at(i).at(j) = toBeSorted.at(j).second;
		}
	}


	// TESTING:
	// unsigned previous;
	// for (int i = 0; i < adjList.size(); ++i)
	// {
	// 	qDebug() << "i = " << i;
	// 	previous = branchWeight[ adjList.at(i).at(0) ];
	// 	for (int j = 1; j < adjList.at(i).size(); ++j)
	// 	{
	// 		// NOTE:  this line will throw an error depending on the configuration of pairCompare
	// 			// if using "left.first < right.first" in pairCompare, then use "branchWeight[ adjList.at(i).at(j) ] < previous" here
	// 			// if using "left.first > right.first" in pairCompare, then use "branchWeight[ adjList.at(i).at(j) ] > previous" here
	// 		if ( branchWeight[ adjList.at(i).at(j) ] > previous)
	// 		{
	// 			qDebug() << "There was a problem between indices " << j - 1 << " and " << j << " in list at " << i;
	// 			throw std::exception();
	// 		}
	// 		previous = branchWeight[ adjList.at(i).at(j) ];
	// 	}
	// 	qDebug() << "so far so good";
	// }
	// qDebug() << "all was well";

} // end of sortAdjacencies

