Example
=====================================

This page demonstrates how to use RIVET to analyze point-cloud data. 

We will analyze the point cloud pictured below, which is a subset of \\(\\mathbb{R}^2\\). 
Of course, in a typical high-dimensional data setting, we would not be able to easily visualize the point cloud.
Yet, because this is a tutorial, we begin with this plot.

.. image:: images/circle1.png
   :width: 400px
   :height: 340px
   :alt: Point cloud
   :align: center

In the plot, it is apparent that the point cloud contains a dense circle of points, as if many of the points were sampled from an annulus. 
However, we observe outliers, both inside and outside of the annulus.
Note that one-parameter persistence (e.g., using a Rips filtration) cannot easily detect the dense cycle, due to the presence of outliers.


Function-Rips Filtration
----------------------------

A common type of input data for RIVET consists of a point cloud with a real-valued function.
We begin by computing values of a density estimator on the point cloud pictured above.
There are many density estimators available: for example, one might count the number of neighbors within some distance of each point, or the distance from each point to its kth nearest neighbor.
We count the number of neighbors in a disc around each point.  (TODO: Specify the value of k.)  The following diagram illustrates these counts by colors.

.. image:: images/circle2.png
   :width: 460px
   :height: 350px
   :alt: Point cloud with density estimator
   :align: center

We then prepare an input file for RIVET.
The input file is a text file written in the format described on the :ref:`inputData` page.
The first lines of the text file appear below (the complete file is in the repository at data/Test_Point_Clouds/circle_300pts_density.txt)::

	points
	2
	4
	[-] density
	1.57	2.40	20
	1.21	2.70	22
	0.79	-2.44	32
	...

The first four lines are parameters for RIVET: ``points`` tells RIVET that the file contains point-cloud data, ``2`` indicates that each point is specified by two coordinates, ``4`` is an experimentally-chosen parameter that gives the maximum length of edges to be constructed in the Rips filtration, and ``density`` is a text label for the horizontal axis in RIVET. 
The ``[-]`` preceeding the axis label tells RIVET that the values on the axis will appear in decreasing order.


We proceed using the the RIVET GUI.
With RIVET properly installed, run ``rivet_GUI``, and the input data dialog box will appear.
Select the data file and computation parameters, as shown below.
Since we are interested in detecting a cycle, we select homology dimension 1.
We must also set parameters for the coarseness of the computation, via the *bins* selectors. 
These cause RIVET to round the computed values for density and distance to a specified number of equally-spaced values, which speeds the computation.
We will choose 40 bins in each direction.

.. image:: images/RIVET_input_dialog.png
   :width: 480px
   :height: 344px
   :alt: RIVET input data dialog box
   :align: center

(Note that setting the bin selectors to *zero* will cause RIVET not to round the computed values, which would increase computation time and memory usage.)

Click **Compute**, and RIVET will compute the augmented arrangement. 
Note that this may take several minutes, depending on the computing power available.

When the Hilbert function and bigraded Betti numbers have been computed, visualizations of these appear in the *Line Selection Window* on the left side of the RIVET window (see the :ref:`visualization` page for more details).
When the computation of the augmented arrangement is complete, a barcode appears in the *Line Selection Window* and a persistence diagram in the *Persistence Diagram* window in RIVET, as shown below.
RIVET is now ready for interactive browsing of barcodes along linear slices through the two-parameter persistence module.

.. image:: images/RIVET_screenshot_circle300.png
   :width: 600px
   :height: 450px
   :alt: RIVET visualization window
   :align: center

For this data, note that the barcode contains a single long bar when the selected line goes roughly from the lower-left corner to the upper-right corner of the Line Selection Window.
This single long bar corresponds to the dense cycle of points in the point cloud.

Furthermore, note that selecting vertical lines effectively thresholds the points by density.
That is, selecting a vertical line with density value \\(d\\) produces a barcode computed from a Rips filtration on only those points with density value greater than \\(d\\), as shown below.
This effectively reduces the analysis to one-parameter persistence, using a density threshold.
The RIVET GUI allows the user to slide the vertical line left and right, exploring the consequences of different choices of the density threshold.

.. image:: images/RIVET_screenshot_circle300v.png
   :width: 600px
   :height: 450px
   :alt: RIVET visualization window
   :align: center

Rather than using the RIVET GUI, one may use the RIVET console application to compute the augmented arrangement and even obtain barcodes.
This is done using the command line, as described in :ref:`runningRIVET`.
For example, the computation described above can be obtained from **rivet_console** using the following command, run from the root directory of the RIVET repository::

	./rivet_GUI data/Test_Point_Clouds/circle_300pts_density.txt circle_300_computed.mif -H 1 -X 40 -y 40

This will produce a module invariants file ``circle_300_computed.mif``, which may then be loaded into the RIVET GUI or queried for barcodes on a collection of user-chosen lines.
Please see :ref:`runningRIVET` for more details.



Degree-Rips Filtration
-------------------------


RIVET is able to compute a degree-Rips bifiltration from point cloud data.

The procedure for doing this is similar to that described above, except that line 4 of the input file is edited to contain the text ``no function``.

Using RIVET GUI, the user must still select the homology dimension and number of bins.
RIVET then produces the following visualization:

Alternately, one may use rivet_console to compute the augmented arrangement, obtaining a computed invariants file.
This file may then be loaded into RIVET GUI for interactive barcode exploration, or barcodes may be queried using rivet_console.



