#include "bifiltration_data.h"

#include "index_matrix.h"
#include "map_matrix.h"

#include "debug.h"

#include <limits>   //std::numeric_limits
#include <stdexcept>
#include <iostream>  //for std::cout, for testing only


//BifiltrationData constructor; requires dimension of homology to be computed and verbosity parameter
BifiltrationData::BifiltrationData(int dim, int v) :
     hom_dim(dim), verbosity(v), x_grades(0), y_grades(0)
{
    if (hom_dim > 5) {
        throw std::runtime_error("BifiltrationData: Dimensions greater than 5 probably don't make sense");
    }
    if (verbosity >= 8) {
        debug() << "Created BifiltrationData(" << hom_dim << ", " << verbosity << ")";
    }
    ordered_low_simplices = new SimplexInfo;
    ordered_simplices = new SimplexInfo;
    ordered_high_simplices = new SimplexInfo;
    ordered_low_grades = new GradeInfo;
    ordered_grades = new GradeInfo;
    ordered_high_grades = new GradeInfo;
}

//destructor
BifiltrationData::~BifiltrationData()
{
    //delete simplex data
    delete ordered_low_simplices;
    delete ordered_simplices;
    delete ordered_high_simplices;
    //delete grade data
    delete ordered_low_grades;
    delete ordered_grades;
    delete ordered_high_grades;
}

//adds a simplex (including all of its faces) to the BifiltrationData
//if simplex or any of its faces already exist, their new grades are added
//WARNING: doesn't verify multigrades are non-comparable
//We don't need to add simplexes of dimension greater than hom_dim+1
void BifiltrationData::add_simplex(std::vector<int>& vertices, AppearanceGrades& grades)
{
    //make sure vertices are sorted
    std::sort(vertices.begin(), vertices.end());

    //add the simplex and all of its faces
    add_faces(vertices, grades);
}//end add_simplex()

//recursively adds faces of a simplex to the BifiltrationData
void BifiltrationData::add_faces(std::vector<int>& vertices, AppearanceGrades& grades)
{
    SimplexInfo::iterator ret;
    //Store the simplex info if it is of dimension (hom_dim - 1), hom_dim, or hom_dim+1. Dimension is parent_indexes.size() - 1
    if (vertices.size() == 0)
    {
        return;
    }
    else if (vertices.size() == (unsigned)hom_dim) //simplex of dimension hom_dim - 1
    {
        ret = ordered_low_simplices->find(vertices);
        if (ret == ordered_low_simplices->end()) //Grade not found
        {
            ordered_low_simplices->emplace(vertices, grades);
        }
        else //Grade found, pointed at by ret
        {
            ret->second.insert(ret->second.end(), grades.begin(), grades.end());
        }
        return;
    }
    else if (vertices.size() == (unsigned)hom_dim + 1) //simplex of dimension hom_dim
    {
        ret = ordered_simplices->find(vertices);
        if (ret == ordered_simplices->end()) //Grade not found
        {
            ordered_simplices->emplace(vertices, grades);
        }
        else //Grade found, pointed at by ret
        {
            ret->second.insert(ret->second.end(), grades.begin(), grades.end());
        }
    }
    else if (vertices.size() == (unsigned)hom_dim + 2) //simplex of dimension hom_dim + 1
    {
        ret = ordered_high_simplices->find(vertices);
        if (ret == ordered_high_simplices->end()) //Grade not found
        {
            ordered_high_simplices->emplace(vertices, grades);
        }
        else //Grade found, pointed at by ret
        {
            ret->second.insert(ret->second.end(), grades.begin(), grades.end());
        }
    }
    else if (vertices.size() < (unsigned)hom_dim) //looking at dimension less than hom_dim -1= 1 and we do not care about those simplices
    {
        return;
    }

    //Iterate for each face
    for(unsigned i = 0; i < vertices.size(); i++)
    {
        //form vector consisting of all the vertices except for vertices[i]
        std::vector<int> face;
        for(unsigned k = 0; k < vertices.size(); k++)
            if(k != i)
                face.push_back(vertices[k]);

        //add the face simplex to the BifiltrationData
        add_faces(face, grades);
    }
}//end add_faces()

