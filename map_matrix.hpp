/* map matrix class
 * stores a matrix representing a simplicial map
 */


//constructor that sets initial size of matrix
MapMatrix::MapMatrix(int i, int j)
{
	columns.resize(j);
	num_rows = i;
}

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
//  NOTE: in matrix perspective, row and columns are indexed starting at 1, not 0
void MapMatrix::set(int i, int j)
{
//	std::cout << "    attempting to set row " << i << ", column " << j << "\n";
	
	//if there aren't enough columns, then expand the matrix
	if(columns.size() < j)
		columns.resize(j);
	
	//if the column is empty, then create a node
	if(columns[j-1] == NULL)
	{
//		std::cout << "    creating an initial node in column " << j << "\n";
		
		columns[j-1] = new MapMatrixNode(i);					//DELETE this later????
		return;
	}
	
	//see if node that we would insert already exists in the first position
	if((*columns[j-1]).get_row() == i)
	{
//		std::cout << "    node already exists (in initial position)!\n";
		return;	//avoid duplicate nodes
	}
	
	//see if we need to insert a new node into the first position
	if((*columns[j-1]).get_row() < i)
	{
//		std::cout << "    insert new node into initial position in column " << j << "\n";
		MapMatrixNode* current = new MapMatrixNode(i);			//DELETE this later???
		(*current).set_next(columns[j-1]);
		columns[j-1] = current;
		return;
	}
	
	//for a non-empty column, loop through the nodes to find the proper insertion point
//	std::cout << "   ...looking...\n";
	MapMatrixNode* current = columns[j-1];
	while((*current).get_next() != NULL)
	{
		MapMatrixNode* next = (*current).get_next();
//		std::cout << "   ...inside while loop...\n";
		
		if((*next).get_row() == i)	//then node aready exists
		{
//			std::cout << "    node already exists!\n";
			return;	//avoid duplicate nodes
		}
		
		if((*next).get_row() < i) //then insert new node between current and next
		{
//			std::cout << "    insert new node between node " << (*current).get_row() << " and node " << (*next).get_row() << "\n";
			
			MapMatrixNode* newnode = new MapMatrixNode(i);			//DELETE this later???
			(*newnode).set_next(next);
			(*current).set_next(newnode);	//is this correct?
			
			return;	
		}
		
		//otherwise, move one step
		current = next;
	}
	
	//if we get here, then append a new node to the end of the list
//	std::cout << "   adding new node to the end of the list, after node " << (*current).get_row() << "\n";
	MapMatrixNode* newnode = new MapMatrixNode(i);	
	(*current).set_next(newnode);
}

//clears (to 0) the entry in row i, column j
//  NOTE: in matrix perspective, row and columns are indexed starting at 1, not 0
void MapMatrix::clear(int i, int j)
{
	std::cout << "----MapMatrix::clear() not implemented yet----\n";
}

//returns true if entry (i,j) is 1, false otherwise
//  NOTE: in matrix perspective, row and columns are indexed starting at 1, not 0
bool MapMatrix::entry(int i, int j)
{
	//check number of columns
	if(columns.size() < j)
		return false;
	
	//get initial node pointer
	MapMatrixNode* np = columns[j-1];
	
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
}

//returns the "low" index in the specified column, or 0 if the column is empty or does not exist
//  NOTE: in matrix perspective, row and columns are indexed starting at 1, not 0
int MapMatrix::low(int j)
{
	//if there aren't enough columns, then return 0
	if(columns.size() < j)
		return 0;
	
	//if the column is empty, then return 0
	if(columns[j-1] == NULL)
	{
		return 0;
	}
	
	//otherwise, we have a non-empty column, so return first index (because row indexes are sorted in descending order)
	return (*columns[j-1]).get_row();
	
}


//modifies this matrix to create a block-diagonal matrix with "other" positioned below and to the right of "this"
//  QUESTION: would it be better if this function returned a copy of this matrix instead of modifying it?
/* void MapMatrix::append_block_diag(MapMatrix& other)
{
	//remember initial size of this matrix
	int prev_width = width();
	int prev_height = num_rows;
	
	//create new columns
	columns.resize(prev_width + other.width());
	
	//insert data into each new column
	for(int j=0; j<other.width(); j++)
	{
		MapMatrixNode* handle = NULL;
		MapMatrixNode* othernode = other.columns[j];
		
		while(othernode != NULL)
		{
			int row = (*othernode).get_row() + prev_height;
			if(row > num_rows)
				num_rows = row;
			
			MapMatrixNode* newnode = new MapMatrixNode(row);
			
			if(handle == NULL) //then the column is empty
				columns[prev_width + j] = newnode;
			else
				(*handle).set_next(newnode);
				
			handle = newnode;
			othernode = (*othernode).get_next();
		}
	}
}//end append_block_diag() */

