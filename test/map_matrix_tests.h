#include "catch.hpp"
#include "math/map_matrix.h"
#include "math/map_matrix.cpp"

TEST_CASE( "MapMatrix can be initialized", "[MapMatrix]" ) {

  MapMatrix mat = MapMatrix(3,4);
  REQUIRE( mat.height() == 3 );
  REQUIRE( mat.width() == 4 );

  MapMatrix eye = MapMatrix(3);

  MapMatrix manual_eye = MapMatrix(3,3);

  manual_eye.set(0,0);
  manual_eye.set(1,1);

  REQUIRE( !(eye == manual_eye) );

  manual_eye.set(2,2);

  REQUIRE( eye == manual_eye );

  MapMatrix explicit_eye = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1}
  };
  
  REQUIRE (eye == explicit_eye );
}
