/* multi-graded Betti number class
 * takes a bifiltration and computes the multi-graded Betti numbers
 */

#include "multi_betti.h"

//constructor
MultiBetti::MultiBetti(SimplexTree* st, int dim, int v) :
	bifiltration(st),		//remember location of the simplex tree
	dimension(dim),			//remember the dimension
	verbosity(v)			//controls the amount of output
{
	//ensure that xi matrix is the correct size
    num_x_grades = bifiltration->num_x_grades();
    num_y_grades = bifiltration->num_y_grades();
    xi.resize(boost::extents[num_x_grades][num_y_grades][2]);

    //SUPPORT FOR DEPRECATED METHODS
    num_times = num_x_grades;
    num_dists = num_y_grades;

}//end constructor

//computes xi_0 and xi_1 at all multi-indexes in a fast way
void MultiBetti::compute_fast()
{
  //// STEP 1: compute nullity
    std::cout << "computing nullity; num_x_grades = " << num_x_grades << ", num_y_grades = " << num_y_grades << "\n";

    //get data
    MapMatrix* bdry1 = bifiltration->get_boundary_mx(dimension);
    IndexMatrix* ind1 = bifiltration->get_index_mx(dimension);

    //set up data structures
    Vector current_lows;
    Vector first_row_lows(bdry1->height(), -1);

    Vector* cur_col_counts = new Vector(num_y_grades);
    Vector* prev_col_counts = new Vector();
    int zeroed_cols = 0;

    //first, handle multi-grade (0,0)
    //do column reduction on columns 0 through ind1->get(0,0); use first_row_lows and zeroed_cols
    reduce(bdry1, 0, ind1->get(0,0), first_row_lows, zeroed_cols);

    //copy first_row_lows to current_lows
    current_lows = first_row_lows;

    //record data
    std::cout << "grade (0,0): " << zeroed_cols << "\n";
    cur_col_counts->at(0) = zeroed_cols;
    xi[0][0][0] = zeroed_cols;

    //handle the rest of the first column
    for(int y=1; y<num_y_grades; y++)
    {
        zeroed_cols = 0;

        //do column reduction on columns ind1->get(y-1,num_x_grades-1)+1 through ind1->get(y,0)
        reduce(bdry1, ind1->get(y-1,num_x_grades-1) + 1, ind1->get(y,0), current_lows, zeroed_cols);

        //record data
        std::cout << "grade (0," << y << "): " << zeroed_cols << "\n";
        cur_col_counts->at(y) = cur_col_counts->at(y-1) + zeroed_cols;
        xi[0][y][0] = cur_col_counts->at(y);
    }

    std::cout << "finished column 0\n";

    //loop through columns after first column
    for(int x=1; x<num_x_grades; x++)
    {
        //set previous_col_counts to current_col_counts; create new current_col_counts
        delete prev_col_counts;                 //IS THIS WHAT WE SHOULD DO???
        prev_col_counts = cur_col_counts;
        cur_col_counts = new Vector(num_y_grades);

        //handle first multi-grade in this column
        zeroed_cols = 0;

        std::cout << "ready to reduce column " << x << "\n";

        //do column reduction on columns ind1->get(0,x-1) + 1 through ind1->get(0,x)
        reduce(bdry1, ind1->get(0,x-1) + 1, ind1->get(0,x), first_row_lows, zeroed_cols);

        //copy first_row_lows to current_lows
        current_lows = first_row_lows;

        //record data
        std::cout << "grade (" << x << ",0): " << zeroed_cols << "\n";
        cur_col_counts->at(0) = prev_col_counts->at(0) + zeroed_cols;
        xi[x][0][0] = cur_col_counts->at(0);

        //now loop through rows after first row
        for(int y=1; y<num_y_grades; y++)
        {
            zeroed_cols = 0;

            //do column reduction on columns ind1->get(y-1,num_x_grades-1) + 1 through ind1->get(y,x)
            reduce(bdry1, ind1->get(y-1,num_x_grades-1) + 1, ind1->get(y,x), current_lows, zeroed_cols);

            //record data
            std::cout << "grade (" << x << "," << y << "): " << zeroed_cols << "\n";
            cur_col_counts->at(y) = prev_col_counts->at(y) - prev_col_counts->at(y-1) + cur_col_counts->at(y-1) + zeroed_cols;
            xi[x][y][0] = cur_col_counts->at(y);
        }
    }

    //clean up
    delete cur_col_counts;
    delete prev_col_counts;
    //////////////////////////////////// ANYTHING ELSE TO DELETE???


  //// STEP 2: compute rank



  //// STEP 3: compute xi_0



  //// STEP 4: compute xi_1



}//end compute_fast();


