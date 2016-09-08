#include "computation.h"
#include "dcel/mesh.h"
#include "dcel/mesh_builder.h"
#include "debug.h"
#include "math/multi_betti.h"
#include "timer.h"
#include <chrono>

Computation::Computation(InputParameters& params, Progress& progress)
    : params(params)
    , progress(progress)
    , verbosity(params.verbosity)
{
}

Computation::~Computation()
{
}

std::unique_ptr<ComputationResult> Computation::compute_raw(ComputationInput& input)
{

    debug() << "entering compute_raw";
    if (verbosity >= 2) {
        debug() << "\nBIFILTRATION:";
        debug() << "   Number of simplices of dimension " << params.dim << " : " << input.bifiltration().get_size(params.dim);
        debug() << "   Number of simplices of dimension " << (params.dim + 1) << " : " << input.bifiltration().get_size(params.dim + 1);
        debug() << "   Number of x-exact:" << input.x_exact.size();
        if (input.x_exact.size()) {
            debug() << "; values " << input.x_exact.front() << " to " << input.x_exact.back();
        }
        debug() << "   Number of y-exact:" << input.y_exact.size();
        if (input.y_exact.size()) {
            debug() << "; values " << input.y_exact.front() << " to " << input.y_exact.back();
        }
    }
    //STAGE 3: COMPUTE MULTIGRADED BETTI NUMBERS

    std::unique_ptr<ComputationResult> result(new ComputationResult);
    //compute xi_0 and xi_1 at all multi-grades
    if (verbosity >= 2) {
        debug() << "COMPUTING xi_0 AND xi_1 FOR HOMOLOGY DIMENSION " << params.dim << ":";
    }
    MultiBetti mb(input.bifiltration(), params.dim, verbosity);
    Timer timer;
    debug() << "Calling compute_fast";
    mb.compute_fast(result->homology_dimensions, progress);

    debug() << "  --> xi_i computation took " << timer.elapsed() << " seconds";

    //store the xi support points
    mb.store_support_points(result->xi_support);

    xiSupportReady(XiSupportMessage{ input.x_label, input.y_label, result->xi_support, result->homology_dimensions, input.x_exact, input.y_exact }); //signal that xi support points are ready for visualization
    progress.advanceProgressStage(); //update progress box to stage 4

    //STAGES 4 and 5: BUILD THE LINE ARRANGEMENT AND COMPUTE BARCODE TEMPLATES

    //build the arrangement
    if (verbosity >= 2) {
        debug() << "CALCULATING ANCHORS AND BUILDING THE DCEL ARRANGEMENT";
    }

    timer.restart();
    MeshBuilder builder(verbosity);
    auto arrangement = builder.build_arrangement(mb, input.x_exact, input.y_exact, result->xi_support, progress); ///TODO: update this -- does not need to store list of xi support points in xi_support
    //NOTE: this also computes and stores barcode templates in the arrangement

    debug() << "   building the line arrangement and computing all barcode templates took"
            << timer.elapsed() << "milliseconds";

    //send (a pointer to) the arrangement back to the VisualizationWindow
    arrangementReady(arrangement);
    //re-send xi support and other anchors
    debug() << "Sending" << result->xi_support.size() << "anchors";
    xiSupportReady(XiSupportMessage{ input.x_label, input.y_label, result->xi_support, result->homology_dimensions, input.x_exact, input.y_exact });
    arrangement->test_consistency();
    result->arrangement = std::move(arrangement);
    return result;
}

std::unique_ptr<ComputationResult> Computation::compute(InputData data)
{
    //STAGES 1 and 2: INPUT DATA AND CREATE BIFILTRATION
    if (verbosity >= 4) {
        write_grades(std::clog, data.x_exact, data.y_exact);
    }

    progress.advanceProgressStage(); //update progress box to stage 3

    auto input = ComputationInput(data);
    //print bifiltration statistics
    debug() << "Computing from raw data";
    return compute_raw(input);
}
