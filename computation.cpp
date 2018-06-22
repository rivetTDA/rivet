/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "computation.h"
#include "dcel/arrangement.h"
#include "dcel/arrangement_builder.h"
#include "debug.h"
#include "math/multi_betti.h"
#include "timer.h"
#include <chrono>

Computation::Computation(int vrbsty, Progress& progress)
    : progress(progress)
    , verbosity(vrbsty)
{
}

Computation::~Computation()
{
}

std::unique_ptr<ComputationResult> Computation::compute_raw(ComputationInput& input, bool koszul)
{
    if (verbosity >= 2) {
        debug() << "FIRep computed:";
        
        //debug() << "   Number of simplices of dimension " << params.dim << " : " << input.bifiltration().get_size(params.dim);
        //debug() << "   Number of simplices of dimension " << (params.dim + 1) << " : " << input.bifiltration().get_size(params.dim + 1);
        
        if (verbosity >= 4) {
            debug() << "   Number of x-exact:" << input.x_exact.size();
            if (input.x_exact.size()) {
                debug() << "; values " << input.x_exact.front() << " to " << input.x_exact.back();
            }
            debug() << "   Number of y-exact:" << input.y_exact.size();
            if (input.y_exact.size()) {
                debug() << "; values " << input.y_exact.front() << " to " << input.y_exact.back();
            }
        }
    }

    //STAGE 3: COMPUTE MINIMAL PRESENTATION AND MULTIGRADED BETTI NUMBERS

    std::unique_ptr<ComputationResult> result(new ComputationResult);

    MultiBetti mb(input.rep());

    Timer timer;
    timer.restart();
    
    Presentation pres;
    
    // If the --koszul flag is not given, then we compute Betti numbers by
    //computing a minimal presentation
    if (!koszul)
    {
        compute_min_pres_and_betti_nums(input,mb,pres,result);
    }
    
    // If --koszul flag is given, then we use the old Betti number computation
    //algorithm, which does not compute a presentation
    else {
        if (verbosity >= 2) {
            debug() << "COMPUTING BETTI NUMBERS VIA KOSZUL HOMOLOGY ALGORITHM:";
        }
        
        mb.compute_koszul(input.rep(),result->homology_dimensions, progress);
        
        mb.compute_xi2(result->homology_dimensions);
    }
    
    
    if (verbosity >= 2) {
        debug() << "  -- Betti number and Hilbert function computation took " << timer.elapsed() << " milliseconds";
    }

    //store the xi support points
    mb.store_support_points(result->template_points);
    
    //signal that xi support points are ready for visualization.
    //Also will print Betti numbers and exit if rivet_console is called with
    //--betti flag.
    template_points_ready(TemplatePointsMessage{ input.x_label, input.y_label, result->template_points, result->homology_dimensions, input.x_exact, input.y_exact,input.x_reverse,input.y_reverse }); 
    
    //Will print minimal presentation and exit if rivet_console is called with
    //--minpres flag.
    
    /* TODO: Code could be restructured to do slightly less work before printing
       minpres and exiting.  But putting minpres_ready() after
       templates_points_ready() allows us to print the exact bigrades using
       templates_points_ready(), which is convenient for now. */
    minpres_ready(pres);
    
    progress.advanceProgressStage(); //update progress box to stage 4

    //STAGES 4 and 5: BUILD THE LINE ARRANGEMENT AND COMPUTE BARCODE TEMPLATES

    //build the arrangement
    if (verbosity >= 2) {
        debug() << "CALCULATING ANCHORS AND BUILDING THE DCEL ARRANGEMENT";
    }

    timer.restart();
    
    std::shared_ptr<Arrangement> arrangement;
    
    //TODO: This block of code probably could be structured better;
    //it's a bit redundant.  (It's a minor point though.)
    //********
    if (!koszul)
    {
        //Copy pres into an FIRep object and use this going forward
        //TODO: This copy operation is unnecessary; eventually it shouldn't happen.
        //The best solution is to make persistence updater take a presentation.
        FIRep fir(pres, verbosity);
        ArrangementBuilder builder(verbosity);

        arrangement = builder.build_arrangement(fir, input.x_exact, input.y_exact, result->template_points, progress);
        //TODO: update this -- does not need to store list of xi support points in xi_support
        //NOTE: this also computes and stores barcode templates in the arrangement
    }
    else
    {
        ArrangementBuilder builder(verbosity);
        
        arrangement = builder.build_arrangement(input.rep(), input.x_exact, input.y_exact, result->template_points, progress); ///TODO: update this -- does not need to store list of xi support points in xi_support
        //NOTE: this also computes and stores barcode templates in the arrangement
    }
    //********
    
        
    if (verbosity >= 2) {
        debug() << "   building the line arrangement and computing all barcode templates took"
                << timer.elapsed() << "milliseconds";
    }

    //send (a pointer to) the arrangement back to the VisualizationWindow
    arrangement_ready(arrangement);
    //re-send xi support and other anchors
    if (verbosity >= 8) {
        debug() << "Sending" << result->template_points.size() << "anchors";
    }
    
    

    
    template_points_ready(TemplatePointsMessage{ input.x_label, input.y_label, result->template_points,
                                                 result->homology_dimensions, input.x_exact, input.y_exact, input.x_reverse,input.y_reverse });
    if (verbosity >= 10) {
        //TODO: Make this a separate flag, rather than using verbosity?
        arrangement->test_consistency();
    }
    result->arrangement = std::move(arrangement);
    return result;
}

