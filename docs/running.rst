.. _runningRIVET:

Running RIVET
=============

The RIVET software consists of two separate but closely related executables: a command-line application called **rivet_console**, and a GUI application **rivet_GUI**.  The executable **rivet_console** is the computational engine of RIVET; it implements the computation pipeline described in the previous section.  **rivet_GUI** is responsible for RIVET’s visualizations.  

**rivet_console**
--------------------------

**rivet_console** has three main functions: 

* Given the input of a *raw data* file in one of the formats described in the :ref:`inputData` section of this documentation, **rivet_console** can compute a file called *output binary*.  The *output binary* stores the Hilbert function, bigraded Betti numbers, and augmented arrangment of a persistent homology module of the input data.  The *output binary is used by the RIVET visualization, and also for the following:

* Given an *output binary* of a 2-D persistence module \\(M\\) and a second file, the *line file*, specifying a list of lines, **rivet_console** prints the barcodes of the 1-D slices of each line to the console.  The computations are performed using fast queries of the augmented arrangment of \\(M\\).

* Given a *raw data* file as input, **rivet_console** can print The Hilbert function and Bigraded Betti numbers of a persistent homology module of the input data.  It can also print a minimal presentation of the module.

For details on how to run **rivet_console, see the help information, which can be accessed via the command::

	rivet_console (-h | --help)
	
Note that **rivet_console** also has some technical functionality intended only for use by the GUI.

TODO: Perhaps and move some of that help information into the documentation.

In what follows we specify the format of the *line file* and the format of the output **rivet_console** prints to the console.

Format of a *line file*
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
A *line file* specifies a list of affine lines in \\(\\mathbb R^2\\) with non-negative slope.  Each line is specified by its *angle* and *offset* parameters (the same parameters displayed near the bottom of the RIVET GUI).  TODO:  THIS IS A FORWARD REFERENCE.  ADD LINK TO THE APPROPRIATE PLACE?

The following diagram shows these parameters for a particular line, with *angle* denoted \\(\\theta\\) and *offset* denoted \\(t\\).

.. image:: images/line_diagram.png
   :width: 237px
   :height: 226px
   :alt: Diagram illustrating angle and offset used in RIVET
   :align: center

As the diagram indicates, \\(\\theta\\) is the angle between the line and the horizontal axis in degrees (0 to 90). 
The offset parameter \\(t\\) is the *signed* distance from the line to the origin, which is positive if the line passes above/left of the origin and negative otherwise. 
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





**rivet_gui**
^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
The visualizations performed by **rivet_GUI** require an *output binary* as input.  This can be computed by an explicit call to **rivet_console** and then opened in **rivet_GUI**.  Alternatively, **rivet_GUI** can call **rivet_console** directly to compute the output binary.

When the user runs **rivet_GUI**, a window opens which allows the user to select a file.
This file can be either a *raw data* file in one of the input formats described in the :ref:`inputData` section of this documentation, or am *output binary* file.

[TODO: ADD FIGURE HERE?”]

If a *raw data* file is selected, then (unless the file is of type firep) the user must choose the homology degree: RIVET currently handles one homology degree at a time.  
After the user clicks the compute button, the *output binary* is computed via a call to **rivet_console** and the visualization is started.  (Note that once the Hilbert Function and Betti numbers are shown in the visualization, it may take a significant amount of additional time to prepare the interactive visualization of the barcodes of 1-D slices.)
Using the file menu in the GUI, the user may save an *output binary* file.

If an *output binary* file is selected in the file dialogue window, the data in the file is loaded immediately into the RIVET visualization, and the visualization begins. 

The next section explains the RIVET visualization.



Computation of a *output binary*
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The **rivet_console** executable allows the user to directly compute a *computed data* file from a *raw data* file, without opening the GUI.  

Printing Minimal Presentation to Console
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Optionally, **rivet_console** can directly output a minimal presentation or the Betti numbers and dimensions to the console, without computing the augmented arrangement.


Printing Hilbert Function and Bigraded Betti numbers to Console
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



Working with a Coarsened Persistence Module
-------------------------------------------

When passing a *raw data* file to RIVET via either the GUI or the command line, the user has the option of choosing *x-bins* and *y-bins* parameters, which control how the persistence module is coarsened; if these parameters are not selected, no binning is done at all.
The runtime of RIVET and memory footprint depends on the choice of these parameters. 
If you are trying RIVET for the first time, we suggest you try small values of *x-bins*  and *y-bins* to start.  
For example, to start you might set both parameters equal to 20, and then try the computation again with a larger value afterwards.
