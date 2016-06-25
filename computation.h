#pragma once

#include "interface/input_parameters.h"
#include "interface/input_manager.h"
#include "dcel/barcode_template.h"
#include "math/simplex_tree.h"
#include "math/xi_point.h"

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;
#include <boost/signals2.hpp>
#include "boost/multi_array.hpp"
typedef boost::multi_array<unsigned, 2> unsigned_matrix;
#include <vector>
#include <interface/progress.h>

class ComputationInput {
 public:
  std::vector<exact> x_exact;
  std::vector<exact> y_exact;

 protected:
    InputData data;
    ComputationInput(InputData data) :
            data(data),
            x_exact(data.x_exact),
            y_exact(data.y_exact)
    { }
};

/// Raw data from points or other sources comes with a bifiltration
class RawDataInput : public ComputationInput {
 public:
  SimplexTree& bifiltration() {
      return *(data.simplex_tree);
  }
  RawDataInput(InputData data) : ComputationInput(data)
  { }
};

/// Precomputed Rivet data comes with xi support points and barcode templates
class RivetInput : public ComputationInput {
 public:
  std::vector<xiPoint> &xi_support;
  std::vector<BarcodeTemplate> &barcode_templates;
  RivetInput(InputData data) :
          ComputationInput(data),
          xi_support(data.xi_support),
          barcode_templates(data.barcode_templates){}
};

struct ComputationResult {
  unsigned_matrix homology_dimensions;
  std::vector<xiPoint> xi_support;
  std::shared_ptr<Mesh> arrangement;
  std::shared_ptr<SimplexTree> bifiltration;
};

class Computation
{
 public:

    //TODO: these signals are a little strange, they should go away soon
    boost::signals2::signal<void(Mesh&)> arrangementReady;
    boost::signals2::signal<void(std::vector<xiPoint>)> xiSupportReady;
  Computation(InputParameters &params, Progress &progress);
  ~Computation();

  std::unique_ptr<ComputationResult> compute(InputData data);

    private:
        InputParameters& params;
        Progress &progress;

        const int verbosity;

        void find_dimensions(const RivetInput &input, unsigned_matrix &homology_dimensions);  //computes homology dimensions from the graded Betti numbers (used when data comes from a pre-computed RIVET file)

        std::unique_ptr<ComputationResult> compute_rivet(RivetInput &input);
        std::unique_ptr<ComputationResult> compute_raw(RawDataInput &input);
};

