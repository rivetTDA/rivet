#pragma once

//forward declarations
class InputManager;
struct InputParameters;
class Mesh;

#include "dcel/barcode_template.h"
#include "math/simplex_tree.h"
#include "math/xi_point.h"

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;
#include "boost/multi_array.hpp"
typedef boost::multi_array<unsigned, 2> unsigned_matrix;

#include <vector>

struct ComputationResult {
  std::vector<double>& x_grades;
  std::vector<exact>& x_exact;
  std::vector<double>& y_grades;
  std::vector<exact>& y_exact;
  std::vector<xiPoint>& xi_support;
  unsigned_matrix& homology_dimensions;
  Mesh* arrangement;
  SimplexTree* bifiltration;
  std::vector<BarcodeTemplate> barcode_templates; //only used if we read a RIVET data file and need to store the barcode templates before the arrangement is ready
};

class Computation
{
 public:
  Computation(InputParameters& params,
              std::function<void(Mesh*)> arrangementReady,
              std::function<void()> xiSupportReady,
              std::function<void(unsigned)> progress);
  ~Computation();

  void compute();

    private:
        InputParameters& params;

        const int verbosity;

        void find_dimensions();  //computes homology dimensions from the graded Betti numbers (used when data comes from a pre-computed RIVET file)
};

