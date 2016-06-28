#include "computation.h"
#include "debug.h"
#include "dcel/mesh.h"
#include "dcel/mesh_builder.h"
#include "math/multi_betti.h"
#include "timer.h"
#include <chrono>

Computation::Computation(InputParameters& params, Progress &progress) :
    params(params),
    progress(progress),
    verbosity(params.verbosity)
{ }

Computation::~Computation()
{ }

std::unique_ptr<ComputationResult> Computation::compute_rivet(RivetInput &input) {
  
      //STAGE 3: MULTIGRADED BETTI NUMBERS ALREADY COMPUTED, BUT MUST COMPUTE THE DIMENSIONS

  std::unique_ptr<ComputationResult> result(new ComputationResult);

  find_dimensions(input, result->homology_dimensions);      //compute the homology dimensions at each grade from the graded Betti numbers

        if(verbosity >= 2) { debug() << "INPUT FINISHED: xi support points ready"; }

        xiSupportReady(input.xi_support);          //signal that xi support points are ready for visualization
        progress.advanceProgressStage();    //update progress box to stage 4


      //STAGES 4 and 5: RE-BUILD THE AUGMENTED ARRANGEMENT

        if(verbosity >= 2) { debug() << "RE-BUILDING THE AUGMENTED ARRANGEMENT"; }

        auto start = std::chrono::system_clock::now();

        //TODO: hook up signals
    Progress progress;
    debug() << "Calling build_arrangement" ;
    MeshBuilder builder(verbosity);
    auto arrangement = builder.build_arrangement(input.x_exact, input.y_exact, input.xi_support, input.barcode_templates, progress);
        auto end = std::chrono::system_clock::now();

        debug() << "   re-building the augmented arrangement took" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "milliseconds";

        //send (a pointer to) the arrangement back to the VisualizationWindow
        arrangementReady(*arrangement);
}

std::unique_ptr<ComputationResult> Computation::compute_raw(RawDataInput &input) {

    debug() << "entering compute_raw" ;
    if(verbosity >= 2)
    {
        debug() << "\nBIFILTRATION:" ;
        debug() << "   Number of simplices of dimension " << params.dim << " : " << input.bifiltration().get_size(params.dim) ;
        debug() << "   Number of simplices of dimension " << (params.dim + 1) << " : " << input.bifiltration().get_size(params.dim + 1) ;
        debug() << "   Number of x-exact:" << input.x_exact.size() ;
        if (input.x_exact.size())
        {
            debug() << "; values " << input.x_exact.front() << " to " << input.x_exact.back() ;
        }
        debug() << "   Number of y-exact:" << input.y_exact.size() ;
        if (input.y_exact.size()) {
            debug() << "; values " << input.y_exact.front() << " to " << input.y_exact.back() ;
        }
    }
      //STAGE 3: COMPUTE MULTIGRADED BETTI NUMBERS

    std::unique_ptr<ComputationResult> result(new ComputationResult);
        //compute xi_0 and xi_1 at all multi-grades
        if(verbosity >= 2) { debug() << "COMPUTING xi_0 AND xi_1 FOR HOMOLOGY DIMENSION " << params.dim << ":"; }
        MultiBetti mb(input.bifiltration(), params.dim, verbosity);
        Timer timer;
    debug() << "Calling compute_fast" ;
        mb.compute_fast(result->homology_dimensions, progress);

        debug() << "  --> xi_i computation took " << timer.elapsed() << " seconds";

        //store the xi support points
        mb.store_support_points(result->xi_support);

        xiSupportReady(result->xi_support);          //signal that xi support points are ready for visualization
        progress.advanceProgressStage();    //update progress box to stage 4


      //STAGES 4 and 5: BUILD THE LINE ARRANGEMENT AND COMPUTE BARCODE TEMPLATES

        //build the arrangement
        if(verbosity >= 2) { debug() << "CALCULATING ANCHORS AND BUILDING THE DCEL ARRANGEMENT"; }

    timer.restart();
    MeshBuilder builder(verbosity);
    auto arrangement = builder.build_arrangement(mb, input.x_exact, input.y_exact, result->xi_support, progress);     ///TODO: update this -- does not need to store list of xi support points in xi_support
        //NOTE: this also computes and stores barcode templates in the arrangement

        debug() << "   building the line arrangement and computing all barcode templates took"
                << timer.elapsed() << "milliseconds";

        //send (a pointer to) the arrangement back to the VisualizationWindow
        arrangementReady(*arrangement);
    result->arrangement = std::move(arrangement);
    //TODO: bifiltration isn't used anywhere? Only part of the input?
    return result;
}

std::unique_ptr<ComputationResult> Computation::compute(InputData data)
{
  //STAGES 1 and 2: INPUT DATA AND CREATE BIFILTRATION
    if(verbosity >= 4)
    {

        write_grades(std::clog, data.x_exact, data.y_exact);
    }

    progress.advanceProgressStage(); //update progress box to stage 3

    if(data.is_data)    //then the user selected a raw data file, and we need to do persistence calculations
    {
      auto input = RawDataInput(data);
        //print bifiltration statistics
        debug() << "Computing from raw data" ;
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

    homology_dimensions.resize(boost::extents[input.x_exact.size()][input.y_exact.size()]);
    std::vector<xiPoint>::iterator it = input.xi_support.begin();
    int col_sum = 0;

    //compute dimensions at (0,y)
    for(unsigned y = 0; y < input.y_exact.size(); y++)
    {
        if(it != input.xi_support.end() && it->x == 0 && it->y == y)
        {
            col_sum += it->zero - it->one + it->two;
            ++it;
        }

        homology_dimensions[0][y] = col_sum;
    }

    //compute dimensions at (x,y) for x > 0
    for(unsigned x = 1; x < input.x_exact.size(); x++)
    {
        col_sum = 0;

        for(unsigned y = 0; y < input.y_exact.size(); y++)
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