//builds BifiltrationData representing a bifiltered Vietoris-Rips complex from discrete data
//requires a list of birth times (one for each point) and a list of distances between pairs of points
//NOTE: automatically computes global indexes and dimension indexes
//CONVENTION: the x-coordinate is "birth time" for points and the y-coordinate is "distance" between points
void BifiltrationData::build_VR_complex(std::vector<unsigned>& times, std::vector<unsigned>& distances, unsigned num_x, unsigned num_y)
{
    x_grades = num_x;
    y_grades = num_y;

    //Add generation points recursively
    for(unsigned i=0; i<times.size(); i++)
    {
        //recursion
        std::vector<int> vertices;
        vertices.push_back(i);

        build_VR_subcomplex(times, distances, vertices, times[i], 0);
    }
}//end build_VR_complex()

//function to add (recursively) a subcomplex of the bifiltration data
void BifiltrationData::build_VR_subcomplex(std::vector<unsigned>& times, std::vector<unsigned>& distances, std::vector<int>& vertices, unsigned prev_time, unsigned prev_dist)
{
    //Store the simplex info if it is of dimension (hom_dim - 1), hom_dim, or hom_dim+1. Dimension is vertices.size() - 1
    if (vertices.size() == (unsigned)hom_dim) //simplex of dimension hom_dim - 1
    {
        AppearanceGrades grades = AppearanceGrades(1, Grade(prev_time, prev_dist));
        ordered_low_simplices->emplace(vertices, grades); //makes copy of parent_indexes so we can still edit it
        addSimplicesToGrade(ordered_low_grades, vertices, grades);
    }
    else if (vertices.size() == (unsigned)hom_dim + 1) //simplex of dimension hom_dim - 1
    {
        AppearanceGrades grades = AppearanceGrades(1, Grade(prev_time, prev_dist));
        ordered_simplices->emplace(vertices, grades); //makes copy of parent_indexes so we can still edit it
        addSimplicesToGrade(ordered_grades, vertices, grades);
    }
    else if (vertices.size() == (unsigned)hom_dim + 2) //simplex of dimension hom_dim - 1
    {
        AppearanceGrades grades = AppearanceGrades(1, Grade(prev_time, prev_dist));
        ordered_high_simplices->emplace(vertices, grades); //makes copy of parent_indexes so we can still edit it
        addSimplicesToGrade(ordered_high_grades, vertices, grades);
        return;
    }
    else if (vertices.size() > (unsigned)hom_dim + 2) //looking at dimension at least hom_dim + 2 and we do not care about those simplices
    {
        return;
    }

    //loop through all points that could be children of this node
    for(unsigned j = vertices.back() + 1; j < times.size(); j++)
    {
        //look up distances from point j to each of the vertices in the simplex
        //distance index is maximum of prev_distance and each of these distances
        unsigned current_dist = prev_dist;
        for(unsigned k = 0; k < vertices.size(); k++)
        {
                int p = vertices[k];
                unsigned d = distances[ (j*(j-1))/2 + p ]; //the distance between points p and j, with p < j
                if(d > current_dist)
                    current_dist = d;
        }

        //see if the distance is permitted
        if(current_dist < std::numeric_limits<unsigned>::max())	//then we will add this larger simplex
        {
            //compute time index of this new node
            unsigned current_time = times[j];
            if(current_time < prev_time)
                current_time = prev_time;

            //recursion
            vertices.push_back(j); //next we will look for children of node j
            build_VR_subcomplex(times, distances, vertices, current_time, current_dist);
            vertices.pop_back(); //finished adding children of node j
        }
    }
}//end build_subtree()

