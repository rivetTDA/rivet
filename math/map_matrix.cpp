/* map matrix class
 * stores a matrix representing a simplicial map
 */

#include "map_matrix.h"

#include <iostream> //for testing only

/*** implementation of class MapMatrixNode ***/

//constructor
MapMatrixNode::MapMatrixNode(int i) :
    row_index(i), next(NULL)
{ }

//returns the row index
int MapMatrixNode::get_row()
{
    return row_index;
}

//sets the pointer to the next node in the column
void MapMatrixNode::set_next(MapMatrixNode* n)
{
    next = n;
}

//returns a pointer to the next node in the column
MapMatrixNode* MapMatrixNode::get_next()
{
    return next;
}



/*** implementation of class MapMatrix ***/

//constructor that sets initial size of matrix
MapMatrix::MapMatrix(int rows, int cols) :
    columns(cols), num_rows(rows)
{ }

//returns the number of columns in the matrix
int MapMatrix::width()
{
	return columns.size();
}

//returns the number of rows in the matrix
int MapMatrix::height()
{
	return num_rows;
}

//sets (to 1) the entry in row i, column j 
void MapMatrix::set(int i, int j)
{
    //make sure this operation is valid
    if(columns.size() <= j)
        throw std::runtime_error("attempting to set column past end of matrix");
    if(num_rows <= i)
        throw std::runtime_error("attempting to set row past end of matrix");
	
	//if the column is empty, then create a node
    if(columns[j] == NULL)
	{
        columns[j] = new MapMatrixNode(i);					//DELETE this later????
		return;
	}
	
	//see if node that we would insert already exists in the first position
    if((*columns[j]).get_row() == i)
	{
		return;	//avoid duplicate nodes
	}
	
	//see if we need to insert a new node into the first position
    if((*columns[j]).get_row() < i)
	{
		MapMatrixNode* current = new MapMatrixNode(i);			//DELETE this later???
        (*current).set_next(columns[j]);
        columns[j] = current;
		return;
	}
	
	//for a non-empty column, loop through the nodes to find the proper insertion point
    MapMatrixNode* current = columns[j];
	while((*current).get_next() != NULL)
	{
		MapMatrixNode* next = (*current).get_next();

		if((*next).get_row() == i)	//then node aready exists
		{
			return;	//avoid duplicate nodes
		}
		
		if((*next).get_row() < i) //then insert new node between current and next
		{
			MapMatrixNode* newnode = new MapMatrixNode(i);			//DELETE this later???
			(*newnode).set_next(next);
            (*current).set_next(newnode);
			
			return;	
		}
		
		//otherwise, move one step
		current = next;
	}
	
	//if we get here, then append a new node to the end of the list
	MapMatrixNode* newnode = new MapMatrixNode(i);	
	(*current).set_next(newnode);
}//end set()

//returns true if entry (i,j) is 1, false otherwise
bool MapMatrix::entry(int i, int j)
{
	//check number of columns
    if(columns.size() <= j)
		return false;
	
	//get initial node pointer
    MapMatrixNode* np = columns[j];
	
	//loop while there is another node to check
	while(np != NULL)
	{
		if((*np).get_row() < i)	//then we won't find row i because row entrys are sorted in descending order
			return false;
		if((*np).get_row() == i) //then we found the row we wanted
			return true;
		
		//if we are still looking, then get the next node
		np = (*np).get_next();
	}
	
	//if we get here, then we didn't find the entry
	return false;
}//end entry()

//returns the "low" index in the specified column, or 0 if the column is empty or does not exist
int MapMatrix::low(unsigned j)
{
    //if there aren't enough columns, then return -1
    if(columns.size() <= j)
        return -1;
	
    //if the column is empty, then return -1
    if(columns[j] == NULL)
	{
        return -1;
	}
	
	//otherwise, we have a non-empty column, so return first index (because row indexes are sorted in descending order)
    return (*columns[j]).get_row();
}


//applies the column reduction algorithm to this matrix
//TODO: this could be improved with a "low array"
void MapMatrix::col_reduce()
{
    //create low array
    std::vector<int> lows(num_rows, -1);

    //loop through columns
    for(unsigned j=0; j<columns.size(); j++)
    {
        //while column j is nonempty and its low number is found in the low array, do column operations
        while(low(j) >= 0 && lows[low(j)] >= 0)
            add_column(lows[low(j)], j);

        if(low(j) >= 0)    //then column is still nonempty, so update lows
            lows[low(j)] = j;
    }
}//end col_reduce()

