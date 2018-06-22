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

#pragma once

#include "dcel/barcode_template.h"
#include "dcel/dcel.h"
#include "interface/input_manager.h"
#include "interface/input_parameters.h"
#include "math/bifiltration_data.h"
#include "math/firep.h"
#include "math/template_point.h"
#include "numerics.h"

#include "boost/multi_array.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/signals2.hpp>
#include <interface/progress.h>
#include <vector>

class Presentation;
class MultiBetti;

//TODO: Remove either this or InputData, since there's no need for both anymore now that RIVET_0 files aren't supported.
class ComputationInput {
protected:
    InputData data;

public:
    std::vector<exact> x_exact;
    std::vector<exact> y_exact;
    std::string x_label;
    std::string y_label;

    bool x_reverse;
    bool y_reverse;
    BifiltrationData& bifiltration()
    {
        return *(data.bifiltration_data);
    }
    
    FIRep& rep()
    {
        return *(data.free_implicit_rep);
    }

    ComputationInput(InputData data)
        : data(data)
        , x_exact(data.x_exact)
        , y_exact(data.y_exact)
        , x_label(data.x_label)
        , y_label(data.y_label)
        , x_reverse(data.x_reverse)
        , y_reverse(data.y_reverse)
    {
    }
};

struct ComputationResult {
    unsigned_matrix homology_dimensions;
    std::vector<TemplatePoint> template_points;
    std::shared_ptr<Arrangement> arrangement;
    //std::shared_ptr<FIRep> bifiltration;
};

class Computation {
public:
    //TODO: these signals are a little strange.  It would be better to do away
    //with these, and to factor Computation::compute_raw into several functions
    //that can be called separately depending on the arguments to rivet_console.
    boost::signals2::signal<void(std::shared_ptr<Arrangement>)> arrangement_ready;
    boost::signals2::signal<void(TemplatePointsMessage)> template_points_ready;
    boost::signals2::signal<void(const Presentation&)> minpres_ready;
    Computation(int vrbsty, Progress& progress);
    ~Computation();

    std::unique_ptr<ComputationResult> compute(InputData data, bool koszul);

private:
    Progress& progress;

    const int verbosity;

    std::unique_ptr<ComputationResult> compute_raw(ComputationInput& input, bool koszul);
    
    void compute_min_pres_and_betti_nums(ComputationInput& input, MultiBetti& mb, Presentation& pres, std::unique_ptr<ComputationResult>& result);
};