//modifies this matrix to create a larger matrix with "other" positioned to the right of "this", with a vertical offset
//  EXAMPLE: if vert_offset is 0, then "other" is positioned directly to the right of "this" (corresponding rows align)
//			if vert_offset equals "this".num_rows, then the new matrix will be block-diagonal
//  QUESTION: would it be better if this function returned a copy of this matrix instead of modifying it?
void MapMatrix::append_block(MapMatrix& other, int vert_offset)
{
	//remember initial size of this matrix
	int prev_width = width();
	
	//create new columns
	columns.resize(prev_width + other.width());
	
	//insert data into each new column
	for(int j=0; j<other.width(); j++)
	{
		MapMatrixNode* handle = NULL;
		MapMatrixNode* othernode = other.columns[j];
		
		while(othernode != NULL)
		{
			int row = (*othernode).get_row() + vert_offset;
			if(row > num_rows)
				num_rows = row;
			
			MapMatrixNode* newnode = new MapMatrixNode(row);
			
			if(handle == NULL) //then the column is empty
				columns[prev_width + j] = newnode;
			else
				(*handle).set_next(newnode);
				
			handle = newnode;
			othernode = (*othernode).get_next();
		}
	}
}//end append_block_diag()





//adds column j to column k
//  NOTE: in matrix perspective, row and columns are indexed starting at 1, not 0
//  RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
void MapMatrix::add_column(int j, int k)
{
	//make sure both columns exist; if not, expand the matrix
	if(columns.size() < j)
		columns.resize(j);
	if(columns.size() < k)
		columns.resize(k);	
	
	//pointers
	MapMatrixNode* jnode = columns[j-1]; //points to next node from column j that we will add to column k
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
		
		if(columns[k-1] == NULL) //then column k is empty, so insert initial node
		{
//			std::cout << "    -inserting new node in initial position into column " << k << "\n";
			MapMatrixNode* newnode = new MapMatrixNode(row);
			columns[k-1] = newnode;
			khandle = newnode;
			
			added = true; //proceed with next element from column j
		}
		
		if(!added && khandle == NULL) //then we haven't yet added anything to column k (but if we get here, column k is nonempty)
		{
			if((*columns[k-1]).get_row() == row) //then remove this node (since 1+1=0)
			{
//				std::cout << "    -removing node " << row << " from initial position (since 1+1=0)\n";
				MapMatrixNode* next = (*columns[k-1]).get_next();
				delete columns[k-1];
				columns[k-1] = next;
				added = true; //proceed with next element from column j
			}
			else if((*columns[k-1]).get_row() < row) //then insert new initial node into column k
			{
//				std::cout << "    -inserting new node " << row << " into initial position\n";
				MapMatrixNode* newnode = new MapMatrixNode(row);
				(*newnode).set_next(columns[k-1]);
				columns[k-1] = newnode;
				khandle = columns[k-1];
				added = true; //proceed with next element from column j
			}
			else //then move to next node in column k
			{
				khandle = columns[k-1];
				//now we want to enter the following while loop
			}
		}//end if
		
		while(!added && ( (*khandle).get_next() != NULL )) //if we get here, both columns[k-1] and khandle are NOT NULL
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


//applies the column reduction algorithm to this matrix
void MapMatrix::col_reduce()
{
	//loop through columns
	for(int j=1; j<=columns.size(); j++)
	{
		int k=1;	//we loop as long as there exists k<j such that low(k) == low(j) != 0
		while(low(j) > 0 && k<j)
		{
			if(low(k) == low(j))	//then add column k to column j, and reset k to 1
			{
				add_column(k,j);
				k=1;
			}
			else	//then increment k
				k++;
		}
	}
}//end col_reduce()

//applies the column reduction algorithm to this matrix, and also performs the same column operations on the other matrix
void MapMatrix::col_reduce(MapMatrix* other)
{
	//loop through columns
	for(int j=1; j<=columns.size(); j++)
	{
		int k=1;	//we loop as long as there exists k<j such that low(k) == low(j) != 0
		while(low(j) > 0 && k<j)
		{
			if(low(k) == low(j))	//then add column k to column j, and reset k to 1
			{
				add_column(k,j);
				(*other).add_column(k,j);
				k=1;
			}
			else	//then increment k
				k++;
		}
	}
}//end col_reduce(MapMatrix* other)


//removes column j from the matrix (decreasing width by 1)
//  NOTE: in matrix perspective, row and columns are indexed starting at 1, not 0
void MapMatrix::remove_column(int j)
{
//	std::cout << "   --deleting column " << j << "\n";
	MapMatrixNode* node = columns[j-1];
	while(node != NULL)
	{
		MapMatrixNode* prev = node;
		node = (*prev).get_next();
//		std::cout << "   ----deleting node at row " << (*prev).get_row() << "\n";
		delete prev;
	}
	columns.erase( columns.begin() + j-1);			//this is probably inefficient---HOW TO IMPROVE???
}//end remove_column()




//function to print the matrix to standard output, mainly for testing purposes
void MapMatrix::print()
{
	//handle empty matrix
	if(num_rows == 0 || columns.size() == 0)
	{
		std::cout << "    (empty matrix)\n";
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
			int row = (*current).get_row()-1;
			mx[row][j] = 1;
			current = (*current).get_next();
//			std::cout << "-------entry (" << row << "," << j << ") set to 1\n";	//TESTING
		}
	}
	
	//print the matrix
	for(int i=0; i<num_rows; i++)
	{
		std::cout << "    |";
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


