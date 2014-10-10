/* InputManager class
 */

#include "input_manager.h"

#include <set>
#include <QDebug>

#include <boost/algorithm/string.hpp>


//epsilon value for use in comparisons
double ExactValue::epsilon = pow(2,-30);


//helper function to convert a string to an exact (rational)
//accepts string such as "12.34", "765", and "-10.8421"
exact str_to_exact(std::string& str)
{
    exact r;

    //find decimal point, if it exists
    int dec = str.find(".");

    if(dec == std::string::npos)	//then decimal point not found
    {
        r = exact(str);
    }
    else	//then decimal point found
    {
        std::string whole = str.substr(0,dec);
        std::string frac = str.substr(dec+1);
        unsigned exp = frac.length();

        std::istringstream s(whole + frac);
        boost::multiprecision::cpp_int num;
        s >> num;
        boost::multiprecision::cpp_int ten = 10;
        boost::multiprecision::cpp_int denom = boost::multiprecision::pow(ten,exp);
        r = exact(num, denom);
    }

    return r;
}



//===============================================================================================//


//constructor
InputManager::InputManager(int d, int v) :
    verbosity(v), hom_dim(d),
    y_squared(false),
    simplex_tree(d, v)
{ }

//function to run the input manager, requires a filename
void InputManager::start(std::string filename, unsigned x_bins, unsigned y_bins)
{
	//read the file
    if(verbosity >= 2) { std::cout << "READING FILE: " << filename << "\n"; }
	std::string line;
    infile.open(filename.data());
	if(infile.is_open())
	{
        //determine what type of file it is
		std::getline(infile,line);
		std::istringstream iss(line);
		std::string filetype;
		iss >> filetype;
		
		//call appropriate handler function
		if(filetype == "points")
		{
            read_point_cloud(x_bins, y_bins);
		}
		else if(filetype == "bifiltration")
		{
			read_bifiltration();
		}
		else
		{
			std::cout << "Error: Unrecognized file type.\n";
			throw std::exception();
		}
	}
	else
	{
        std::cout << "Error: Unable to open file " << filename << ".\n";
		throw std::exception();
	}
	
	infile.close();
	
}//end start()

//functions to return grade values
std::vector<double> InputManager::get_x_grades()
{
    return x_grades;
}

std::vector<exact> InputManager::get_x_exact()
{
    return x_exact;
}

std::vector<double> InputManager::get_y_grades()
{
    return y_grades;
}

std::vector<exact> InputManager::get_y_exact()
{
    return y_exact;
}

bool InputManager::y_values_squared()
{
    return y_squared;
}

//returns a pointer to the simplex tree representing the bifiltration
SimplexTree* InputManager::get_bifiltration()
{
    return &simplex_tree;
}


