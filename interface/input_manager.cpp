/* InputManager class
 */

#include "input_manager.h"

#include "../computationthread.h"
#include "file_input_reader.h"
#include "input_parameters.h"
#include "../math/simplex_tree.h"
#include "exception.h"

#include <QDebug>
#include <QString>
#include <QTime>

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <set>
#include <sstream>
#include <vector>


//epsilon value for use in comparisons
double ExactValue::epsilon = pow(2,-30);


//helper function to convert a string to an exact (rational)
//accepts string such as "12.34", "765", and "-10.8421"
exact str_to_exact(std::string str)
{
    QString qstr = QString::fromUtf8(str.c_str());

    //make sure str represents a number
    bool isDouble;
    qstr.toDouble(&isDouble);
    if(!isDouble)
    {
        QString err_str("Error: In input file, " + qstr + " is not a number.");
        qDebug() << err_str << endl;
        throw Exception(err_str);
    }

    //now convert str to an exact value
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
        else if(filetype == QString("metric"))
        {
            read_discrete_metric_space(reader);
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
    if(verbosity >= 6) { qDebug() << "  Found a point cloud file."; }

  // STEP 1: read data file and store exact (rational) values

    //read dimension of the points from the first line of the file
    QStringList dimension_line = reader.next_line();
    if (dimension_line.size() != 1)
    {
    	qDebug() << "There was more than one value in the expected dimension line.  There may be a problem with your input file.  " << endl;
    }
    int dimension = dimension_line.first().toInt();

    //check for invalid input
    if (dimension == 0)
    {
    	qDebug() << "An invalid input was received for the dimension." << endl;
    	// throw an exception
    }

    //read maximum distance for edges in Vietoris-Rips complex
    QStringList distance_line = reader.next_line();
    if (distance_line.size() != 1)
    {
    	qDebug() << "There was more than one value in the expected distance line.  There may be a problem with your input file.  " << endl;
    }

    exact max_dist = str_to_exact(distance_line.first().toStdString());  ///TODO: don't convert to std::string
    if (max_dist == 0)
    {
        qDebug() << "An invalid input was received for the max distance." << endl;
        // throw an exception
    }

    if(verbosity >= 4)
    {
        std::ostringstream oss;
        oss << max_dist;
        qDebug() << "  maximum distance:" << QString::fromStdString(oss.str());
    }

    //read label for x-axis
    input_params.x_label = reader.next_line_str();

    //set label for y-axis to "distance"
    input_params.y_label = QString("distance");

    //read points
    std::vector<DataPoint> points;
    while( reader.has_next_line() )
    {
        QStringList tokens = reader.next_line();
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

    if(verbosity >= 4) { qDebug() << "  read" << points.size() << "points; input finished"; }


  // STEP 2: compute distance matrix, and create ordered lists of all unique distance and time values

    if(verbosity >= 6) { qDebug() << "BUILDING DISTANCE AND TIME LISTS"; }
    cthread->advanceProgressStage();

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
    build_grade_vectors(time_set, time_indexes, x_grades, x_exact, input_params.x_bins);

    //second, distances
    std::vector<unsigned> dist_indexes((num_points*(num_points-1))/2, max_unsigned);  //discrete distance matrix (triangle); max_unsigned shall represent undefined distance
    build_grade_vectors(dist_set, dist_indexes, y_grades, y_exact, input_params.y_bins);

    //update progress
    cthread->setCurrentProgress(30);


  // STEP 4: build the bifiltration

    //simplex_tree stores only DISCRETE information!
    //this only requires (suppose there are k points):
    //  1. a list of k discrete times
    //  2. a list of k(k-1)/2 discrete distances
    //  3. max dimension of simplices to construct, which is one more than the dimension of homology to be computed

    if(verbosity >= 6) { qDebug() << "BUILDING VIETORIS-RIPS BIFILTRATION"; }

    simplex_tree->build_VR_complex(time_indexes, dist_indexes, x_grades.size(), y_grades.size());

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

//reads data representing a discrete metric space with a real-valued function and constructs a simplex tree
void InputManager::read_discrete_metric_space(FileInputReader& reader)
{
    if(verbosity >= 2) { qDebug() << "  Found a discrete metric space file."; }

  // STEP 1: read data file and store exact (rational) values of the function for each point

    //first read the label for x-axis
    input_params.x_label = reader.next_line_str();

    //now read the values
    QStringList line = reader.next_line();
    std::vector<exact> values;
    values.reserve(line.size());

    for(int i = 0; i < line.size(); i++)
    {
        values.push_back(str_to_exact(line.at(i).toStdString()));
    }


  // STEP 2: read data file and store exact (rational) values for all distances

    //first read the label for y-axis
    input_params.y_label = reader.next_line_str();

    //read the maximum length of edges to construct
    exact max_dist = str_to_exact(reader.next_line().first().toStdString());  ///TODO: don't convert to std::string
    if(verbosity >= 4)
    {
        std::ostringstream oss;
        oss << max_dist;
        qDebug() << "  maximum distance:" << QString::fromStdString(oss.str());
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
//            line = reader.next_line();

            for(unsigned j = i+1; j < num_points; j++)
            {
                //read distance between points i and j
                if(!reader.has_next_token())
                    qDebug() << "ERROR: no distance between points" << i << "and" << j;

                QString str = reader.next_token();
                qDebug() << str;

                exact cur_dist = str_to_exact(str.toStdString());

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

    cthread->advanceProgressStage();    //advance progress box to stage 2: building bifiltration


  // STEP 3: build vectors of discrete indexes for constructing the bifiltration

    unsigned max_unsigned = std::numeric_limits<unsigned>::max();

    //first, values
    std::vector<unsigned> value_indexes(num_points, max_unsigned);   //vector of discrete value indexes for each point; max_unsigned shall represent undefined value (is this reasonable?)
    build_grade_vectors(value_set, value_indexes, x_grades, x_exact, input_params.x_bins);

    //second, distances
    std::vector<unsigned> dist_indexes((num_points*(num_points-1))/2, max_unsigned);  //discrete distance matrix (triangle); max_unsigned shall represent undefined distance
    build_grade_vectors(dist_set, dist_indexes, y_grades, y_exact, input_params.y_bins);

    //update progress
    cthread->setCurrentProgress(30);

  // STEP 4: build the bifiltration

    if(verbosity >= 2) { qDebug() << "BUILDING VIETORIS-RIPS BIFILTRATION"; }

    //build the Vietoris-Rips bifiltration from the discrete index vectors
    simplex_tree->build_VR_complex(value_indexes, dist_indexes, x_grades.size(), y_grades.size());

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
}//end read_discrete_metric_space()

//reads a bifiltration and constructs a simplex tree
void InputManager::read_bifiltration(FileInputReader& reader)
{
    if(verbosity >= 2) { qDebug() << "  Found a bifiltration file.\n"; }

    //read the label for x-axis
    input_params.x_label = reader.next_line_str();

    //read the label for y-axis
    input_params.y_label = reader.next_line_str();

    //temporary data structures to store grades
    ExactSet x_set; //stores all unique x-alues; must DELETE all elements later!
    ExactSet y_set; //stores all unique x-alues; must DELETE all elements later!
    std::pair<ExactSet::iterator, bool> ret;    //for return value upon insert()

	//read simplices
    unsigned num_simplices = 0;
    while( reader.has_next_line() )
	{
        QStringList tokens = reader.next_line();

		//read dimension of simplex
        int dim = tokens.size() - 3; //-3 because a n-simplex has (n+1) vertices, and the line also contains two grade values

        //read vertices
        std::vector<int> verts;
        for(int i = 0; i <= dim; i++)
        {
            int v = tokens.at(i).toInt();
            verts.push_back(v);
        }

        //read multigrade and remember that it corresponds to this simplex
        ret = x_set.insert(new ExactValue( str_to_exact(tokens.at(dim + 1).toStdString()) ));  ///TODO: don't convert to std::string
        (*(ret.first))->indexes.push_back(num_simplices);
        ret = y_set.insert(new ExactValue( str_to_exact(tokens.at(dim + 2).toStdString()) ));  ///TODO: don't convert to std::string
        (*(ret.first))->indexes.push_back(num_simplices);

        //add the simplex to the simplex tree
        simplex_tree->add_simplex(verts, num_simplices, num_simplices);  //multigrade to be set later!
            ///TODO: FIX THE ABOVE FUNCTION!!!
        num_simplices++;
	}

    cthread->advanceProgressStage();    //advance progress box to stage 2: building bifiltration

    //build vectors of discrete grades, using bins
    unsigned max_unsigned = std::numeric_limits<unsigned>::max();
    std::vector<unsigned> x_indexes(num_simplices, max_unsigned);   //x_indexes[i] gives the discrete x-index for simplex i in the input order
    std::vector<unsigned> y_indexes(num_simplices, max_unsigned);   //y_indexes[i] gives the discrete y-index for simplex i in the input order

    build_grade_vectors(x_set, x_indexes, x_grades, x_exact, input_params.x_bins);
    build_grade_vectors(y_set, y_indexes, y_grades, y_exact, input_params.y_bins);

//TESTING
//    qDebug() << "x-grades sorted order:";
//    for(ExactSet::iterator it = x_set.begin(); it != x_set.end(); ++it)
//    {
//        std::ostringstream oss;
//        oss << (*it)->exact_value << " = " << (*it)->double_value;
//        qDebug() << "   " << QString::fromStdString(oss.str());
//    }
//    qDebug() << "x-index vector:";
//    for(std::vector<unsigned>::iterator it = x_indexes.begin(); it != x_indexes.end(); ++it)
//        qDebug() << "   " << *it;

    //update simplex tree nodes
    simplex_tree->update_xy_indexes(x_indexes, y_indexes, x_grades.size(), y_grades.size());

    //compute indexes
    simplex_tree->update_global_indexes();
    simplex_tree->update_dim_indexes();

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
}//end read_bifiltration()

//reads a file of previously-computed data from RIVET
void InputManager::read_RIVET_data(FileInputReader& reader)
{
    //read parameters
    cthread->params.dim = reader.next_line().first().toInt();
    cthread->params.x_label = reader.next_line_str();
    cthread->params.y_label = reader.next_line_str();

    //read x-grades
    reader.next_line();  //this line should say "x-grades"
    QStringList line = reader.next_line();
    while(line.first().at(0) != QChar('y')) //stop when we reach "y-grades"
    {
        exact num(line.first().toStdString());
        x_exact.push_back(num);
        x_grades.push_back( numerator(num).convert_to<double>() / denominator(num).convert_to<double>() );
        line = reader.next_line();
    }

    //read y-grades
    line = reader.next_line();  //because the current line says "y-grades"
    while(line.first().at(0) != QChar('x')) //stop when we reach "xi"
    {
        exact num(line.first().toStdString());
        y_exact.push_back(num);
        y_grades.push_back( numerator(num).convert_to<double>() / denominator(num).convert_to<double>() );
        line = reader.next_line();
    }

    //read xi values
    line = reader.next_line();  //because the current line says "xi"
    while(line.first().at(0) != QChar('b')) //stop when we reach "barcode templates"
    {
        unsigned x = line.first().toUInt();
        unsigned y = line.at(1).toUInt();
        int zero = line.at(2).toInt();
        int one = line.at(3).toInt();
        int two = line.at(4).toInt();
        cthread->xi_support.push_back(xiPoint(x, y, zero, one, two));
        line = reader.next_line();
    }

    //read barcode templates
    //  NOTE: the current line says "barcode templates"
    while(reader.has_next_line())
    {
        line = reader.next_line();
        cthread->barcode_templates.push_back(BarcodeTemplate());    //create a new BarcodeTemplate

        if(line.first() != QString("-"))    //then the barcode is nonempty
        {
            for(int i = 0; i < line.size(); i++)    //loop over all bars
            {
                QStringList nums = line.at(i).split(",");
                unsigned a = nums.first().toUInt();
                unsigned b = -1;                    //default, for b = infinity
                if(nums.at(1) != QChar('i'))        //then b is finite
                    b = nums.at(1).toUInt();
                unsigned m = nums.at(2).toUInt();
                cthread->barcode_templates.back().add_bar(a, b, m);
            }
        }
    }

    ///TODO: maybe make a different progress box for RIVET input???
    cthread->advanceProgressStage();    //advance progress box to stage 2: building bifiltration

}//end read_RIVET_data()

//converts an ExactSet of values to the vectors of discrete values that SimplexTree uses to build the bifiltration, and also builds the grade vectors (floating-point and exact)
void InputManager::build_grade_vectors(ExactSet& value_set, std::vector<unsigned>& discrete_indexes, std::vector<double>& grades_fp, std::vector<exact>& grades_exact, unsigned num_bins)
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
        x_grades.reserve(num_bins);
        x_exact.reserve(num_bins);

        ExactSet::iterator it = value_set.begin();
        for(unsigned c = 0; c < num_bins; c++)    //loop through all bins
        {
            ExactValue cur_bin( min + (c+1)*bin_size );    //store the bin value (i.e. the right endpoint of the bin interval)
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