//builds BifiltrationData representing a bifiltered Vietoris-Rips complex from discrete data
//requires a list of vertices and a list of distances between pairs of points
//CONVENTION: the x-coordinate is "scale threshold" for points and the y-coordinate is "degree threshold"
void BifiltrationData::build_BR_complex(unsigned num_vertices, std::vector<unsigned>& distances, std::vector<unsigned>& degrees, unsigned num_x, unsigned num_y)
{
    x_grades = num_x;
    y_grades = num_y;

    //build complex recursively
    //this also assigns global indexes to each simplex
    if(verbosity >= 6) { debug() << "BUILDING BRIPS COMPLEX"; }

    std::vector<AppearanceGrades> vertexMultigrades;
    generateVertexMultigrades(vertexMultigrades, num_vertices, distances, degrees);

    std::vector<int> simplex_indices;
    for(unsigned i = 0; i < num_vertices; i++)
    {
        //Look at simplex with smallest vertex vertex i
        simplex_indices.push_back(i);

        //Determine the neighbors of vertex i
        std::vector<int> candidates;
        for (unsigned j = i + 1; j < num_vertices; j++)//Dist of (i, j) with i < j stored in distances[j(j - 1)/2 + i]
        {
            if (distances[j * (j - 1)/2 + i] < std::numeric_limits<unsigned>::max()) //if an edge is between i and j
                candidates.push_back(j);
        }

        //recursion
        build_BR_subcomplex(distances, simplex_indices, candidates, vertexMultigrades[i], vertexMultigrades);
        simplex_indices.pop_back();
    }
}//end build_BR_complex()

//function to build (recursively) a subcomplex for the BRips complex
void BifiltrationData::build_BR_subcomplex(std::vector<unsigned>& distances, std::vector<int>& parent_indexes, std::vector<int>& candidates, AppearanceGrades& parent_grades, std::vector<AppearanceGrades>& vertexMultigrades)
{
    //Store the simplex info if it is of dimension (hom_dim - 1), hom_dim, or hom_dim+1. Dimension is parent_indexes.size() - 1
    if (parent_indexes.size() == (unsigned)hom_dim) //simplex of dimension hom_dim - 1
    {
        ordered_low_simplices->emplace(parent_indexes, parent_grades); //makes copy of parent_indexes so we can still edit it
        addSimplicesToGrade(ordered_low_grades, parent_indexes, parent_grades);
    }
    else if (parent_indexes.size() == (unsigned)hom_dim + 1) //simplex of dimension hom_dim - 1
    {
        ordered_simplices->emplace(parent_indexes, parent_grades); //makes copy of parent_indexes so we can still edit it
        addSimplicesToGrade(ordered_grades, parent_indexes, parent_grades);
    }
    else if (parent_indexes.size() == (unsigned)hom_dim + 2) //simplex of dimension hom_dim - 1
    {
        ordered_high_simplices->emplace(parent_indexes, parent_grades); //makes copy of parent_indexes so we can still edit it
        addSimplicesToGrade(ordered_high_grades, parent_indexes, parent_grades);
        return;
    }
    else if (parent_indexes.size() > (unsigned)hom_dim + 2) //looking at dimension at least hom_dim + 2 and we do not care about those simplices
    {
        return;
    }

    //loop through all points that could be added to form a larger simplex (candidates)
    for(std::vector<int>::iterator it = candidates.begin(); it != candidates.end(); it++)
    {
        //Determine the grades of appearance of the clique with parent_indexes and *it
        //First determine the minimal scale parameter necessary for all the edges between the clique parent_indexes, and *it to appear
        unsigned minDist = std::numeric_limits<unsigned>::max();
        for (std::vector<int>::iterator it2 = parent_indexes.begin(); it2 != parent_indexes.end(); it2++)
            if (distances[(*it) * (*it - 1) / 2 + *it2] < minDist) //By construction, each of the parent indices are strictly less than *it
                minDist = distances[(*it) * (*it - 1) / 2 + *it2];
        AppearanceGrades newGrades;
        combineMultigrades(newGrades, parent_grades, vertexMultigrades[*it], minDist);

        //Determine subset of candidates which are still candidates after adding *it
        std::vector<int> newCandidates;
        for (std::vector<int>::iterator it2 = it + 1; it2 != candidates.end(); it2++)
        {
            if (distances[(*it2) * (*it2 - 1) / 2 + *it] < std::numeric_limits<unsigned>::max()) //We know that *it2 > *it
            {
                newCandidates.push_back(*it2); //We knew there was connection between *it2 and all of parent_index, and now also to *it as well
            }
        }

        parent_indexes.push_back(*it);
        //recurse
        build_BR_subcomplex(distances, parent_indexes, newCandidates, newGrades, vertexMultigrades);
        parent_indexes.pop_back(); //Finished looking at cliques adding *it as well
    }
}//end build_subcomplex()

