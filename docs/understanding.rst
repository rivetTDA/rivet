Understanding the RIVET Visualization
=====================================

This page briefly explains the RIVET visualization. For more details, see `the RIVET paper <a href="https://arxiv.org/pdf/1512.00180v1.pdf>`_. 

For \\(a,b \\in \\mathbb{R}^2\\), we write \\(a \\leq b\\) if \\(a_1 \\leq b_1\\) and \\(a_2 \\leq b_2\\).
A *2-D persistence module* \\(M\\) is a collection of vector spaces \\(\\{M_a\\}_{a\\in \\mathbb{R^2}}\\), together with a collection of linear maps \\(\\{M_{a,b}:M_a\\to M_b\\}_{a\\leq b}\\), such that \\(M_{a,a}=\\mathrm{Id}_{M_a}\\) for all \\(a\\) and \\(M_{b,c}\\circ M_{a,b}=M_{a,c}\\) for all \\(a \\leq b\\leq c\\).

RIVET visualizes three classes of invariants of a 2-D persistence module \\(M\\):

* The barcode \\(\\mathcal B(M^L)\\) of the restriction \\(M^L\\) of \\(M\\) along \\(L\\), for \\(L\\) any affine line with non-negative slope.
* The dimension of each vector space \\(M_a\\).
* The *bigraded Betti numbers* \\(\\xi_i(M)\\). These are functions \\(\\mathbb{R}^2 \\to \\mathbb{N}\\) that, respectively, count the number of births, deaths, and "relations amongst deaths" at each bigrade. Formally, given \\(r \\in \\mathbb{R}^2\\) and a minimal free resolution $$0 \\to F^2\\to F^1\\to F^0$$ for \\(M\\), \\(\\xi_i(M)(r)\\) is the number of elements at bigrade \\(r\\) in a basis for \\(F^i\\).

One key feature of RIVET is the ability to interactively select the line \\(L\\) via the mouse and have the barcode \\(\\mathcal B(M^L)\\) update in real time.

[Note that as of 2018, RIVET also computes minimal presentations of 2-parameter persistence modules, in addition to the above three invariants.  The rivet_console executable can print a minimal presentation, but the RIVET visualization does not provide any direct way to visualize or otherwise access the minimal presentation.]

The RIVET interface contains two main windows, the *Line Selection Window* and the *Persistence Diagram Window*, shown in the screenshot below.

.. image:: images/RIVET_screenshot.png
   :width: 556px
   :height: 419px
   :alt: Screenshot of the RIVET GUI
   :align: center

Line Selection Window
---------------------

The Line Selection Window plots a rectangle in \\(\\mathbb{R}^2\\) containing the union of the supports of bigraded Betti number functions \\(\\xi_i(M)\\), \\(i\\in \\{0,1,2\\}\\). 
Points in the supports of \\(\\xi_0(M)\\), \\(\\xi_1(M)\\), and \\(\\xi_2(M)\\) are marked with green, red, and yellow dots, respectively (though these colors are customizable via the Preferences tab). 
The area of each dot is proportional to the corresponding function value. 
The dots are translucent, so that, for example, overlaid red and green dots appear brown on their intersection. 
This allows the user to read the values of the Betti numbers at points which are in the support of more than one of the functions. 
Furthermore, hovering the mouse over a dot produces a popup box that gives the values of the bigraded Betti numbers at that point.

The greyscale shading at a point \\(a\\) in this rectangle represents \\(\\dim M_a\\): \\(a\\) is unshaded when \\(\\dim M_a=0\\), and larger \\(\\dim M_a\\) corresponds to darker shading. 
Hovering the mouse over \\(a\\) brings up a popup box which gives the precise value of \\(\\dim M_a\\).

The Line Selection Window contains a blue line \\(L\\) of non-negative slope, with endpoints on the boundary of the displayed region of \\(\\mathbb{R}^2\\). 
RIVET displays a barcode for \\(M^L\\) in the line selection window, provided the "show barcode" box is checked below. 
The intervals in the barcode for \\(M^L\\) are displayed in purple, perpendicularly offset from the line \\(L\\).

The user can click and drag the blue line with the mouse to change the choice of line \\(L\\).
Clicking and dragging an endpoint of the line moves that endpoint while keeping the other fixed. 
One endpoint is locked to the top and right sides of the displayed rectangle; the other endpoint is locked to the bottom and left sides.
Clicking and dragging the interior of the line (away from its endpoints) moves the line as follows:

* Left-clicking moves the line in the direction perpendicular to its slope, keeping the slope constant.
* Right-clicking changes the slope of the line, while keeping the bottom/left endpoint fixed.

As the line moves, both the barcode in the Line Selection Window and its persistence diagram representation in the Persistence Diagram Window are updated in real time. 
The "slope" and "offset" controls below the Line Selection Window can also be used to select the line.

Persistence Diagram Window
--------------------------

[NOTE: RIVET now allows the user adjust the bounds of the viewable region.  As part of this change, we introduced some refinements to the persistence diagram window, and the text here has not yet been updated to reflect that.]

The Persistence Diagram Window (at right in the screenshot above) displays a persistence diagram representation of the barcode for \\(M^L\\).

The bounds for the square viewable region (surrounded by dashed lines) in this window are chosen statically, depending on \\(M\\) but not on \\(L\\). 
Let the square \\([0,B]\\times[0,B]\\) be the viewable region. 
It may be that the barcode contains some intervals \\([\\alpha, \\beta)\\) with \\(\\alpha \\gt B\\) or \\(\\beta \\gt B\\), and it is necessary to represent these on the screen. 
For this reason, RIVET includes some information in horizontal strips at the top of the persistence diagram, which is not found in typical persistence diagrams, to represent these points.

Above the square region of persistence diagram are two narrow horizontal strips, separated by a dashed horizontal line. 
The upper strip is labeled *inf*, and the lower strip is labeled \\(\\lt\\)\ *inf*. 
RIVET plots a point in the upper strip for each interval \\([\\alpha, \\infty)\\) in the barcode with \\(\\alpha 
\\le B\\). 
RIVET plots a point in the lower strip for each interval \\([\\alpha, \\beta)\\) in the barcode with \\(\\alpha \\le B\\) and \\(B \\lt \\beta \\lt \\infty)\\).

Just to the right of each of the two horizontal strips is a number, separated from the strip by a dashed vertical line. 
The upper number is the count of intervals \\([\\alpha, \\infty)\\) in the barcode with \\(B \\lt \\alpha\\). 
The lower number is the the count of intervals \\([\\alpha, \\infty)\\) in the barcode with \\(B \\lt \\alpha, \\beta \\lt \\infty\\).

As with the bigraded Betti numbers in the Line Selection Window, the multiplicity of a point in the persistence diagram is indicated by the area of the corresponding dot. 
Additionally, hovering the mouse over a dot produces a popup that displays the multiplicity of the dot.

In the lower portion of the persistence diagram square, RIVET displays the dimension of homology being visualized and the name of the input file.


