.. _visualization:

The RIVET Visualization: Further Details
========================================
In the section :ref:`overviewVisualization`, we gave brief overview of RIVET's visualization of bipersistence modules.  Here we provide a more detailed description.  To make this self-contained, we revisit the material of :ref:`overviewVisualization` along the way.


**Handling MI Files with rivet_GUI**
---------------------------------------

As explained earlier, RIVET's visualizations are handled by the executable **rivet_GUI**, and require an MI file as input.  The MI file can be computed by a direct call to **rivet_console**,, as explained on the :ref:`rivet console' page, and then opened in **rivet_GUI**.  Alternatively, **rivet_GUI** can call **rivet_console** to compute the MI file.  (The example on the :ref:`gettingstarted` page is an instance of the latter approach, though MI files were not explicitly mentioned there.)

When the user runs **rivet_GUI**, a window opens which allows the user to select a file
This file can be either an input data file in one of the input formats described in :ref:`inputData`, or a MI file.

If an input data file is chosen, the GUI allows the user to graphically select options for  computation of a MI file, as we have seen earlier on the :ref:`gettingstarted` page.  Most  options that can be selected via a command line flag, as described on the :ref:`rivetconsole' page, can also be selected in the GUI.  (However, some technical options, such as choosing the maximum number of cores to use in a parallel computation, are not available through the **rivet_GUI**.)  After the user clicks the *Compute* button, the MI file is computed via a call to **rivet_console** and the visualization is started.  
(Note that after the Hilbert Function and Betti numbers are shown in the visualization, it may take a significant amount of additional time to prepare the interactive visualization of the barcodes of 1-D slices.). Using the *File* menu in the GUI, the user may save the MI file; a module invariants file computed by **rivet_GUI** is not saved automatically.

If an MI file is selected in the file dialogue window, the data in the file is loaded immediately into the RIVET visualization, and the visualization begins. 


Line Selection Window
---------------------

The RIVET interface contains two main windows, the *Line Selection Window* and the *Persistence Diagram Window*, shown in the screenshot below.

.. image:: images/RIVET_screenshot_circle300_balldensity.png
   :width: 600px
   :height: 449px
   :alt: The file input dialog box with selected options
   :align: center


The *Line Selection Window* not only visualizes the Hilbert function values and the bigraded Betti numbers of a bipersistence module :math:`M`, but also allows the user choose linear slices along which barcodes are displayed. 

By default, the *Line Selection Window* plots a rectangle in :math:`\mathbb{R}^2` containing the union of the supports of bigraded Betti number functions :math:`\xi_i^M`, :math:`i\in \{0,1,2\}`.
(If the input to RIVET is an firep and the Betti numbers are not supported on a horizontal or vertical line, this will be the smallest such rectangle.  If the input is a point cloud, metric space, or bifiltration, and the birth indices of all simplices in the bifiltration do not lie on a single line, the rectangle will be the smallest one containing the birth indices of all simplices.)

The Hilbert function values are shown as grayscale shading.
Specifically, the greyscale shading at a point :math:`a` in this rectangle represents :math:`\dim M_a`: :math:`a` is unshaded when :math:`\dim M_a=0`, and larger :math:`\dim M_a` corresponds to darker shading. 
Hovering the mouse over :math:`a` brings up a popup box which gives the precise value of :math:`\dim M_a`.

Points in the supports of :math:`\xi_0^M`, :math:`\xi_1^M`, and :math:`\xi_2^M` are marked with green, red, and yellow dots, respectively (though these colors are customizable via the Edit menu on Linux or the Preferences menu on Mac). 
The area of each dot is proportional to the corresponding function value. 
The dots are translucent, so that, for example, overlaid red and green dots appear brown on their intersection. 
This allows the user to read the values of the Betti numbers at points which are in the support of more than one of the functions. 
Furthermore, hovering the mouse over a dot produces a popup box that gives the values of the bigraded Betti numbers at that point.

A key feature of the RIVET visualization is the ability to interactively select the line :math:`L` via the mouse and have the barcode :math:`\mathcal B(M^L)` update in real time.
The Line Selection Window contains a blue line :math:`L` of non-negative slope, with endpoints on the boundary of the displayed region of :math:`\mathbb{R}^2`. 
RIVET displays a barcode for :math:`M^L` in the line selection window, provided the "show barcode" box is checked below. 
The intervals in the barcode for :math:`M^L` are displayed in purple, perpendicularly offset from the line :math:`L`.

The user can click and drag the blue line with the mouse to change the choice of line :math:`L`.
Clicking and dragging an endpoint of the line moves that endpoint while keeping the other fixed. 
One endpoint is locked to the top and right sides of the displayed rectangle; the other endpoint is locked to the bottom and left sides.
Clicking and dragging the interior of the line (away from its endpoints) moves the line as follows:

* Left-clicking moves the line in the direction perpendicular to its slope, keeping the slope constant.
* Right-clicking changes the slope of the line, while keeping the bottom/left endpoint fixed.

As the line moves, both the barcode in the Line Selection Window and its persistence diagram representation in the Persistence Diagram Window are updated in real time. 
The *Angle* and *Offset* controls below the Line Selection Window can also be used to select the line.

The coordinate bounds of the viewable rectangle may be adjusted using the *Top*, *Bottom*, *Left*, and *Right* control boxes at the bottom of the RIVET window.
The window can be reset to the default by choosing *View → Restore Default Window*.  Choosing *View → Betti number window* sets the window to the smallest rectangle containing all non-zero Betti numbers.


Persistence Diagram Window
--------------------------

The *Persistence Diagram Window* (at right in the screenshot above) displays a persistence diagram representation of the barcode for :math:`M^L`.

The bounds for the square viewable region (surrounded by dashed lines) in this window are chosen automatically.  They depend on the bounds of the viewable region in the slice diagram window, but not on :math:`L`.

Let the square :math:`[0,B]\times[0,B]` be the viewable region.  It may be that the barcode contains some intervals :math:`[\alpha,\beta)` with either :math:`\alpha` or :math:`\beta` not contained in :math:`[0,B]`.  To represent such intervals on the screen, RIVET displays some information at the top and left of the persistence diagram which is not found in typical persistence diagrams.

Above the square region of persistence diagram are two narrow horizontal strips, separated by a dashed horizontal line. 
The upper strip is labeled *inf*, and the lower strip is labeled :math:`\lt`\ *inf*. 
RIVET plots a point in the upper strip for each interval :math:`[\alpha, \infty)` in the barcode with :math:`0\leq \alpha 
\le B`. 
RIVET plots a point in the lower strip for each interval :math:`[\alpha, \beta)` in the barcode with :math:`0\leq \alpha \le B\lt \beta \lt \infty)`.  

To the left of the square region of persistence diagram is a vertical strip labeled - *inf* :math:`\lt`.  RIVET plots a point in this strip for each interval :math:`[\alpha, \beta)` in the barcode with :math:`\alpha \lt 0\leq \beta \leq  B)`.  

Just to the right and to the left of each of the two upper horizontal strips is a number, separated from the strip by a dashed vertical line:  

* To the upper right is the number of intervals :math:`[\alpha, \infty)` in the barcode with :math:`B \lt \alpha`. 
* To the lower right is the the number of intervals :math:`[\alpha, \beta)` with :math:`B \lt \alpha` and :math:`\beta \lt \infty`.
* To upper left is the number of intervals :math:`[\alpha, \infty)` with :math:`\alpha\lt 0`.  
* To the lower left is the number of intervals :math:`[\alpha, \beta)` with :math:`\alpha< 0` and :math:`B\lt\beta \lt\infty`.    

Finally, there is a number in the bottom left corner of the persistence diagram window.  This is the number of intervals :math:`[\alpha, \beta)` with :math:`\alpha\lt \beta<0`. 

As with the bigraded Betti numbers in the Line Selection Window, the multiplicity of a point in the persistence diagram is indicated by the area of the corresponding dot. 
Additionally, hovering the mouse over a dot produces a popup that displays the multiplicity of the dot.


Customizing the Visualization
----------------------------------------------

The look of the visualization can be customized by choosing *RIVET → Preferences* on Mac, or *Edit → Configure* on Linux, and adjusting the settings there.  