//For each point in a BRips bifiltration, generates an array of incomparable grades of appearance. distances should be of size vertices(vertices - 1)/2
//Stores result in the vector container "multigrades". Each vector of grades is sorted in reverse lexicographic order
void BifiltrationData::generateVertexMultigrades(std::vector<AppearanceGrades>& multigrades, unsigned vertices, std::vector<unsigned>& distances, std::vector<unsigned>& degrees)
{
    for (unsigned i = 0; i < vertices; i++)
    {
        std::vector<unsigned> neighborDists; //Generate list of how far the neighbors are, vertex of the neighbor does not matter
        for (unsigned j = 0; j < vertices; j++) //Dist of (i, j) with i < j stored in distances[j(j - 1)/2 + i]
        {
            if (j < i && distances[i * (i - 1)/2 + j] < std::numeric_limits<unsigned>::max()) //If i and j are neighbors
            {
                neighborDists.push_back(distances[i * (i - 1)/2 + j]);
            }
            else if (j > i && distances[j * (j - 1)/2 + j] < std::numeric_limits<unsigned>::max())
            {
                neighborDists.push_back(distances[j * (j - 1)/2 + j]);
            }
        }
        std::sort(neighborDists.begin(), neighborDists.end(), std::greater<unsigned>()); //Sort the distances in descending order because grades should be in reverse lexicographic order
        AppearanceGrades iGrades; //Stores grades of appearance for vertex i
        unsigned minScale;
        for (int j = neighborDists.size() - 1; j >= 0;)
        {
            minScale = neighborDists[j];
            iGrades.push_back(Grade(-degrees[j + 1], minScale)); //If the scale parameter is >= minScale, then vertex i has j+1 neighbors. Negative because degree is sorted opposite of the usual ordering on R
            while (j >= 0 && neighborDists[j] == minScale) j--; //Iterate until the next distance is < minScale
        }
        update_grades(iGrades); //Makes sure all of them are incomparable after the binning
        multigrades.push_back(iGrades);
    }
}//end generateVertexMultigrades()

//Determines the grades of appearance of when both simplices exist subject to some minimal distance parameter mindist
//Grade arrays are assumed to be sorted in reverse lexicographic order, output will be sorted in reverse lexicographic order
void BifiltrationData::combineMultigrades(AppearanceGrades& merged, AppearanceGrades& grades1, AppearanceGrades& grades2, unsigned mindist)
{
    int maxScale = std::numeric_limits<int>::max();
    int max1 = maxScale, max2 = maxScale;
    int maxY, from; //from tells us which vector the grade we are considering comes from (1, 2, or 3(both))
    AppearanceGrades::iterator it1 = grades1.begin();
    AppearanceGrades::iterator it2 = grades2.begin();
    while (it1 != grades1.end() || it2 != grades2.end())
    {
        maxY = std::numeric_limits<int>::max();
        if (it1 != grades1.end())
        {
            maxY = it1->y;
            from = 1;
        }
        if (it2 != grades2.end() && maxY >= it2->y)
        {
            if (maxY == it2->y)
                from = 3;
            else
            {
                maxY = it2->y;
                from = 2;
            }
        }
        switch(from)
        {
        case 1:
        {
            int temp = maxScale; //Store maxScale for now
            max1 = std::max(it1->x, (int)mindist);
            maxScale = std::max(max1, max2);
            if (temp > maxScale) //max rightmost entry changed
            {
                merged.push_back(Grade(maxY, maxScale));
            }
            it1++;
            break;
        }
        case 2:
        {
            int temp = maxScale; //Store maxScale for now
            max2 = std::max(it2->x, (int)mindist);
            maxScale = std::max(max1, max2);
            if (temp > maxScale) //max rightmost entry changed
            {
                merged.push_back(Grade(maxY, maxScale));
            }
            it2++;
            break;
        }
        case 3:
        {
            int temp = maxScale; //Store maxScale for now
            max1 = std::max(it1->x, (int)mindist);
            max2 = std::max(it2->x, (int)mindist);
            maxScale = std::max(max1, max2);
            if (temp > maxScale) //max rightmost entry changed
            {
                merged.push_back(Grade(maxY, maxScale));
            }
            it1++;
            it2++;
            break;
        }
        }
        if (maxScale == (int)mindist)
            break;
    }
}//end combineMultigrades

