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
    if (verbosity >= 2) {
        debug() << "\nBIFILTRATION:";
        debug() << "   Number of simplices of dimension " << params.dim << " : " << input.bifiltration().get_size(params.dim);
        debug() << "   Number of simplices of dimension " << (params.dim + 1) << " : " << input.bifiltration().get_size(params.dim + 1);
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

    //STAGE 3: COMPUTE MULTIGRADED BETTI NUMBERS

    std::unique_ptr<ComputationResult> result(new ComputationResult);
    //compute xi_0 and xi_1 at all bigrades
    if (verbosity >= 2) {
        debug() << "COMPUTING xi_0, xi_1, AND xi_2 FOR HOMOLOGY DIMENSION " << params.dim << ":";
    }
    MultiBetti mb(input.bifiltration(), params.dim);
    Timer timer;
    mb.compute(result->homology_dimensions, progress);
    mb.compute_xi2(result->homology_dimensions);

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
    ArrangementBuilder builder(verbosity);
    auto arrangement = builder.build_arrangement(mb, input.x_exact, input.y_exact, result->template_points, progress); ///TODO: update this -- does not need to store list of xi support points in xi_support
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
