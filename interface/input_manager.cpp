/* InputManager class
 */

#include "input_manager.h"
#include "../computation.h"
#include "file_input_reader.h"
#include "input_parameters.h"
#include "../math/simplex_tree.h"

#include "debug.h"

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <set>
#include <sstream>
#include <vector>
#include <boost/algorithm/string.hpp>

//epsilon value for use in comparisons
double ExactValue::epsilon = pow(2,-30);

// function for determining whether or not a string is a number
bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && (std::isdigit(*it) || *it =='.')) ++it;
    return !s.empty() && it == s.end();
}

std::string join(const std::vector<std::string> &strings) {
  std::stringstream ss;
  for(int i = 0; i < strings.size(); i++) {
    if (i > 0)
      ss << " ";
    ss << strings[i];
  }
  return ss.str();
}

std::vector<std::string> split(std::string& str, std::string separators) {
  std::vector<std::string> strings;
  boost::split(strings, str, boost::is_any_of(separators));
  return strings;
}

//helper function to convert a string to an exact (rational)
//accepts string such as "12.34", "765", and "-10.8421"
exact str_to_exact(std::string str)
{
    if(!is_number(str)){
  	 	debug()<<"Error: "<<str<<" is not a number"<<std::endl;
    	return 0;
    }
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

class TokenReader {
public:
  TokenReader(FileInputReader &reader): reader(reader) { }
  bool has_next_token() {
    if (it == tokens.end()) {
      tokens = reader.next_line();
      it = tokens.begin();
    }
    return it != tokens.end();
  }

  std::string next_token() {
    if (has_next_token()) {
      return *it;
    }
    return "";
  }

private:
  FileInputReader &reader;
  std::vector<std::string> tokens;
  std::vector<std::string>::iterator it;
};

//==================== InputManager class ====================


//constructor
InputManager::InputManager(InputParameters &params) :
    input_params(params),
    verbosity(params.verbosity),
    hom_dim(input_params.dim)
{
    register_file_type(FileType {"points", "point-cloud data", true,
                               std::bind(&InputManager::read_point_cloud, this, std::placeholders::_1, std::placeholders::_2) });

    register_file_type(FileType {"metric", "metric data", true,
                                 std::bind(&InputManager::read_discrete_metric_space, this, std::placeholders::_1, std::placeholders::_2) });
    register_file_type(FileType {"bifiltration", "bifiltration data", true,
                                 std::bind(&InputManager::read_bifiltration, this, std::placeholders::_1, std::placeholders::_2) });
    register_file_type(FileType {"RIVET_0", "pre-computed RIVET data", false,
                                 std::bind(&InputManager::read_RIVET_data, this, std::placeholders::_1, std::placeholders::_2) });
};

void InputManager::register_file_type(FileType file_type) {
    supported_types.push_back(file_type);
}
FileType& InputManager::get_file_type(std::string fileName) {
    std::ifstream stream(fileName);
    if (!stream.is_open()) {
        throw std::runtime_error("Could not open " + fileName);
    }
    FileInputReader reader(stream);
    std::string filetype_name = reader.next_line()[0];

    auto it = std::find_if(supported_types.begin(), supported_types.end(), [filetype_name](FileType t) { return t.identifier == filetype_name; });

    if (it == supported_types.end()) {
        std::stringstream ss;
        ss << "Unsupported file type: " << filetype_name;
        throw std::runtime_error(ss.str());
    }

    return *it;
}


//function to run the input manager, requires a filename
//  post condition: x_grades and x_exact have size x_bins, and they contain the grade values for the 2-D persistence module in double and exact form (respectively)
//                  similarly for y_grades and y_exact
std::shared_ptr<InputData> InputManager::start(Progress &progress)
{
	//read the file
  if(verbosity >= 2) { debug() << "READING FILE:" << input_params.fileName << std::endl; }
    auto file_type = get_file_type(input_params.fileName);
        std::ifstream infile(input_params.fileName);                   //input file
        if (!infile.is_open()) {
            throw std::runtime_error("Could not open input file");
        }
    auto data = file_type.parser(infile, progress);
    data->is_data = file_type.is_data;
    debug() << "Set is_data to " << data->is_data << std::endl;
    return data;
}//end start()


//reads a point cloud
//  points are given by coordinates in Euclidean space, and each point has a "birth time"
//  constructs a simplex tree representing the bifiltered Vietoris-Rips complex
InputData* InputManager::read_point_cloud(std::ifstream &stream, Progress &progress)
{
    FileInputReader reader(stream);
    auto data = new InputData();
    if(verbosity >= 6) { debug() << "  Found a point cloud file." << std::endl; }

  // STEP 1: read data file and store exact (rational) values
//skip first line
    reader.next_line();
    //read dimension of the points from the first line of the file
    std::vector<std::string> dimension_line = reader.next_line();
    if (dimension_line.size() != 1)
    {
    	debug() << "There was more than one value in the expected dimension line.  There may be a problem with your input file.  " << std::endl;
    }
    debug() << "Dimension: " << dimension_line[0] << std::endl;
    int dimension = std::stoi(dimension_line[0]);

    //check for invalid input
    if (dimension == 0)
    {
    	debug() << "An invalid input was received for the dimension." << std::endl;
    	// throw an exception
    }

    //read maximum distance for edges in Vietoris-Rips complex
    std::vector<std::string> distance_line = reader.next_line();
    if (distance_line.size() != 1)
    {
    	debug() << "There was more than one value in the expected distance line.  There may be a problem with your input file.  " << std::endl;
    }

    exact max_dist = str_to_exact(distance_line[0]);
    if (max_dist == 0)
    {
    	throw std::runtime_error("An invalid input was received for the max distance.");
    }


    if(verbosity >= 4)
    {
        std::ostringstream oss;
        oss << max_dist;
        debug() << "  maximum distance: " << oss.str() << std::endl;
    }

    //read label for x-axis
    input_params.x_label = reader.next_line()[0];

    //set label for y-axis to "distance"
    input_params.y_label = "distance";

    //read points
    std::vector<DataPoint> points;
    while( reader.has_next_line() )
    {
        std::vector<std::string> tokens = reader.next_line();
        if (tokens.size() != dimension + 1 )
        {
        	// TODO: need a check for characters in the point data
        	// look up qexception object
        	// handle in dataselectDialogue
        	continue;
        }
        DataPoint p(tokens);
        points.push_back(p);
    }

    if(verbosity >= 4) { debug() << "  read" << points.size() << "points; input finished" << std::endl; }

    if (points.empty()) {
        throw std::runtime_error("No points loaded");
    }

  // STEP 2: compute distance matrix, and create ordered lists of all unique distance and time values

    if(verbosity >= 6) { debug() << "BUILDING DISTANCE AND TIME LISTS" << std::endl; }
    progress.advanceProgressStage();

    unsigned num_points = points.size();

    ExactSet dist_set;  //stores all unique distance values; must DELETE all elements later
    ExactSet time_set;  //stores all unique time values; must DELETE all elements later
    std::pair<ExactSet::iterator, bool> ret;    //for return value upon insert()

    dist_set.insert(new ExactValue(exact(0)));  //distance from a point to itself is always zero

    //consider all points
    for(unsigned i=0; i<num_points; i++)
    {
        //store time value, if it doesn't exist already
        ret = time_set.insert(new ExactValue(points[i].birth));

        //remember that point i has this birth time value
        (*(ret.first))->indexes.push_back(i);

        //compute (approximate) distances from this point to all following points
        for(unsigned j=i+1; j<num_points; j++)
        {
            //compute (approximate) distance squared between points[i] and points[j]
            double fp_dist_squared = 0;
            for(int k=0; k < dimension; k++)
                fp_dist_squared += (points[i].coords[k] - points[j].coords[k])*(points[i].coords[k] - points[j].coords[k]);

            //find an approximate square root of dist_squared, and store it as an exact value
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
    }//end for

  // STEP 3: build vectors of discrete indexes for constructing the bifiltration

    unsigned max_unsigned = std::numeric_limits<unsigned>::max();

    //first, times
    std::vector<unsigned> time_indexes(num_points, max_unsigned);   //vector of discrete time indexes for each point; max_unsigned shall represent undefined time (is this reasonable?)
    build_grade_vectors(*data, time_set, time_indexes, data->x_grades, data->x_exact, input_params.x_bins);

    //second, distances
    std::vector<unsigned> dist_indexes((num_points*(num_points-1))/2, max_unsigned);  //discrete distance matrix (triangle); max_unsigned shall represent undefined distance
    build_grade_vectors(*data, dist_set, dist_indexes, data->y_grades, data->y_exact, input_params.y_bins);

    //update progress
    progress.progress(30);


  // STEP 4: build the bifiltration

    //simplex_tree stores only DISCRETE information!
    //this only requires (suppose there are k points):
    //  1. a list of k discrete times
    //  2. a list of k(k-1)/2 discrete distances
    //  3. max dimension of simplices to construct, which is one more than the dimension of homology to be computed

    if(verbosity >= 6) { debug() << "BUILDING VIETORIS-RIPS BIFILTRATION"; }

    debug() << "x grades: " << data->x_grades.size() << " y grades: " << data->y_grades.size() << std::endl;
    data->simplex_tree.reset(new SimplexTree(input_params.dim, input_params.verbosity));
    data->simplex_tree->build_VR_complex(time_indexes, dist_indexes, data->x_grades.size(), data->y_grades.size());

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
    return data;
}//end read_point_cloud()

//reads data representing a discrete metric space with a real-valued function and constructs a simplex tree
std::shared_ptr<InputData> InputManager::read_discrete_metric_space(std::ifstream &stream, Progress &progress)
{
    if(verbosity >= 2) { debug() << "  Found a discrete metric space file."; }
    std::shared_ptr<InputData> data(new InputData);
    FileInputReader reader(stream);
  // STEP 1: read data file and store exact (rational) values of the function for each point

    //first read the label for x-axis
    input_params.x_label = reader.next_line()[0];

    //now read the values
    std::vector<std::string> line = reader.next_line();
    std::vector<exact> values;
    values.reserve(line.size());

    for(int i = 0; i < line.size(); i++)
    {
        values.push_back(str_to_exact(line.at(i)));
    }


  // STEP 2: read data file and store exact (rational) values for all distances

    //first read the label for y-axis
    input_params.y_label = join(reader.next_line());

    //read the maximum length of edges to construct
    exact max_dist = str_to_exact(reader.next_line()[0]);
    if(verbosity >= 4)
    {
        std::ostringstream oss;
        oss << max_dist;
        debug() << "  maximum distance:" << oss.str();
    }

    //prepare data structures
    ExactSet value_set;     //stores all unique values of the function; must DELETE all elements later
    ExactSet dist_set;      //stores all unique values of the distance metric; must DELETE all elements later
    std::pair<ExactSet::iterator, bool> ret;    //for return value upon insert()

    dist_set.insert(new ExactValue(exact(0)));  //distance from a point to itself is always zero

    //consider all points
    unsigned num_points = values.size();
    for(unsigned i = 0; i < num_points; i++)
    {
        //store value, if it doesn't exist already
        ret = value_set.insert(new ExactValue(values[i]));

        //remember that point i has this value
        (*(ret.first))->indexes.push_back(i);

        //read distances from this point to all following points
        if(i < num_points - 1)  //then there is at least one point after point i, and there should be another line to read
        {
          TokenReader tokens(reader);
            for(unsigned j = i+1; j < num_points; j++)
            {
                //read distance between points i and j
                if(!tokens.has_next_token())
                    debug() << "ERROR: no distance between points" << i << "and" << j;

                std::string str = tokens.next_token();
                debug() << str;

                exact cur_dist = str_to_exact(str);

                if( cur_dist <= max_dist )  //then this distance is allowed
                {
                    //store distance value, if it doesn't exist already
                    ret = dist_set.insert(new ExactValue(cur_dist));

                    //remember that the pair of points (i,j) has this distance value, which will go in entry j(j-1)/2 + i
                    (*(ret.first))->indexes.push_back( (j*(j-1))/2 + i );
                }
            }
        }
    }//end for

    progress.advanceProgressStage();    //advance progress box to stage 2: building bifiltration


  // STEP 3: build vectors of discrete indexes for constructing the bifiltration

    unsigned max_unsigned = std::numeric_limits<unsigned>::max();

    //first, values
    std::vector<unsigned> value_indexes(num_points, max_unsigned);   //vector of discrete value indexes for each point; max_unsigned shall represent undefined value (is this reasonable?)
    build_grade_vectors(*data, value_set, value_indexes, data->x_grades, data->x_exact, input_params.x_bins);

    //second, distances
    std::vector<unsigned> dist_indexes((num_points*(num_points-1))/2, max_unsigned);  //discrete distance matrix (triangle); max_unsigned shall represent undefined distance
    build_grade_vectors(*data, dist_set, dist_indexes, data->y_grades, data->y_exact, input_params.y_bins);

    //update progress
    progress.progress(30);

  // STEP 4: build the bifiltration

    if(verbosity >= 2) { debug() << "BUILDING VIETORIS-RIPS BIFILTRATION"; }

    //build the Vietoris-Rips bifiltration from the discrete index vectors
    data->simplex_tree->build_VR_complex(value_indexes, dist_indexes, data->x_grades.size(), data->y_grades.size());

    //clean up
    for(ExactSet::iterator it = value_set.begin(); it != value_set.end(); ++it)
    {
        ExactValue* p = *it;
        delete p;
    }
    for(ExactSet::iterator it = dist_set.begin(); it != dist_set.end(); ++it)
    {
        ExactValue* p = *it;
        delete p;
    }
    return data;
}//end read_discrete_metric_space()

//reads a bifiltration and constructs a simplex tree
std::shared_ptr<InputData> InputManager::read_bifiltration(std::ifstream &stream, Progress &progress)
{
    std::shared_ptr<InputData> data(new InputData);
    FileInputReader reader(stream);
    if(verbosity >= 2) { debug() << "  Found a bifiltration file.\n"; }

    //read the label for x-axis
    input_params.x_label = join(reader.next_line());

    //read the label for y-axis
    input_params.y_label = join(reader.next_line());

    //temporary data structures to store grades
    ExactSet x_set; //stores all unique x-alues; must DELETE all elements later!
    ExactSet y_set; //stores all unique x-alues; must DELETE all elements later!
    std::pair<ExactSet::iterator, bool> ret;    //for return value upon insert()

	//read simplices
    unsigned num_simplices = 0;
    while( reader.has_next_line() )
	{
        std::vector<std::string> tokens = reader.next_line();

		//read dimension of simplex
        int dim = tokens.size() - 3; //-3 because a n-simplex has (n+1) vertices, and the line also contains two grade values

        //read vertices
        std::vector<int> verts;
        for(int i = 0; i <= dim; i++)
        {
          int v = std::stoi(tokens[i]);
          verts.push_back(v);
        }

        //read multigrade and remember that it corresponds to this simplex
        ret = x_set.insert(new ExactValue( str_to_exact(tokens.at(dim + 1))));
        (*(ret.first))->indexes.push_back(num_simplices);
        ret = y_set.insert(new ExactValue( str_to_exact(tokens.at(dim + 2))));
        (*(ret.first))->indexes.push_back(num_simplices);

        //add the simplex to the simplex tree
        data->simplex_tree->add_simplex(verts, num_simplices, num_simplices);  //multigrade to be set later!
            ///TODO: FIX THE ABOVE FUNCTION!!!
        num_simplices++;
	}

    progress.advanceProgressStage();    //advance progress box to stage 2: building bifiltration

    //build vectors of discrete grades, using bins
    unsigned max_unsigned = std::numeric_limits<unsigned>::max();
    std::vector<unsigned> x_indexes(num_simplices, max_unsigned);   //x_indexes[i] gives the discrete x-index for simplex i in the input order
    std::vector<unsigned> y_indexes(num_simplices, max_unsigned);   //y_indexes[i] gives the discrete y-index for simplex i in the input order

    build_grade_vectors(*data, x_set, x_indexes, data->x_grades, data->x_exact, input_params.x_bins);
    build_grade_vectors(*data, y_set, y_indexes, data->y_grades, data->y_exact, input_params.y_bins);

//TESTING
//    debug() << "x-grades sorted order:";
//    for(ExactSet::iterator it = x_set.begin(); it != x_set.end(); ++it)
//    {
//        std::ostringstream oss;
//        oss << (*it)->exact_value << " = " << (*it)->double_value;
//        debug() << "   " << oss.str();
//    }
//    debug() << "x-index vector:";
//    for(std::vector<unsigned>::iterator it = x_indexes.begin(); it != x_indexes.end(); ++it)
//        debug() << "   " << *it;

    //update simplex tree nodes
    data->simplex_tree->update_xy_indexes(x_indexes, y_indexes, data->x_grades.size(), data->y_grades.size());

    //compute indexes
    data->simplex_tree->update_global_indexes();
    data->simplex_tree->update_dim_indexes();

    //clean up
    for(ExactSet::iterator it = x_set.begin(); it != x_set.end(); ++it)
    {
        ExactValue* p = *it;
        delete p;
    }
    for(ExactSet::iterator it = y_set.begin(); it != y_set.end(); ++it)
    {
        ExactValue* p = *it;
        delete p;
    }
    return data;
}//end read_bifiltration()

//reads a file of previously-computed data from RIVET
std::shared_ptr<InputData> InputManager::read_RIVET_data(std::ifstream &stream, Progress &progress)
{
    std::shared_ptr<InputData> data(new InputData);
    FileInputReader reader(stream);
  //read parameters
    auto line = reader.next_line();
    debug() << join(line) << std::endl;
  input_params.dim = std::stoi(reader.next_line()[0]);
  input_params.x_label = join(reader.next_line());
  input_params.y_label = join(reader.next_line());

  //read x-grades
  reader.next_line();  //this line should say "x-grades"
  line = reader.next_line();
  while(line[0][0] != 'y') //stop when we reach "y-grades"
    {
      exact num(line[0]);
      data->x_exact.push_back(num);
      data->x_grades.push_back( numerator(num).convert_to<double>() / denominator(num).convert_to<double>() );
      line = reader.next_line();
    }

    //read y-grades
    line = reader.next_line();  //because the current line says "y-grades"
    while(line[0][0] != 'x') //stop when we reach "xi"
    {
        exact num(line[0]);
        data->y_exact.push_back(num);
        data->y_grades.push_back( numerator(num).convert_to<double>() / denominator(num).convert_to<double>() );
        line = reader.next_line();
    }

    //read xi values
    line = reader.next_line();  //because the current line says "xi"
    while(line[0][0] != 'b') //stop when we reach "barcode templates"
    {
      unsigned x = std::stoi(line[0]);
      unsigned y = std::stoi(line[1]);
      int zero = std::stoi(line[2]);
      int one = std::stoi(line[3]);
      int two = std::stoi(line[4]);
      data->xi_support.push_back(xiPoint(x, y, zero, one, two));
      line = reader.next_line();
    }

    //read barcode templates
    //  NOTE: the current line says "barcode templates"
    while(reader.has_next_line())
    {
        line = reader.next_line();
        data->barcode_templates.push_back(BarcodeTemplate());    //create a new BarcodeTemplate

        if(line[0] != std::string("-"))    //then the barcode is nonempty
        {
            for(int i = 0; i < line.size(); i++)    //loop over all bars
            {
              std::vector<std::string> nums = split(line[i], ",");
              unsigned a = std::stol(nums[0]);
              unsigned b = -1;                    //default, for b = infinity
              if(nums[1][0] != 'i')        //then b is finite
                b = std::stol(nums[1]);
              unsigned m = std::stol(nums[2]);
              data->barcode_templates.back().add_bar(a, b, m);
            }
        }
    }

    ///TODO: maybe make a different progress box for RIVET input???
    progress.advanceProgressStage();    //advance progress box to stage 2: building bifiltration

}//end read_RIVET_data()

//converts an ExactSet of values to the vectors of discrete values that SimplexTree uses to build the bifiltration, and also builds the grade vectors (floating-point and exact)
void InputManager::build_grade_vectors(InputData &data,
                                       ExactSet& value_set,
                                       std::vector<unsigned>& discrete_indexes,
                                       std::vector<double>& grades_fp,
                                       std::vector<exact>& grades_exact,
                                       unsigned num_bins)
{
    if(num_bins == 0 || num_bins >= value_set.size())    //then don't use bins
    {
        grades_fp.reserve(value_set.size());
        grades_exact.reserve(value_set.size());

        unsigned c = 0;  //counter for indexes
        for(ExactSet::iterator it = value_set.begin(); it != value_set.end(); ++it)   //loop through all UNIQUE values
        {
            grades_fp.push_back( (*it)->double_value );
            grades_exact.push_back( (*it)->exact_value );

            for(unsigned i = 0; i < (*it)->indexes.size(); i++ ) //loop through all point indexes for this value
                discrete_indexes[ (*it)->indexes[i] ] = c;   //store discrete index

            c++;
        }
    }
    else    //then use bins: then the number of discrete indexes will equal the number of bins, and exact values will be equally spaced
    {
        //compute bin size
        exact min = (*value_set.begin())->exact_value;
        exact max = (*value_set.rbegin())->exact_value;
        exact bin_size = (max - min)/num_bins;

        //store bin values
        data.x_grades.reserve(num_bins);
        data.x_exact.reserve(num_bins);

        ExactSet::iterator it = value_set.begin();
        for(unsigned c = 0; c < num_bins; c++)    //loop through all bins
        {
          ExactValue cur_bin(static_cast<exact>(min + (c+1)*bin_size ));    //store the bin value (i.e. the right endpoint of the bin interval)
            grades_fp.push_back(cur_bin.double_value);
            grades_exact.push_back(cur_bin.exact_value);

            //store bin index for all points whose time value is in this bin
            while( it != value_set.end() && **it <= cur_bin )
            {
                for(unsigned i = 0; i < (*it)->indexes.size(); i++ ) //loop through all point indexes for this value
                    discrete_indexes[ (*it)->indexes[i] ] = c;   //store discrete index
                ++it;
            }
        }
    }
}//end build_grade_vectors()

//finds a rational approximation of a floating-point value
// precondition: x > 0
exact InputManager::approx(double x)
{
    int d = 7;	//desired number of significant digits
    int log = (int) floor( log10(x) ) + 1;

    if(log >= d)
        return exact( (int) floor(x) );

    long denom = pow(10, d-log);
    return exact( (long) floor(x*denom), denom);
}