//Takes a simplex and its grades of appearance and adds it to ordered_high_grades, ordered_grades, or ordered_low_grades
void BifiltrationData::addSimplicesToGrade(GradeInfo* orderedGrades, std::vector<int> simplex, AppearanceGrades& grades)
{
    GradeInfo::iterator ret;
    for (AppearanceGrades::iterator it = grades.begin(); it != grades.end(); it++)
    {
        ret = orderedGrades->find(*it);
        if (ret == orderedGrades->end()) //Grade not found
        {
            std::vector<std::vector<int> > simplexSet(1, simplex);
            orderedGrades->emplace(*it, simplexSet);
        }
        else //Grade found, pointed at by ret
        {
            ret->second.push_back(simplex);
        }
    }
}

//Assumes that the SimplexInfo hash tables are created
//Goes through the SimplexInfo hash tables and
//1. Makes sure that each of the simplex multigrades are incomparable
//2. Creates the inverse hashtable stored in GradeInfo
void BifiltrationData::createGradeInfo()
{
    GradeInfo::iterator ret;
    for (SimplexInfo::iterator it = ordered_low_simplices->begin(); it != ordered_low_simplices->end(); it++)
    {
        //Make sure the grades are OK
        update_grades(it->second);
        for (AppearanceGrades::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            ret = ordered_low_grades->find(*it2);
            if (ret == ordered_low_grades->end()) //Grade not found
            {
                std::vector<std::vector<int> > simplexSet(1, it->first);
                ordered_low_grades->emplace(*it2, simplexSet);
            }
            else //Grade found, pointed at by ret
            {
                ret->second.push_back(it->first);
            }
        }
    }

    for (SimplexInfo::iterator it = ordered_simplices->begin(); it != ordered_simplices->end(); it++)
    {
        //Make sure the grades are OK
        update_grades(it->second);
        for (AppearanceGrades::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            ret = ordered_grades->find(*it2);
            if (ret == ordered_grades->end()) //Grade not found
            {
                std::vector<std::vector<int> > simplexSet(1, it->first);
                ordered_grades->emplace(*it2, simplexSet);
            }
            else //Grade found, pointed at by ret
            {
                ret->second.push_back(it->first);
            }
        }
    }

    for (SimplexInfo::iterator it = ordered_high_simplices->begin(); it != ordered_high_simplices->end(); it++)
    {
        //Make sure the grades are OK
        update_grades(it->second);
        for (AppearanceGrades::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            ret = ordered_high_grades->find(*it2);
            if (ret == ordered_high_grades->end()) //Grade not found
            {
                std::vector<std::vector<int> > simplexSet(1, it->first);
                ordered_high_grades->emplace(*it2, simplexSet);
            }
            else //Grade found, pointed at by ret
            {
                ret->second.push_back(it->first);
            }
        }
    }
}