//reads a point cloud
//  points are given by coordinates in Euclidean space, and each point has a birth time
//  constructs a simplex tree representing the bifiltered Vietoris-Rips complex
void InputManager::read_point_cloud(unsigned x_bins, unsigned y_bins)
{
	if(verbosity >= 2) { std::cout << "  Found a point cloud file.\n"; }

    y_squared = true;   //we will store the SQUARES of the distance values
	
  /* step 1: read data file and store exact (rational) values */

	//prepare (temporary) data structures
    int dimension;              //integer dimension of data
    double max_dist;            //maximum distance for edges in Vietoris-Rips complex
    std::vector<ExactPoint> points;	//create for points
	std::string line;		//string to hold one line of input
		
	//read dimension of the points from the first line of the file
	std::getline(infile,line);
	std::stringstream(line) >> dimension;
    if(verbosity >= 4) { std::cout << "  dimension of data: " << dimension << "; max dimension of simplices: " << hom_dim << "\n"; }
	
	//read maximum distance for edges in Vietoris-Rips complex
	std::getline(infile,line);
	std::stringstream(line) >> max_dist;
	if(verbosity >= 4) { std::cout << "  maximum distance: " << max_dist << "\n"; }
		
	//read points
	while( std::getline(infile,line) )
	{
		//parse current point from string
        std::vector<std::string> coords;
        coords.reserve(dimension + 1);

        boost::algorithm::trim(line);
        boost::split(coords, line, boost::is_any_of("\t "), boost::token_compress_on);
        ExactPoint p(coords);
		
		//add current point to the vector
		points.push_back(p);
	}
	
	if(verbosity >= 4) { std::cout << "  read " << points.size() << " points; input finished\n"; }

    //sort the points -- THIS IS UNNECESSARY, RIGHT???
//	if(verbosity >= 2) { std::cout << "SORTING POINTS BY BIRTH TIME\n"; }
//	sort(points.begin(), points.end());
	
	//test points vector
	if(verbosity >= 6) 
	{
		std::cout << "TESTING VECTOR:\n";
        for(unsigned i=0; i<points.size(); i++)
		{
            ExactPoint p = points[i];
			std::cout << "  point " << i << ": (";
            for(int i=0; i<p.coords.size(); i++)
			{
                std::cout << p.coords[i];
                if(i<p.coords.size()-1) { std::cout << ", "; }
			}
            std::cout << ") born at time " << p.birth << "\n";
		}
		std::cout << "  found " << points.size() << " points\n";
	}

  /* step 2: compute distance matrix, and create ordered lists of all unique distance and time values */

    if(verbosity >= 2) { std::cout << "BUILDING DISTANCE AND TIME LISTS:\n"; }

    unsigned num_points = points.size();

    typedef std::set<ExactValue*, ExactValueComparator> ExactSet;
    ExactSet dist_set;  //stores all unique time values; must DELETE all elements later
    ExactSet time_set;  //stores all unique distance values; must DELETE all elements later
    std::pair<ExactSet::iterator, bool> ret;    //for return value upon insert()

    dist_set.insert(new ExactValue(exact(0)));

    double max_dist_squared = max_dist*max_dist;

    //consider all points
    for(unsigned i=0; i<num_points; i++)
    {
        //store time value, if it doesn't exist already
        ret = time_set.insert(new ExactValue(points[i].birth));

        //remember that point i has this birth time value
        (*(ret.first))->indexes.push_back(i);

        //compute distances from this point to all following points
        for(int j=i+1; j<num_points; j++)
        {
            //compute distance squared between points[i] and points[j]
            exact dist_squared(0);
            for(int k=0; k < dimension; k++)
                dist_squared += (points[i].coords[k] - points[j].coords[k])*(points[i].coords[k] - points[j].coords[k]);

            double cur_dist_squared = numerator(dist_squared).convert_to<double>() / denominator(dist_squared).convert_to<double>();
            if( cur_dist_squared <= max_dist_squared ) //then this distance is allowed  --  TODO: MAKE THIS EXACT???
            {
                //store distance value, if it doesn't exist already
                ret = dist_set.insert(new ExactValue(dist_squared));

                //remember that the pair of points (i,j) has this distance value, which will go in entry j(j-1)/2 + i
                (*(ret.first))->indexes.push_back( (j*(j-1))/2 + i );
            }
        }
    }

    //convert distance and time sets to vectors
    // also build vectors of discrete indexes for constructing the bifiltration

    unsigned max_unsigned = std::numeric_limits<unsigned>::max();
    std::vector<unsigned> time_indexes(num_points, max_unsigned);   //max_unsigned shall represent undefined time (is this reasonable?)
    std::vector<unsigned> dist_indexes((num_points*(num_points-1))/2, max_unsigned);  //max_unsigned shall represent undefined distance

    //first, times
    if(x_bins == 0 || x_bins >= time_set.size())    //then don't use bins
    {
        x_grades.reserve(time_set.size());
        x_exact.reserve(time_set.size());

        unsigned c = 0;  //counter for time indexes
        for(ExactSet::iterator it=time_set.begin(); it!=time_set.end(); ++it)   //loop through all UNIQUE time values
        {
            x_grades.push_back( (*it)->double_value );
            x_exact.push_back( (*it)->exact_value );

            for(unsigned i = 0; i < (*it)->indexes.size(); i++ ) //loop through all point indexes for this time value
                time_indexes[ (*it)->indexes[i] ] = c;   //store time index

            c++;
        }
    }
    else    //then use bins: then the list size will equal the number of bins, and x-values will be equally spaced
    {
        //compute bin size
        exact x_min = (*time_set.begin())->exact_value;
        exact x_max = (*time_set.rbegin())->exact_value;
        exact x_bin_size = (x_max - x_min)/x_bins;

        //store bin values
        x_grades.reserve(x_bins);
        x_exact.reserve(x_bins);

        ExactSet::iterator it = time_set.begin();
        for(unsigned c = 0; c < x_bins; c++)    //loop through all bins
        {
            ExactValue cur_bin(x_min + (c+1)*x_bin_size);    //store the bin value (i.e. the right endpoint of the bin interval)
            x_grades.push_back(cur_bin.double_value);
            x_exact.push_back(cur_bin.exact_value);

            //store bin index for all points whose time value is in this bin
            while( it != time_set.end() && **it <= cur_bin )
            {
                for(unsigned i = 0; i < (*it)->indexes.size(); i++ ) //loop through all point indexes for this time value
                    time_indexes[ (*it)->indexes[i] ] = c;   //store time index
                ++it;
            }
        }
    }

    //now, distances
    if(y_bins == 0 || y_bins >= dist_set.size())    //then don't use bins
    {
        y_grades.reserve(dist_set.size());
        y_exact.reserve(dist_set.size());

        unsigned c = 0;  //counter for distance indexes
        for(ExactSet::iterator it=dist_set.begin(); it!=dist_set.end(); ++it)   //loop through all UNIQUE distance values
        {
            y_grades.push_back( (*it)->double_value );
            y_exact.push_back( (*it)->exact_value );

            for(unsigned i = 0; i < (*it)->indexes.size(); i++ ) //loop through all pair-indexes for this distance value
                dist_indexes[ (*it)->indexes[i] ] = c;   //store distance index
            c++;
        }
    }
    else    //then use bins: then the list size will equal the number of bins, and y-values will be equally spaced
    {
        //compute bin size: min distance is 0, so bin size is sqrt(max distance)/y_bins
        exact y_max = (*dist_set.rbegin())->exact_value;

        //store bin values
        y_grades.reserve(y_bins);
        y_exact.reserve(y_bins);

        ExactSet::iterator it = dist_set.begin();
        for(unsigned c = 0; c < y_bins; c++)    //loop through all bins
        {
            //FIXED THE FOLLOWING LINE -- VERIFY!!!
            ExactValue cur_bin( (c+1)*(c+1)*y_max/(y_bins*y_bins) );    //store the bin value (i.e. this is the SQUARE of the right endpoint of the bin interval)
            y_grades.push_back(cur_bin.double_value);
            y_exact.push_back(cur_bin.exact_value);

            //store bin index for all pair-indexes whose distance value is in this bin
            while( it != dist_set.end() && **it <= cur_bin )
            {
                for(unsigned i = 0; i < (*it)->indexes.size(); i++ ) //loop through all pair-indexes for this distance value
                    dist_indexes[ (*it)->indexes[i] ] = c;   //store distance index
                ++it;
            }
            //TODO: TEST END CASE TO MAKE SURE THAT WE GET ALL THE DISTANCE VALUES THAT WE SHOULD GET
        }
    }


    //======= TESTING ONLY =======
    std::cout << "CHECK: (time) x_grades.size() = " << x_grades.size() << ", x_exact.size() = " << x_exact.size() << ", time_indexes.size() = " << time_indexes.size() << ", x_bins = " << x_bins << "\n";
    std::cout << "  all x-grades: \n";
    for(unsigned i=0; i<x_grades.size(); i++)
        std::cout << "    " << i << ": " << x_grades[i] << " = " << x_exact[i] << "\n";
    std::cout << "  x-indexes of points: ";
    for(unsigned i=0; i<time_indexes.size(); i++)
        std::cout << time_indexes[i] << " ";
    std::cout << "\n";

    std::cout << "CHECK: (distance) y_grades.size() = " << y_grades.size() << ", y_exact.size() = " << y_exact.size() << ", dist_indexes.size() = " << dist_indexes.size() << ", y_bins = " << y_bins << "\n";
    std::cout << "  all y-grades: \n";
    for(unsigned i=0; i<y_grades.size(); i++)
        std::cout << "    " << i << ": " << y_grades[i] << " = " << y_exact[i] << "\n";
    std::cout << "  y-indexes of pairs:\n";
    for(unsigned i=0; i<num_points-1; i++)
        for(unsigned j=i+1; j<num_points; j++)
            std::cout << "    dist(" << i << ", " << j << ") = " << dist_indexes[ (j*(j-1))/2 + i ] << "\n";
    std::cout << "\n";

  /* step 3: build the bifiltration */

    //simplex_tree will store only DISCRETE information!!!
    //this only requires (suppose there are k points):
    //  1. a list of k discrete times
    //  2. a list of k(k-1)/2 discrete distances
    //  3. max dimension of simplices to construct

    if(verbosity >= 2) { std::cout << "BUILDING VIETORIS-RIPS BIFILTRATION\n"; }

    simplex_tree.build_VR_complex(time_indexes, dist_indexes, x_grades.size(), y_grades.size());

    //clean up
    for(ExactSet::iterator it = time_set.begin(); it != time_set.end(); ++it)
    {
        ExactValue* p = *it;
        time_set.erase(it);
        delete p;
    }
    for(ExactSet::iterator it = dist_set.begin(); it != dist_set.end(); ++it)
    {
        ExactValue* p = *it;
        dist_set.erase(it);
        delete p;
    }
}//end read_point_cloud()

//reads a bifiltration and constructs a simplex tree
void InputManager::read_bifiltration()
{
	if(verbosity >= 2) { std::cout << "  Found a bifiltration file.\n"; }
/* THIS MUST BE UPDATED!!!
	//prepare (temporary) data structures
	std::string line;		//string to hold one line of input
	
	//read simplices
	while( std::getline(infile,line) )
	{
		std::istringstream iss(line);
		
		//read dimension of simplex
		int dim;
		iss >> dim;
		
		//read vertices
		std::vector<int> verts;
		for(int i=0; i<=dim; i++)
		{
			int v;
			iss >> v;
			verts.push_back(v);
		}
		
		//read multi-index
		int time, dist;
		iss >> time;
		iss >> dist;
		
		if(verbosity >= 4)
		{
			std::cout << "    simplex [" << verts[0];
			for(int i=1; i<=dim; i++)
				std::cout  << " " << verts[i];
			std::cout << "] with multi-index (" << time << ", " << dist << ")\n"; 
		}
		
		//add the simplex to the simplex tree
		simplex_tree.add_simplex(verts, time, dist);
	}

    //compute indexes
    simplex_tree.update_global_indexes();
    simplex_tree.update_dim_indexes();
*/
}//end read_bifiltration()