//reduce matrix: perform column operations from first_col to last_col, inclusive
//TODO: when testing finished, remove print statements
void MultiBetti::reduce(MapMatrix* mm, int first_col, int last_col, Vector& lows, int& zeroed_cols)
{
    std::cout << "  reducing columns " << first_col << " to " << last_col << "...";
    for(int i = first_col; i <= last_col; i++)
    {
        std::cout << "  looping...";
        //while column i is nonempty and its low number is found in the low array, do column operations
        while(mm->low(i) >= 0 && lows[mm->low(i)] >= 0)
        {
            std::cout << "  --adding column " << lows[mm->low(i)] << " to column " << i << "\n";
            mm->add_column(lows[mm->low(i)], i);
        }

        if(mm->low(i) >= 0) //column is still nonempty, so update lows
            lows[mm->low(i)] = i;
        else //column was zeroed out
            zeroed_cols++;
    }
    //testing
    std::cout << "  finished; lows: ";
    for(int i=0; i<lows.size(); i++)
        std::cout << lows[i] << ", ";
    std::cout << "\n";
    mm->print();
}




/////// DEPRECATED FUNCTIONS ///////

//computes xi_0 and xi_1 at ALL multi-indexes
//void MultiBetti::compute_all_xi()
//{
//	if(verbosity >= 4) { std::cout << "  computing xi_0 and xi_1; num_times=" << num_times << ", num_dists=" << num_dists << "\n"; }
	
//	//compute and store xi_0 and xi_1 at ALL multi-indexes
//	for(int time = 0; time < num_times; time++)
//	{
//		for(int dist = 0; dist < num_dists; dist++)
//		{
//			compute_xi(time, dist);
//		}
//	}
//}


//computes xi_0 and xi_1 at a specified multi-index
//void MultiBetti::compute_xi(int time, int dist)
//{
//	//xi_0 first
//	if(verbosity >= 6) { std::cout << "  COMPUTING xi_0(" << time << "," << dist << ") in dimension " << dimension << "\n"; }
	
//	//build boundary matrix B+C
//	MapMatrix* bdry_bc = (*bifiltration).get_boundary_mx(time, dist-1, dimension);
//	MapMatrix* bdry_c = (*bifiltration).get_boundary_mx(time-1, dist, dimension);
//	(*bdry_bc).append_block( (*bdry_c), (*bdry_bc).height() );
//	if(verbosity >= 8)
//	{
//		std::cout << "    boundary matrix for direct sum B+C:\n";
//		(*bdry_bc).print();
//	}

//	//build matrix [B+C,D]
//	MapMatrix* bcd = (*bifiltration).get_merge_mx(time, dist, dimension);
//	if(verbosity >= 8)
//	{
//		std::cout << "    map matrix [B+C,D]:\n";
//		(*bcd).print();
//	}

//	//apply column-reduction algorithm to boundary matrix B+C, and do the same column operations to [B+C,D]
//	(*bdry_bc).col_reduce(bcd);
//	if(verbosity >= 8)
//	{
//		std::cout << "    reducing boundary matrix B+C, and applying same column operations to [B+C,D]\n";
//		std::cout << "      reduced boundary matrix B+C:\n";
//		(*bdry_bc).print();
//		std::cout << "      reduced matrix [B+C,D]:\n";
//		(*bcd).print();
//	}

