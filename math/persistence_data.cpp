#include "persistence_data.h"




//// implementation of PersistenceData ////

PersistenceData::PersistenceData()
{ }

void PersistenceData::add_pair(double birth, double death)
{
    pairs.insert(std::pair<double,double>(birth, death));
}

void PersistenceData::add_cycle(double birth)
{
    cycles.insert(birth);
}

std::multiset<std::pair<double, double>, PairComparator > *PersistenceData::get_pairs()
{
    return &pairs;
}

std::multiset<double> *PersistenceData::get_cycles()
{
    return &cycles;
}
