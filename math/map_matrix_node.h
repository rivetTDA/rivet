/**
 * \class	MapMatrixNode
 * \brief	A node for the MapMatrix class.
 * \author	Matthew L. Wright
 * \date	February 2014
 * 
 * The MapMatrixNode class stores a node for the sparse implementation of a simplex map.
 */
 
#ifndef __MapMatrixNode_H__
#define __MapMatrixNode_H__


class MapMatrixNode {
	public:
		MapMatrixNode(int);		//constructor
		
		int get_row();			//returns the row index
		void set_next(MapMatrixNode*);	//sets the pointer to the next node in the column
		MapMatrixNode* get_next();	//returns a pointer to the next node in the column


	private:
		int row_index;			//index of matrix row corresponding to this node
		MapMatrixNode * next;		//pointer to the next entry in the column containing this node

};

//constructor
MapMatrixNode::MapMatrixNode(int i)
{
	row_index = i;
	next = NULL;
}

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

#endif // __MapMatrixNode_H__

