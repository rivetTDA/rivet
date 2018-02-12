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

std::unique_ptr<ComputationResult> Computation::compute_raw(ComputationInput& input)
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

    //STAGE 3: COMPUTE MINIMAL PRESENTTION AND MULTIGRADED BETTI NUMBERS

    std::unique_ptr<ComputationResult> result(new ComputationResult);
    //compute xi_0 and xi_1 at all bigrades
    if (verbosity >= 2) {
        debug() << "COMPUTING MINIMAL PRESENTATION:";
    }
    
    MultiBetti mb(input.rep());
    
    Timer timer;
    Timer timer_sub;
    timer.restart();
    timer_sub.restart();
    
    Presentation pres(input.rep(),progress,verbosity);
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

    //TODO: Introduce an option to use either the old or new Betti number computation.
    //For now, just using the new option
    //mb.compute_koszul(result->homology_dimensions, progress);
    
    mb.read_betti(pres,result->homology_dimensions);
    mb.compute_xi2(pres.hom_dims);
    
    //TODO: In the new code, the Presentation class keeps its own public hom_dims matrix,
    //so the one stored by the object named "result" is no longer necessary.
    //However, for compatibility with the old Betti number algorithm, for now I am keeping the latter.
    //A more uniform way to do this would be to also make the hom_dims a public member of MultiBetti.
    result->homology_dimensions = pres.hom_dims;
    
    //Now that I've copied the hom_dims matrix, I might as well make the original one trivial.
    pres.hom_dims.resize(boost::extents[0][0]);
    
    if (verbosity >= 2) {
        debug() << "  -- xi_i computation took " << timer.elapsed() << " milliseconds";
    }

    //store the xi support points
    mb.store_support_points(result->template_points);

    template_points_ready(TemplatePointsMessage{ input.x_label, input.y_label, result->template_points, result->homology_dimensions, input.x_exact, input.y_exact }); //signal that xi support points are ready for visualization
    progress.advanceProgressStage(); //update progress box to stage 4

    //STAGES 4 and 5: BUILD THE LINE ARRANGEMENT AND COMPUTE BARCODE TEMPLATES

    //build the arrangement
    if (verbosity >= 2) {
        debug() << "CALCULATING ANCHORS AND BUILDING THE DCEL ARRANGEMENT";
    }

    timer.restart();
    
    //Copy pres into an FIRep object and use this going forward
    //TODO: This copy operation is unnecessary; eventually it shouldn't happen.
    //The best solution is to make persistence updater take a presentation.
    FIRep fir(pres, verbosity);
    
    ArrangementBuilder builder(verbosity);

    
    auto arrangement = builder.build_arrangement(fir, input.x_exact, input.y_exact, result->template_points, progress); ///TODO: update this -- does not need to store list of xi support points in xi_support
    //NOTE: this also computes and stores barcode templates in the arrangement

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
                                                 result->homology_dimensions, input.x_exact, input.y_exact });
    if (verbosity >= 10) {
        //TODO: Make this a separate flag, rather than using verbosity?
        arrangement->test_consistency();
    }
    result->arrangement = std::move(arrangement);
    return result;
}

std::unique_ptr<ComputationResult> Computation::compute(InputData data)
{
    progress.advanceProgressStage(); //update progress box to stage 3

    auto input = ComputationInput(data);
    //print bifiltration statistics
    //debug() << "Computing from raw data";
    return compute_raw(input);
}
