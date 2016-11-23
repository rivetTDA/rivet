#pragma once

#include "dcel/barcode_template.h"
#include "dcel/dcel.h"
#include "interface/input_manager.h"
#include "interface/input_parameters.h"
#include "math/simplex_tree.h"
#include "math/template_point.h"
#include "numerics.h"

#include "boost/multi_array.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/signals2.hpp>
#include <interface/progress.h>
#include <vector>

//TODO: Remove either this or InputData, since there's no need for both anymore now that RIVET_0 files aren't supported.
class ComputationInput {
protected:
    InputData data;

public:
    std::vector<exact> x_exact;
    std::vector<exact> y_exact;
    std::string x_label;
    std::string y_label;

    SimplexTree& bifiltration()
    {
        return *(data.simplex_tree);
    }

    ComputationInput(InputData data)
        : data(data)
        , x_exact(data.x_exact)
        , y_exact(data.y_exact)
            , x_label(data.x_label)
            , y_label(data.y_label)
    {
    }

};

struct ComputationResult {
    unsigned_matrix homology_dimensions;
    std::vector<TemplatePoint> template_points;
    std::shared_ptr<Arrangement> arrangement;
    std::shared_ptr<SimplexTree> bifiltration;
};

class Computation {
public:
    //TODO: these signals are a little strange, they should go away soon
    boost::signals2::signal<void(std::shared_ptr<Arrangement>)> arrangement_ready;
    boost::signals2::signal<void(TemplatePointsMessage)> template_points_ready;
    Computation(InputParameters& params, Progress& progress);
    ~Computation();

    std::unique_ptr<ComputationResult> compute(InputData data);

private:
    InputParameters& params;
    Progress& progress;

    const int verbosity;

    std::unique_ptr<ComputationResult> compute_raw(ComputationInput& input);
};
