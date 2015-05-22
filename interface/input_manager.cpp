/* InputManager class
 */

#include "input_manager.h"

#include "../computationthread.h"
#include "input_parameters.h"
#include "../math/simplex_tree.h"

#include <QDebug>
#include <QString>

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <set>
#include <sstream>


//epsilon value for use in comparisons
double ExactValue::epsilon = pow(2,-30);


//helper function to convert a string to an exact (rational)
//accepts string such as "12.34", "765", and "-10.8421"
exact str_to_exact(std::string str)
{
    exact r;

    //find decimal point, if it exists
    std::string::size_type dec = str.find(".");

    if(dec == std::string::npos)	//then decimal point not found
    {
        r = exact(str);
    }
    else	//then decimal point found
    {
        //get whole part and fractional part
        std::string whole = str.substr(0,dec);
        std::string frac = str.substr(dec+1);
        unsigned exp = frac.length();

        //test for negative, and remove minus sign character
        bool neg = false;
        if(whole.length() > 0 && whole[0] == '-')
        {
            neg = true;
            whole.erase(0, 1);
        }

        //remove leading zeros (otherwise, c++ thinks we are using octal numbers)
        std::string num_str = whole + frac;
        boost::algorithm::trim_left_if(num_str, boost::is_any_of("0"));

        //now it is safe to convert to rational
        std::istringstream s(num_str);
        boost::multiprecision::cpp_int num;
        s >> num;
        boost::multiprecision::cpp_int ten = 10;
        boost::multiprecision::cpp_int denom = boost::multiprecision::pow(ten,exp);

        r = exact(num, denom);
        if(neg)
            r = -1*r;
    }
    return r;
}



//==================== InputManager class ====================


//constructor
InputManager::InputManager(ComputationThread* cthread) :
    cthread(cthread),
    input_params(cthread->params),
    verbosity(cthread->verbosity),
    hom_dim(input_params.dim),
    infile(input_params.fileName),
    x_grades(cthread->x_grades), x_exact(cthread->x_exact),
    y_grades(cthread->y_grades), y_exact(cthread->y_exact),
    simplex_tree(cthread->bifiltration)
{ }

//function to run the input manager, requires a filename
//  post condition: x_grades and x_exact have size x_bins, and they contain the grade values for the 2-D persistence module in double and exact form (respectively)
//                  similarly for y_grades and y_exact
void InputManager::start()
{
	//read the file
    if(verbosity >= 2) { qDebug() << "READING FILE:" << input_params.fileName; }
    if(infile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
        FileInputReader reader(infile);

        //determine what type of file it is
        QString filetype = reader.next_line().first();  ///TODO: error handling -- the file must have at least one line with non-whitespace characters
		
		//call appropriate handler function
        if(filetype == QString("points"))
		{
            read_point_cloud(reader);
		}
        else if(filetype == QString("bifiltration"))
		{
            read_bifiltration(reader);
		}
        else if(filetype == QString("RIVET_0"))
        {
            read_RIVET_data(reader);
        }
        else
		{
            qDebug() << "Error: Unrecognized file type.";
			throw std::exception();
		}
	}
	else
	{
        qDebug() << "Error: Unable to open file: " << input_params.fileName;
		throw std::exception();
	}
	
}//end start()