std::unique_ptr<ComputationResult> Computation::compute(InputData data, bool koszul)
{
    progress.advanceProgressStage(); //update progress box to stage 3

    auto input = ComputationInput(data);
    //print bifiltration statistics
    //debug() << "Computing from raw data";
    return compute_raw(input, koszul);
}

void Computation::compute_min_pres_and_betti_nums(ComputationInput& input, MultiBetti& mb, Presentation& pres,std::unique_ptr<ComputationResult>& result)
{
    Timer timer_sub;
    timer_sub.restart();
    
    if (verbosity >= 2) {
        debug() << "COMPUTING MINIMAL PRESENTATION:";
    }
    
    //Because the assignment operator for the unsigned_matrix class doesn't work properly,
    //have to resize the object by hand before assignment.
    pres.hom_dims.resize(boost::extents[input.x_exact.size()][input.y_exact.size()]);
    
    pres = Presentation(input.rep(),progress,verbosity);
    
    if (verbosity >= 2) {
        std::cout << "COMPUTED (UNMINIMIZED) PRESENTATION!" << std::endl;
        std::cout << "Unminimized presentation has " << pres.row_ind.last()+1 << " rows and " << pres.col_ind.last()+1 << " cols." <<std::endl;
    }
    
    if (verbosity >= 4) {
        std::cout << "  --> computing the unminimized presentation took "
        << timer_sub.elapsed() << " milliseconds" << std::endl;
    }
    
    if (verbosity > 7)
    {
        std::cout << "UNMINIMIZED PRESENTATION: " << std::endl;
        pres.print();
    }
    
    timer_sub.restart();
    
    pres.minimize(verbosity);
    
    if (verbosity >= 2) {
        std::cout << "COMPUTED MINIMAL PRESENTATION!" << std::endl;
        std::cout << "Minimal presentation has " << pres.row_ind.last()+1 << " rows and " << pres.col_ind.last()+1 << " cols." <<std::endl;
    }
    
    if (verbosity >= 4) {
        std::cout << "  --> minimizing the presentation took "
        << timer_sub.elapsed() << " milliseconds" << std::endl;
    }
    
    if (verbosity > 7)
    {
        std::cout << "MINIMAL PRESENTATION: " << std::endl;
        pres.print();
    }
    
    progress.progress(95);
    
    mb.read_betti(pres);
    mb.compute_xi2(pres.hom_dims);
    
    //TODO: In the new code, the Presentation class keeps its own public
    //hom_dims matrix, so the one stored by the object named "result" is no
    //longer necessary.  However, for compatibility with the old Betti
    //number algorithm, for now I am keeping the latter.  A more uniform way
    //to do this would be to also make the hom_dims a public member of
    //MultiBetti.
    
    //NOTE: The boost matrix package actually requires to resize the matrix
    //before assignment.
    result->homology_dimensions.resize(boost::extents[pres.col_ind.width()][pres.col_ind.height()]);
    result->homology_dimensions = pres.hom_dims;
    
    //Now that I've copied the hom_dims matrix, I might as well make the
    //original one trivial.
    pres.hom_dims.resize(boost::extents[0][0]);
}



