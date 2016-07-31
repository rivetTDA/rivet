// NOTE:  this file will eventually contain more functions necessary for the graph cutting procedures that are in progress
	// for now, it just contains the adjacency list sorting functions

#include <stack>
#include <vector>


// NOTE:  this function currently sorts in reverse order (most branching first, least branching last)
bool pairCompare(std::pair<unsigned, unsigned> left, std::pair<unsigned, unsigned> right)
{
    // return left.first < right.first; // "NORMAL"  pairCompare FOR VERSION 2 OF find_subpath
    return left.first > right.first; // "REVERSED" pairCompare FOR VERSION 1 OF find_subpath
}

// TODO:  I think this should be rewritten to use just a boolean array for keeping track of which nodes have been discovered
	// note:  we cannot use the adjacency deleting strategy that we used in version 1 of find_subpath() since we cannot destroy the
	// adjacency list at this stage
void sortAdjacencies(std::vector<std::vector<unsigned> > &adjList, std::vector<std::vector<unsigned> > &distances, unsigned start)
{
	// first do DFS to find which nodes have the most branching
	std::vector<unsigned> branching(adjList.size(), 0); // this will contain the number of nodes "hanging" from the node represented by its index in branching
	std::vector<bool> discovered(adjList.size(), false);
	discovered.at(start) = true;

	std::stack<unsigned> nodes, edgeStack;
	nodes.push(start);
	edgeStack.push(0);
	unsigned node, edgeIndex, runningSum;

	while (!nodes.empty())
	{
		node = nodes.top();
		edgeIndex = edgeStack.top();

		if (edgeIndex < adjList.at(node).size())
		{
			if (!discovered.at(adjList.at(node).at(edgeIndex)))
			{
				discovered.at(adjList.at(node).at(edgeIndex)) = true;
				nodes.push(adjList.at(node).at(edgeIndex));
				edgeStack.top()++;
				edgeStack.push(0);
			}
			else
			{
				edgeIndex = ++edgeStack.top();
			}
		}
		else
		{
			nodes.pop();
			edgeStack.pop();
			runningSum = 0;
			if (!nodes.empty())
			{
				for (int i = 0; i < adjList.at(node).size(); ++i)
				{
					if ( adjList.at(node).at(i) != nodes.top() )
					{
						runningSum += 1 + branching.at(adjList.at(node).at(i));
					}
				}
			}
			else
			{
				for (int i = 0; i < adjList.at(node).size(); ++i)
				{
 					runningSum += 1 + branching.at(adjList.at(node).at(i));
				}
			}

			branching.at(node) = runningSum;
		}
	} // end of while

	// TESTING:
	// qDebug() << "adjList = ";
	// for (int i = 0; i < adjList.size(); ++i)
	// {
	// 	qDebug() << "branching.at(" << i << ") = " << branching.at(i);
	// 	// qDebug().nospace() << i << " = ";
	// 	for (int j = 0; j < adjList.at(i).size(); ++j)
	// 	{
	// 		qDebug().nospace() << adjList.at(i).at(j) << "  ";
	// 	}
	// }

	// qDebug() << "start = " << start;
	// for (int i = 0; i < branching.size(); ++i)
	// {
	// 	qDebug() << "branching.at(" << i << ") = " << branching.at(i);
	// }

    std::vector<std::pair<unsigned, unsigned> > toBeSorted; // There may be a more efficient way to store this
    std::pair<unsigned, unsigned> newPair;

	for (int i = 0; i < adjList.size(); ++i)
	{
		toBeSorted.clear();
		for (int j = 0; j < adjList.at(i).size(); ++j)
		{
			newPair.first = branching.at(adjList.at(i).at(j));
			newPair.second = adjList.at(i).at(j);
			toBeSorted.push_back(newPair);
		}

		// now sort them
		// NOTE:  pairCompare currently places the most branching first and the least branching last
		sort(toBeSorted.begin(), toBeSorted.end(), pairCompare);

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
	// 	previous = branching.at( adjList.at(i).at(0) );
	// 	for (int j = 1; j < adjList.at(i).size(); ++j)
	// 	{
	// 		if ( branching.at( adjList.at(i).at(j) ) < previous)
	// 		{
	// 			qDebug() << "There was a problem between indices " << j - 1 << " and " << j << " in list at " << i;
	// 			exit(0);
	// 		}
	// 		previous = branching.at( adjList.at(i).at(j) );
	// 	}
	// 	qDebug() << "so far so good";
	// }

} // end of sortAdjacencies