//reads a point cloud
//  points are given by coordinates in Euclidean space, and each point has a "birth time"
//  constructs a simplex tree representing the bifiltered Vietoris-Rips complex
void InputManager::read_point_cloud(FileInputReader& reader)
{
    if(verbosity >= 2) { qDebug() << "  Found a point cloud file."; }
	
  // STEP 1: read data file and store exact (rational) values

    //read dimension of the points from the first line of the file
    int dimension = reader.next_line().first().toInt();
    if(verbosity >= 4) { qDebug() << "  dimension of data:" << dimension << "; max dimension of simplices: " << (hom_dim + 1); }

    //read maximum distance for edges in Vietoris-Rips complex
    exact max_dist = str_to_exact(reader.next_line().first().toStdString());  ///TODO: don't convert to std::string
    if(verbosity >= 4)
    {
        std::ostringstream oss;
        oss << max_dist;
        qDebug().noquote() << "  maximum distance:" << QString::fromStdString(oss.str());
    }

    //read points
    std::vector<ExactPoint> points;
    while( reader.has_next() )
    {
        QStringList tokens = reader.next_line();
        ExactPoint p(tokens);
        points.push_back(p);
    }

    if(verbosity >= 4) { qDebug() << "  read " << points.size() << " points; input finished"; }


  // STEP 2: compute distance matrix, and create ordered lists of all unique distance and time values

    if(verbosity >= 2) { qDebug() << "BUILDING DISTANCE AND TIME LISTS"; }
    cthread->advanceProgressStage();

    unsigned num_points = points.size();

    typedef std::set<ExactValue*, ExactValueComparator> ExactSet;
    ExactSet dist_set;  //stores all unique time values; must DELETE all elements later
    ExactSet time_set;  //stores all unique distance values; must DELETE all elements later
    std::pair<ExactSet::iterator, bool> ret;    //for return value upon insert()

    dist_set.insert(new ExactValue(exact(0)));

    //consider all points
    for(unsigned i=0; i<num_points; i++)
    {
        //store time value, if it doesn't exist already
        ret = time_set.insert(new ExactValue(points[i].birth));

        //remember that point i has this birth time value
        (*(ret.first))->indexes.push_back(i);

        //compute distances from this point to all following points
        for(unsigned j=i+1; j<num_points; j++)
        {
            //compute distance squared between points[i] and points[j]
            exact dist_squared(0);
            for(int k=0; k < dimension; k++)
                dist_squared += (points[i].coords[k] - points[j].coords[k])*(points[i].coords[k] - points[j].coords[k]);

            //find an approximate square root of the exact dist_squared, and store it as an exact value
            double fp_dist_squared = numerator(dist_squared).convert_to<double>() / denominator(dist_squared).convert_to<double>();
            exact cur_dist(0);
            if(fp_dist_squared > 0)
                cur_dist = approx( sqrt(fp_dist_squared) ); //OK for now...

            if( cur_dist <= max_dist ) //then this distance is allowed
            {
                //store distance value, if it doesn't exist already
                ret = dist_set.insert(new ExactValue(cur_dist));

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
    if(input_params.x_bins == 0 || input_params.x_bins >= time_set.size())    //then don't use bins
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
        exact x_bin_size = (x_max - x_min)/input_params.x_bins;

        //store bin values
        x_grades.reserve(input_params.x_bins);
        x_exact.reserve(input_params.x_bins);

        ExactSet::iterator it = time_set.begin();
        for(unsigned c = 0; c < input_params.x_bins; c++)    //loop through all bins
        {
            ExactValue cur_bin( x_min + (c+1)*x_bin_size );    //store the bin value (i.e. the right endpoint of the bin interval)
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
    if(input_params.y_bins == 0 || input_params.y_bins >= dist_set.size())    //then don't use bins
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
        //compute bin size: min distance is 0, so bin size is (max distance)/y_bins
        exact y_bin_size = ((*dist_set.rbegin())->exact_value)/input_params.y_bins;

        //store bin values
        y_grades.reserve(input_params.y_bins);
        y_exact.reserve(input_params.y_bins);

        ExactSet::iterator it = dist_set.begin();
        for(unsigned c = 0; c < input_params.y_bins; c++)    //loop through all bins
        {
            ExactValue cur_bin( (c+1)*y_bin_size );    //store the bin value (i.e. the right endpoint of the bin interval)
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

    //update progress
    cthread->setCurrentProgress(30);


  // STEP 3: build the bifiltration

    //simplex_tree stores only DISCRETE information!
    //this only requires (suppose there are k points):
    //  1. a list of k discrete times
    //  2. a list of k(k-1)/2 discrete distances
    //  3. max dimension of simplices to construct

    if(verbosity >= 2) { qDebug() << "BUILDING VIETORIS-RIPS BIFILTRATION"; }

    simplex_tree.build_VR_complex(time_indexes, dist_indexes, x_grades.size(), y_grades.size());

    //clean up
    for(ExactSet::iterator it = time_set.begin(); it != time_set.end(); ++it)
    {
        ExactValue* p = *it;
        delete p;
    }
    for(ExactSet::iterator it = dist_set.begin(); it != dist_set.end(); ++it)
    {
        ExactValue* p = *it;
        delete p;
    }
}//end read_point_cloud()

//reads a bifiltration and constructs a simplex tree
void InputManager::read_bifiltration(FileInputReader& reader)
{
    if(verbosity >= 2) { qDebug() << "  Found a bifiltration file. CANNOT CURRENTLY READ BIFILTRATION FILES!\n"; }
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
		
		//add the simplex to the simplex tree
		simplex_tree.add_simplex(verts, time, dist);
	}

    //compute indexes
    simplex_tree.update_global_indexes();
    simplex_tree.update_dim_indexes();
*/
}//end read_bifiltration()

//reads a file of previously-computed data from RIVET
void InputManager::read_RIVET_data(FileInputReader& reader)
{
    //read parameters
    cthread->params.dim = reader.next_line().first().toInt();
    cthread->params.x_label = reader.next_line().first();
    cthread->params.y_label = reader.next_line().first();

    //read x-grades


    //read y-grades


    //read xi values


    //read barcode templates


}//end read_RIVET_data()


//finds a rational approximation of a floating-point value
// precondition: x > 0
exact InputManager::approx(double x)
{
    int d = 5;	//desired number of significant digits
    int log = (int) floor( log10(x) ) + 1;

    if(log >= d)
        return exact( (int) floor(x) );

    int denom = pow(10, d-log);
    return exact( (int) floor(x*denom), denom);
}

//==================== FileInputReader class ====================

InputManager::FileInputReader::FileInputReader(QFile& file) :
    in(&file),
    next_line_found(false)
{
    find_next();
}

void InputManager::FileInputReader::find_next()
{
    QChar comment('#');
    while( !in.atEnd() && !next_line_found )
    {
        QString line = in.readLine().trimmed();
        if( line.isEmpty() || line.at(0) == comment)    //then skip this line
            continue;
        //else -- found a nonempty next line
        next_line_found = true;
        next_line_tokens = line.split(QRegExp("\\s+"));
    }
}

bool InputManager::FileInputReader::has_next()
{
    return next_line_found;
}

QStringList InputManager::FileInputReader::next_line()   ///TODO: maybe too much copying of QStringLists
{
    QStringList current = next_line_tokens;

    next_line_found = false;
    find_next();

    return current;
}
