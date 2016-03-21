#include "catch.hpp"
#include "math/map_matrix.h"
#include "math/map_matrix.cpp"

TEST_CASE( "MapMatrix can be initialized", "[MapMatrix]" ) {

  MapMatrix mat = MapMatrix(3,4);
  REQUIRE( mat.height() == 3 );
  REQUIRE( mat.width() == 4 );
}
