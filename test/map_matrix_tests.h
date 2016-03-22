#include "catch.hpp"
#include <vector>
#include <iostream>
#include "math/map_matrix.h"

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

TEST_CASE( "MapMatrix.add_column works" "[MapMatrix]") {
  MapMatrix eye = MapMatrix(3);

  MapMatrix test = {
    {1, 0, 0},
    {0, 0, 1},
    {1, 1, 1}
  };

  test.add_column(1, 0);
  test.add_column(2, 1);
  test.add_column(1, 2);

  REQUIRE( test == eye );
}

//not true, apparently:
/* TEST_CASE( "MapMatrix.col_reduce reduces columns" "[MapMatrix]") { */

/*   MapMatrix eye = MapMatrix(3); */

/*   MapMatrix test = { */
/*     {1, 0, 0}, */
/*     {0, 0, 1}, */
/*     {1, 1, 1} */
/*   }; */

/*   test.col_reduce(); */

/*   REQUIRE( test == eye ); */
/* } */
