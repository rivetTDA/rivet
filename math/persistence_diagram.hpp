/**
 * \class	PersistenceDiagram
 * \brief	Stores the data necessary to draw a persistence diagram
 * \author	Matthew L. Wright
 * \date	May 2014
 */

#ifndef __PERSISTENCE_DIAGRAM_H__
#define __PERSISTENCE_DIAGRAM_H__

//includes????
#include <vector>
#include <utility>      // std::pair


class PersistenceDiagram
{
	public:
		PersistenceDiagram();	//constructor
		
		void add_pair(double birth, double death);	//stores a persistence pair
		void add_cycle(double birth);			//stores an essential cycle
		
		std::vector< std::pair<double,double> >* get_pairs();	//gets a pointer to the vector of persistence pairs
		std::vector<double>* get_cycles();			//gets a pointer to the vector of essential cycles
	
	private:
		std::vector< std::pair<double,double> > pairs;	//stores persistence pairs (birth and death times)
		
		std::vector<double> cycles;	//stores birth times of essential cycles
		
};

////////// implementation //////////

PersistenceDiagram::PersistenceDiagram()
{ }

void PersistenceDiagram::add_pair(double birth, double death)
{
	pairs.push_back(std::pair<double,double>(birth, death));
}

void PersistenceDiagram::add_cycle(double birth)
{
	cycles.push_back(birth);
}

std::vector< std::pair<double,double> >* PersistenceDiagram::get_pairs()
{
	return &pairs;
}

std::vector<double>* PersistenceDiagram::get_cycles()
{
	return &cycles;
}



#endif // __PERSISTENCE_DIAGRAM_H__