////applies the column reduction algorithm to this matrix, and also performs the same column operations on the other matrix
//void MapMatrix::col_reduce(MapMatrix* other)
//{
//    //loop through columns
//    for(int j=0; j<columns.size(); j++)
//    {
//        int k=0;	//we loop as long as there exists k<j such that low(k) == low(j) != -1
//        while(low(j) >= 0 && k<j)
//        {
//            if(low(k) == low(j))	//then add column k to column j, and reset k to 0
//            {
//                add_column(k,j);
//                (*other).add_column(k,j);
//                k=0;
//            }
//            else	//then increment k
//                k++;
//        }
//    }
//}//end col_reduce(MapMatrix* other)


//adds column j to column k
//  RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
void MapMatrix::add_column(unsigned j, unsigned k)
{
    //make sure this operation is valid
    if(columns.size() <= j || columns.size() <= k)
        throw std::runtime_error("attempting to access column past end of matrix");
	
	//pointers
    MapMatrixNode* jnode = columns[j]; //points to next node from column j that we will add to column k
	MapMatrixNode* khandle = NULL; //points to node in column k that was most recently added; will be non-null after first node is added
	
	//loop through all entries in column j
	while(jnode != NULL)
	{
		//now it is save to dereference jnode (*jnode)...
		int row = (*jnode).get_row();
		
		//so we want to add row to column k
//		std::cout << "  --we want to add row " << row << " to column " << k << "\n";
		
		//loop through entries in column k, starting at the current position
		bool added = false;
		
        if(columns[k] == NULL) //then column k is empty, so insert initial node
		{
//			std::cout << "    -inserting new node in initial position into column " << k << "\n";
			MapMatrixNode* newnode = new MapMatrixNode(row);
            columns[k] = newnode;
			khandle = newnode;
			
			added = true; //proceed with next element from column j
		}
		
		if(!added && khandle == NULL) //then we haven't yet added anything to column k (but if we get here, column k is nonempty)
		{
            if((*columns[k]).get_row() == row) //then remove this node (since 1+1=0)
			{
//				std::cout << "    -removing node " << row << " from initial position (since 1+1=0)\n";
                MapMatrixNode* next = (*columns[k]).get_next();
                delete columns[k];
                columns[k] = next;
				added = true; //proceed with next element from column j
			}
            else if((*columns[k]).get_row() < row) //then insert new initial node into column k
			{
//				std::cout << "    -inserting new node " << row << " into initial position\n";
				MapMatrixNode* newnode = new MapMatrixNode(row);
                (*newnode).set_next(columns[k]);
                columns[k] = newnode;
                khandle = columns[k];
				added = true; //proceed with next element from column j
			}
			else //then move to next node in column k
			{
                khandle = columns[k];
				//now we want to enter the following while loop
			}
		}//end if
		
        while(!added && ( (*khandle).get_next() != NULL )) //if we get here, both columns[k] and khandle are NOT NULL
		{
			//consider the next node
			MapMatrixNode* next = (*khandle).get_next();
			
			if((*next).get_row() == row) //then remove the next node (since 1+1=0)
			{
//				std::cout << "    -removing node " << row << " (since 1+1=0)\n";
				(*khandle).set_next( (*next).get_next() );
				delete next;
				added = true; //proceed with next element from column j
			}
			else if((*next).get_row() < row) //then insert new initial node into column k
			{
//				std::cout << "    -inserting new node " << row << "\n";
				MapMatrixNode* newnode = new MapMatrixNode(row);
				(*newnode).set_next(next);
				(*khandle).set_next(newnode);
				added = true; //proceed with next element from column j
			}
			else //then (*next).get_row() > row, so move to next node in column k
			{
				khandle = next;
			}
		}//end while
		
		if(!added && ( (*khandle).get_next() == NULL )) //then we have reached the end of the list, and we should append a new node
		{
//			std::cout << "    -appending a new node " << row << " to the end of column\n";
			MapMatrixNode* newnode = new MapMatrixNode(row);
			(*khandle).set_next(newnode);
			khandle = newnode;
		}
		
		//move to the next entry in column j
		jnode = (*jnode).get_next();
    }//end while(jnode != NULL)

}//end add_column(int, int)