//	//identify zero columns from boundary matrix for B+C, and select those columns from [B+C,D]
//	int c=1;	//keep track of current column in [B+C,D]
//	for(int j=1; j<=(*bdry_bc).width(); j++)
//	{
//		if((*bdry_bc).low(j) > 0)	//then delete this column from [B+C,D]
//			(*bcd).remove_column(c);
//		else
//			c++;
//	}
//	int nullity_bc1 = c-1;	//we need nullity of B+C for later
//	//now bcd contains a basis for [B+C,D](ker(boundary map B+C))
//	if(verbosity >= 8)
//	{
//		std::cout << "      basis for [B+C,D](ker(boundary map B+C)):\n";
//		(*bcd).print();
//	}

//	//form concatenated matrix
//	MapMatrix* bdry_d2 = (*bifiltration).get_boundary_mx(time, dist, dimension+1);
//	if(verbosity >= 8)
//	{
//		std::cout << "    boundary matrix D2:";
//		(*bdry_d2).print();
//	}
//	int d2_width = (*bdry_d2).width();
//	(*bdry_d2).append_block( (*bcd), 0);
//	if(verbosity >= 8)
//	{
//		std::cout << "    concatenating D2 with [B+C,D](ker(..)):\n";
//		(*bdry_d2).print();
//	}

//	//reduce the concatenated matrix
//	(*bdry_d2).col_reduce();
//	if(verbosity >= 8)
//	{
//		std::cout << "    reduced form of concatenated matrix:\n";
//		(*bdry_d2).print();
//	}

//	//count nonzero columns in reduced form of right block of concatenated matrix
//	int alpha = 0;
//	for(int j = d2_width+1; j<=(*bdry_d2).width(); j++)
//		if((*bdry_d2).low(j) > 0)
//			alpha++;
//	if(verbosity >= 8) { std::cout << "    number of nonzero columns in right block (alpha): " << alpha << "\n"; }

//	//compute dimensionension of homology at D
//	MapMatrix* bdry_d1 = (*bifiltration).get_boundary_mx(time, dist, dimension);
//	if(verbosity >= 8)
//	{
//		std::cout << "    boundary matrix D1:\n";
//		(*bdry_d1).print();
//	}
//	//compute nullity of D1
//	(*bdry_d1).col_reduce();
//	int nullity_d1 = 0;
//	for(int j=1; j<=(*bdry_d1).width(); j++)
//		if( (*bdry_d1).low(j) == 0)
//			nullity_d1++;
//	//compute rank of D2
//	int rank_d2 = 0;
//	for(int j=1; j<=d2_width; j++)
//		if( (*bdry_d2).low(j) > 0)
//			rank_d2++;
//	if(verbosity >= 8)
//	{
//		std::cout << "      nullity of D1 is: " << nullity_d1 << "\n";
//		std::cout << "      rank of D2 is: " << rank_d2 << "\n";
//	}

//	//compute xi_0
//	xi[time][dist][0] = nullity_d1 - rank_d2 - alpha;

//	//now for xi_1
//	if(verbosity >= 8) { std::cout << "  COMPUTING xi_1(" << time << "," << dist << ") in dimension " << dimension << "\n"; }

//	//build boundary matrix A
//	MapMatrix* bdry_a = (*bifiltration).get_boundary_mx(time-1, dist-1, dimension);
//	if(verbosity >= 8)
//	{
//		std::cout << "    boundary matrix A:\n";
//		(*bdry_a).print();
//	}

//	//build matrix [A,B+C]
//	MapMatrix* abc = (*bifiltration).get_split_mx(time, dist, dimension);
//	if(verbosity >= 8)
//	{
//		std::cout << "    map matrix [A,B+C]:\n";
//		(*abc).print();
//	}

