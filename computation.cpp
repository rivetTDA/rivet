#include "computation.h"
#include "debug.h"
#include "dcel/mesh.h"
#include "interface/input_manager.h"
#include "interface/input_parameters.h"
#include "math/multi_betti.h"

#include <chrono>

Computation::Computation(InputParameters& params, Progress &progress) :
    params(params),
    progress(progress),
    verbosity(params.verbosity)
{ }

Computation::~Computation()
{ }

std::shared_ptr<ComputationResult> Computation::compute_rivet(RivetInput &input) {
  
      //STAGE 3: MULTIGRADED BETTI NUMBERS ALREADY COMPUTED, BUT MUST COMPUTE THE DIMENSIONS

  std::shared_ptr<ComputationResult> result(new ComputationResult);

  find_dimensions(input, result->homology_dimensions);      //compute the homology dimensions at each grade from the graded Betti numbers

        if(verbosity >= 2) { debug() << "INPUT FINISHED: xi support points ready"; }

        xiSupportReady(input.xi_support);          //signal that xi support points are ready for visualization
        progress.advanceProgressStage();    //update progress box to stage 4


      //STAGES 4 and 5: RE-BUILD THE AUGMENTED ARRANGEMENT

        if(verbosity >= 2) { debug() << "RE-BUILDING THE AUGMENTED ARRANGEMENT"; }

        auto start = std::chrono::system_clock::now();

        std::shared_ptr<Mesh> arrangement(new Mesh(input.x_grades,
                                                   input.x_exact,
                                                   input.y_grades,
                                                   input.y_exact,
                                                   verbosity));
        //TODO: hook up signals
    Progress progress;
        arrangement->build_arrangement(input.xi_support, input.barcode_templates, progress);
        auto end = std::chrono::system_clock::now();

        debug() << "   re-building the augmented arrangement took" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "milliseconds";

        //send (a pointer to) the arrangement back to the VisualizationWindow
        arrangementReady(*arrangement);
}

std::shared_ptr<ComputationResult> Computation::compute_raw(RawDataInput &input) {

    if(verbosity >= 2)
    {
        debug() << "\nBIFILTRATION:";
        debug() << "   Number of simplices of dimension" << params.dim << ":" << input.bifiltration().get_size(params.dim);
        debug() << "   Number of simplices of dimension" << (params.dim + 1) << ":" << input.bifiltration().get_size(params.dim + 1);
        debug() << "   Number of x-grades:" << input.x_grades.size() << "; values" << input.x_grades.front() << "to" << input.x_grades.back();
        debug() << "   Number of y-grades:" << input.y_grades.size() << "; values" << input.y_grades.front() << "to" << input.y_grades.back() << "\n";
    }
      //STAGE 3: COMPUTE MULTIGRADED BETTI NUMBERS

    std::shared_ptr<ComputationResult> result(new ComputationResult);
        //compute xi_0 and xi_1 at all multi-grades
        if(verbosity >= 2) { debug() << "COMPUTING xi_0 AND xi_1 FOR HOMOLOGY DIMENSION " << params.dim << ":"; }
        MultiBetti mb(input.bifiltration(), params.dim, verbosity);

        auto start = std::chrono::system_clock::now();
        mb.compute_fast(result->homology_dimensions, progress);

        auto end = std::chrono::system_clock::now();
        debug() << "  --> xi_i computation took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " seconds";

        //store the xi support points
        mb.store_support_points(result->xi_support);

        xiSupportReady(result->xi_support);          //signal that xi support points are ready for visualization
        progress.advanceProgressStage();    //update progress box to stage 4


      //STAGES 4 and 5: BUILD THE LINE ARRANGEMENT AND COMPUTE BARCODE TEMPLATES

        //build the arrangement
        if(verbosity >= 2) { debug() << "CALCULATING ANCHORS AND BUILDING THE DCEL ARRANGEMENT"; }

        start = std::chrono::system_clock::now();
        Mesh *arrangement = new Mesh(input.x_grades, input.x_exact, input.y_grades, input.y_exact, verbosity);
        arrangement->build_arrangement(mb, result->xi_support, progress);     ///TODO: update this -- does not need to store list of xi support points in xi_support
        //NOTE: this also computes and stores barcode templates in the arrangement

        end = std::chrono::system_clock::now();
        debug() << "   building the line arrangement and computing all barcode templates took"
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "milliseconds";

        //send (a pointer to) the arrangement back to the VisualizationWindow
        arrangementReady(*arrangement);
    //TODO: bifiltration isn't used anywhere? Only part of the input?
}

std::shared_ptr<ComputationResult> Computation::compute(InputData &data)
{
  //STAGES 1 and 2: INPUT DATA AND CREATE BIFILTRATION

  std::chrono::time_point<std::chrono::system_clock> start, end;

    if(verbosity >= 4)
    {
        debug() << "x-grades:";
        for(unsigned i=0; i<data.x_grades.size(); i++)
        {
          std::ostringstream oss;
          oss << data.x_exact[i];
          debug() << "  " << data.x_grades[i] << "=" << oss.str().data();
        }
        debug() << "y-grades:";
        for(unsigned i=0; i<data.y_grades.size(); i++)
        {
          std::ostringstream oss;
          oss << data.y_exact[i];
          debug() << "  " << data.y_grades[i] << "=" << oss.str().data();
        }
    }

    progress.advanceProgressStage(); //update progress box to stage 3

    if(params.raw_data)    //then the user selected a raw data file, and we need to do persistence calculations
    {
      auto input = RawDataInput(data);
        //print bifiltration statistics
      return compute_raw(input);
    }
    else    //then the user selected a RIVET file with pre-computed persistence information, and we just need to re-build the arrangement
    {
      auto input = RivetInput(data);
      return compute_rivet(input);
    }
}//end run()

//computes homology dimensions from the graded Betti numbers (used when data comes from a pre-computed RIVET file)
void Computation::find_dimensions(const RivetInput &input, unsigned_matrix &homology_dimensions)
{

    homology_dimensions.resize(boost::extents[input.x_grades.size()][input.y_grades.size()]);
    std::vector<xiPoint>::iterator it = input.xi_support.begin();
    int col_sum = 0;

    //compute dimensions at (0,y)
    for(unsigned y = 0; y < input.y_grades.size(); y++)
    {
        if(it != input.xi_support.end() && it->x == 0 && it->y == y)
        {
            col_sum += it->zero - it->one + it->two;
            ++it;
        }

        homology_dimensions[0][y] = col_sum;
    }

    //compute dimensions at (x,y) for x > 0
    for(unsigned x = 1; x < input.x_grades.size(); x++)
    {
        col_sum = 0;

        for(unsigned y = 0; y < input.y_grades.size(); y++)
        {
            if(it != input.xi_support.end() && it->x == x && it->y == y)
            {
                col_sum += it->zero - it->one + it->two;
                ++it;
            }

            homology_dimensions[x][y] = homology_dimensions[x-1][y] + col_sum;
        }
    }
}//end find_dimensions()
