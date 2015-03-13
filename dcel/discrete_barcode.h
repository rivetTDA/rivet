/**
 * \class	DiscreteBarcode
 * \brief	Stores a barcode indexed by \xi support points, to be used in each 2-cell of the arrangement.
 * \author	Matthew L. Wright
 * \date	March 2015
 */


#ifndef __DISCRETE_BARCODE_H__
#define __DISCRETE_BARCODE_H__

typedef std::vector< std::pair<unsigned,unsigned> > PairVec;
typedef std::vector<unsigned>                       CycleVec;

struct DiscreteBarcode
{
        PairVec pairs;      //stores persistence pairs (each pair is a bar in the barcode)
        CycleVec cycles;    //stores essential cycles (i.e. infinite persistence bars)
};

#endif // __DISCRETE_BARCODE_H__