//	//apply column-reduction algorithm to boundary matrix A, and do the same column operations to [A,B+C]
//	(*bdry_a).col_reduce(abc);
//	if(verbosity >= 8)
//	{
//		std::cout << "    reducing boundary matrix A, and applying same column operations to [A,B+C]\n";
//		std::cout << "      reduced boundary matrix A:\n";
//		(*bdry_a).print();
//		std::cout << "      reduced matrix [A,B+C]:\n";
//		(*abc).print();
//	}

//	//identify zero columns from boundary matrix for A, and select those columns from [A,B+C]
//	c=1;	//keep track of current column in [A,B+C]
//	for(int j=1; j<=(*bdry_a).width(); j++)
//	{
//		if((*bdry_a).low(j) > 0)	//then delete this column from [A,B+C]
//			(*abc).remove_column(c);
//		else
//			c++;
//	}
//	//now abc contains a basis for [A,B+C](ker(boundary map A))
//	if(verbosity >= 8)
//	{
//		std::cout << "      basis for [A,B+C](ker(boundary map A)):\n";
//		(*abc).print();
//	}

//	//build boundary matrix B+C for one dimension higher
//	MapMatrix* bdry_bc2 = (*bifiltration).get_boundary_mx(time, dist-1, dimension+1);
//	MapMatrix* bdry_c2 = (*bifiltration).get_boundary_mx(time-1, dist, dimension+1);
//	(*bdry_bc2).append_block( (*bdry_c2), (*bdry_bc2).height() );
//	if(verbosity >= 8)
//	{
//		std::cout << "    boundary matrix for direct sum B+C (2):\n";
//		(*bdry_bc2).print();
//	}

//	//form concatenated matrix
//	int bc2_width = (*bdry_bc2).width();
//	(*bdry_bc2).append_block( (*abc), 0);
//	if(verbosity >= 8)
//	{
//		std::cout << "    concatenating B+C (2) with [A,B+C](ker(..)):\n";
//		(*bdry_bc2).print();
//	}

//	//reduce the concatenated matrix
//	(*bdry_bc2).col_reduce();
//	if(verbosity >= 8)
//	{
//		std::cout << "    reduced form of concatenated matrix:\n";
//		(*bdry_bc2).print();
//	}

//	//count nonzero columns in reduced form of right block of concatenated matrix
//	int nu = 0;
//	for(int j = bc2_width+1; j<=(*bdry_bc2).width(); j++)
//		if((*bdry_bc2).low(j) > 0)
//			nu++;
//	if(verbosity >= 8) { std::cout << "    number of nonzero columns in right block (nu): " << nu << "\n"; }

//	//compute dimension of homology at B+C
//	//compute nullity of boundary B+C (1) was computed earlier and stored in nullity_bc
//	//rank of boundary B+C (2)

//	//compute rank of D2
//	int rank_bc2 = 0;
//	for(int j=1; j<=bc2_width; j++)
//		if( (*bdry_bc2).low(j) > 0)
//			rank_bc2++;
//	if(verbosity >= 8)
//	{
//		std::cout << "      nullity of B+C (1) is: " << nullity_bc1 << "\n";
//		std::cout << "      rank of B+C (2) is: " << rank_bc2 << "\n";
//		std::cout << "      nullity(gamma_M) = " << (nullity_bc1 - rank_bc2 - alpha) << "\n";
//		std::cout << "      alpha = " << alpha << "\n";
//	}

//	//compute \xi_1
//	xi[time][dist][1] = nullity_bc1 - rank_bc2 - alpha - nu;
//}//end compute_xi


///// FUNCTIONS TO ACCESS THE MULTI-GRADED BETTI NUMBERS

//returns xi_0 at the specified multi-index
int MultiBetti::xi0(int x, int y)
{
    return xi[x][y][0];
}

//returns xi_1 at the specified multi-index
int MultiBetti::xi1(int x, int y)
{
    return xi[x][y][1];
}
	



