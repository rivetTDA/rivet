.. _running_RIVET:
Running RIVET
=============

The RIVET software consists of two separate executables: the **RIVET** GUI application and a command-line application called **rivet_console**.
All visualizations are done using **RIVET** and for most users trying our software for the first time, we recommend to start with this.  
**RIVET** and **rivet_console** are dependent, in the following senses: **RIVET** calls  **rivet_console**, and **rivet_console** can precompute data files which  **RIVET** can then load and visualize.  

As we explain below, **rivet_console** also has functionality which we have found useful for working with 2-D persistence modules, even apart from visualization.

RIVET saves and loads the output of its main computations in a custom file format, which we call the *computed data* file type.
This stores:

* The augmented arrangement data structure for querying the fibered barcode
* Betti numbers
* Dimensions of all vector spaces.


The RIVET GUI
-------------

When the user runs **RIVET**, a window opens which allows the user to select a file.
This file can be either a *raw data* file in one of the input formats described in the :ref:`input_data` section of this documentation, or a *computed data* file. 

If a *raw data* file is selected, then the user must choose the homology degree: RIVET currently handles one homology degree at a time.  
After the user clicks the compute button, the visualization data is computed and the visualization is started.  
(Note that once the dimension vectors and Betti numbers are shown in the visualization, it may take a significant amount of additional time to prepare the interactive visualization of the barcodes of 1-D slices.)
Using the file menu in the GUI, the user may save a *computed data* file.

If a computed data file is selected in the file dialogue window, the RIVET visualization is started immediately.


rivet_console
-------------

** rivet_console ** allows the user to directly compute a *computed data* file from a *raw data* file, without opening the GUI.  
Optionally, **rivet_console** can directly output a minimal presentation or the Betti numbers and dimensions to the console, without computing the augmented arrangement.

Further, **rivet_console** can also take as input a *computed data* file and a second file specifying a list of lines.  
RIVET then returns the barcodes of the 1-D slices of each line, using the same fast query framework used for the interactive visualization. 
(This last feature may be useful for testing the statistical significance of features found by RIVET.)
 
Note that **rivet_console** also has some technical functionality intended only for use by the GUI.

For details on how to run **rivet_console**, see the help file, which can be accessed via the command::

	rivet_console (-h | --help)

When using **rivet_console** to return barcodes from a *computed data* file, lines are specified according to *angle* and *offset* parameters (the same parameters displayed near the bottom of the RIVET GUI).
The following diagram shows these parameters for a particular line, with *angle* denoted \\(\\theta\\) and *offset* denoted \\(t\\).

.. image:: images/line_diagram.png
   :width: 237px
   :height: 226px
   :alt: Diagram illustrating angle and offset used in RIVET
   :align: center

Note that \\(\\theta\\) gives the angle between the line and the horizontal axis, in degrees (0 to 90). 
The offset parameter \\(t\\) gives the *signed* distance from the line to the origin, which is positive if the line passes above/left of the origin and negative otherwise. 
This choice of parameters makes it possible to specify any line of nonnegative slope, including vertical lines. 

The following gives a sample line file::

	#A line that starts with a # character will be ignored, as will blank lines
	23 -0.22
	67 1.88
	10 0.92
	#100 0.92   <-- will error if uncommented, 100 > 90

For each line specified, **rivet_console** will print barcode information as a single line of text, beginning by repeating the query parameters. For example, output corresponding to the sample line file above might be::

	23 -0.22: 88.1838 inf x1, 88.1838 91.2549 x5, 88.1838 89.7194 x12
	67 0.88: 23.3613 inf x1
	10 0.92: 11.9947 inf x1, 11.9947 19.9461 x2, 11.9947 16.4909 x1, 11.9947 13.0357 x4

Note that barcodes are given with respect to a parameterization of the query line that takes zero to be the intersection of the query line with the nonnegative portions of the coordinate axes (for more details, see Appendix A.1 of `Lesnick and Wright <https://arxiv.org/abs/1512.00180>`_). 
Furthermore, barcodes are returned as multisets of intervals. 
For example, in the sample output above, ``88.1838 inf x1`` indicates a single interval \\([88.1838, \\infty)\\).

Working with a Coarsened Persistence Module
-------------------------------------------

When passing a *raw data* file to RIVET via either the GUI or the command line, the user has the option of choosing *x-bins* and *y-bins* parameters, which control how the persistence module is coarsened; if these parameters are not selected, no binning is done at all.
The runtime of RIVET and memory footprint depends on the choice of these parameters. 
If you are trying RIVET for the first time, we suggest you try small values of *x-bins*  and *y-bins* to start.  
For example, to start you might set both parameters equal to 20, and then try the computation again with a larger value afterwards.