//Given a list of multigrades, sort them and remove all comparable multigrades
void BifiltrationData::update_grades(AppearanceGrades& grades)
{
    //Sort grades
    std::sort(grades.begin(), grades.end());

    //Iterate through the sorted grades and make sure they are all incomparable, delete the ones that are not
    for (AppearanceGrades::iterator it = grades.begin(); it != grades.end() - 1;)
    {
        if (it->x <= (it + 1)->x && it->y <= (it + 1)->y) //grades are comparable
            it = grades.erase(it + 1) - 1;
        else
            it++;
    }
}

//sets x_grades and y_grades as necessary
void BifiltrationData::set_xy_grades(unsigned num_x, unsigned num_y)
{
    x_grades = num_x;
    y_grades = num_y;
}

//returns the number of unique x-coordinates of the multi-grades
unsigned BifiltrationData::num_x_grades()
{
    return x_grades;
}

//returns the number of unique y-coordinates of the multi-grades
unsigned BifiltrationData::num_y_grades()
{
    return y_grades;
}

//returns the number of simplices of dimension (hom_dim-1), hom_dim, or (hom_dim+1)
int BifiltrationData::get_size(int dim)
{
    if(dim == hom_dim - 1)
        return ordered_low_simplices->size();
    else if(dim == hom_dim)
        return ordered_simplices->size();
    else if(dim == hom_dim + 1)
        return ordered_high_simplices->size();
    else
        return -1;
}

//returns the list of simplices and their grades of appearance in dimension (hom_dim-1), hom_dim, or (hom_dim+1)
//Returns NULL if incorrect dimension is given
SimplexInfo* BifiltrationData::getSimplices(int dim)
{
    if(dim == hom_dim - 1)
        return ordered_low_simplices;
    else if(dim == hom_dim)
        return ordered_simplices;
    else if(dim == hom_dim + 1)
        return ordered_high_simplices;
    else
        return NULL;
}

//returns the list of grades and their simplices in dimension (hom_dim-1), hom_dim, or (hom_dim+1)
//Returns NULL if incorrect dimension is given
GradeInfo* BifiltrationData::getGrades(int dim)
{
    if(dim == hom_dim - 1)
        return ordered_low_grades;
    else if(dim == hom_dim)
        return ordered_grades;
    else if(dim == hom_dim + 1)
        return ordered_high_grades;
    else
        return NULL;
}

//print bifiltration in the RIVET bifiltration input format
//prints simplices in no particular order, grades are in reverse-lexicographic order
void BifiltrationData::print_bifiltration()
{
    for (SimplexInfo::const_iterator it = ordered_low_simplices->begin(); it != ordered_low_simplices->end(); it++)
    {
        for (std::vector<int>::const_iterator it2 = it->first.begin(); it2 != it->first.end(); it2++)
        {
            std::cout << *it2 << " ";
        }
        std::cout << "; "; //Separator of vertex and grade info
        for (AppearanceGrades::const_iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            std::cout << it2->x << " " << it2->y << " ";
        }
        std:: cout << std::endl;
    }

    for (SimplexInfo::const_iterator it = ordered_simplices->begin(); it != ordered_simplices->end(); it++)
    {
        for (std::vector<int>::const_iterator it2 = it->first.begin(); it2 != it->first.end(); it2++)
        {
            std::cout << *it2 << " ";
        }
        std::cout << "; "; //Separator of vertex and grade info
        for (AppearanceGrades::const_iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            std::cout << it2->x << " " << it2->y << " ";
        }
        std:: cout << std::endl;
    }

    for (SimplexInfo::const_iterator it = ordered_high_simplices->begin(); it != ordered_high_simplices->end(); it++)
    {
        for (std::vector<int>::const_iterator it2 = it->first.begin(); it2 != it->first.end(); it2++)
        {
            std::cout << *it2 << " ";
        }
        std::cout << "; "; //Separator of vertex and grade info
        for (AppearanceGrades::const_iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            std::cout << it2->x << " " << it2->y << " ";
        }
        std:: cout << std::endl;
    }
}