//adds column j from MapMatrix* other to column k of this matrix
void MapMatrix::add_column(MapMatrix* other, int j, int k)
{
    //make sure this operation is valid
    if(other->columns.size() <= j || columns.size() <= k)
        throw std::runtime_error("attempting to access column past end of matrix");

//    std::cout << "      add_column(other, " << j << ", " << k << ")\n";

    //pointers
    MapMatrixNode* jnode = other->columns[j]; //points to next node from column j that we will add to column k
    MapMatrixNode* khandle = NULL; //points to node in column k that was most recently added; will be non-null after first node is added

    //loop through all entries in column j
    while(jnode != NULL)
    {
        //now it is save to dereference jnode
        int row = jnode->get_row();

        //so we want to add row to column k
//        std::cout << "  --we want to add row " << row << " to column " << k << "\n";

        //loop through entries in column k, starting at the current position
        bool added = false;

        if(columns[k] == NULL) //then column k is empty, so insert initial node
        {
//			std::cout << "    -inserting new node in initial position into column " << k << "\n";
            MapMatrixNode* newnode = new MapMatrixNode(row);
            columns[k] = newnode;
            khandle = newnode;

            added = true; //proceed with next element from column j
        }

        if(!added && khandle == NULL) //then we haven't yet added anything to column k (but if we get here, column k is nonempty)
        {
            if((*columns[k]).get_row() == row) //then remove this node (since 1+1=0)
            {
//				std::cout << "    -removing node " << row << " from initial position (since 1+1=0)\n";
                MapMatrixNode* next = (*columns[k]).get_next();
                delete columns[k];
                columns[k] = next;
                added = true; //proceed with next element from column j
            }
            else if((*columns[k]).get_row() < row) //then insert new initial node into column k
            {
//				std::cout << "    -inserting new node " << row << " into initial position\n";
                MapMatrixNode* newnode = new MapMatrixNode(row);
                (*newnode).set_next(columns[k]);
                columns[k] = newnode;
                khandle = columns[k];
                added = true; //proceed with next element from column j
            }
            else //then move to next node in column k
            {
                khandle = columns[k];
                //now we want to enter the following while loop
            }
        }//end if

        while(!added && ( (*khandle).get_next() != NULL )) //if we get here, both columns[k] and khandle are NOT NULL
        {
            //consider the next node
            MapMatrixNode* next = (*khandle).get_next();

            if((*next).get_row() == row) //then remove the next node (since 1+1=0)
            {
//				std::cout << "    -removing node " << row << " (since 1+1=0)\n";
                (*khandle).set_next( (*next).get_next() );
                delete next;
                added = true; //proceed with next element from column j
            }
            else if((*next).get_row() < row) //then insert new initial node into column k
            {
//				std::cout << "    -inserting new node " << row << "\n";
                MapMatrixNode* newnode = new MapMatrixNode(row);
                (*newnode).set_next(next);
                (*khandle).set_next(newnode);
                added = true; //proceed with next element from column j
            }
            else //then (*next).get_row() > row, so move to next node in column k
            {
                khandle = next;
            }
        }//end while

        if(!added && ( (*khandle).get_next() == NULL )) //then we have reached the end of the list, and we should append a new node
        {
//			std::cout << "    -appending a new node " << row << " to the end of column\n";
            MapMatrixNode* newnode = new MapMatrixNode(row);
            (*khandle).set_next(newnode);
            khandle = newnode;
        }

        //move to the next entry in column j
        jnode = jnode->get_next();
    }//end while(jnode != NULL)

}//end add_column(MapMatrix*, int, int)





//function to print the matrix to standard output, mainly for testing purposes
void MapMatrix::print()
{
    //handle empty matrix
	if(num_rows == 0 || columns.size() == 0)
	{
        std::cout << "        (empty matrix: " << num_rows << " rows by " << columns.size() << " columns)\n";
		return;
	}
	
	//create a 2D array of booleans to temporarily store the matrix
	bool mx[num_rows][columns.size()];
	for(int i=0; i<num_rows; i++)
		for(int j=0; j<columns.size(); j++)
			mx[i][j] = false;
	
	//traverse the linked lists in order to fill the 2D array
	MapMatrixNode* current;
	for(int j=0; j<columns.size(); j++)
	{
		current = columns[j];
		while(current != NULL)
		{
            int row = (*current).get_row();
			mx[row][j] = 1;
			current = (*current).get_next();
		}
	}
	
	//print the matrix
	for(int i=0; i<num_rows; i++)
	{
		std::cout << "        |";
		for(int j=0; j<columns.size(); j++)
		{
			if(mx[i][j])
				std::cout << " 1";
			else
				std::cout << " 0";
		}
		std::cout << " |\n";
	}
}//end print()


