..    include:: <isonum.txt>

Structure of RIVET
==================
The RIVET software consists of two separate executables: a command-line application called **rivet_console**, and a GUI application **rivet_gui**. 
**rivet_console** is the core computational engine of RIVET, and all visualizations are done using **rivet_gui**.  Below, TODO WHERE.  GIVE LINK) we explain in detail how these work.

**rivet_console** has several functions:

* It can compute print Hilbert functions, Bigraded, Betti numbers, and 

* It can take as input a data file on one of the formats specified in the :ref:`inputData` section of this documentation.  It can print Hilbert functions, Bigraded Betti numbers, and minimal presentations to the console.  It also can save a binary file called a *RIVET data file.



**rivet_gui** is a GUItakes as input either an augmented arrangment 


on :ref:`inputData` 

If one is not interes**rivet_compute** and **rivet_gui** are dependent, in the following senses:

* The visualizations performed by **rivet_gui** require a binary file called the *RIVET binary*, as input.  This is computed by **rivet_compute**.  The visualization data file can be computed by an explicit call to **rivet_compute** and then opened in **rivet_gui**.  Alternatively, to compute the visualization **rivet_gui** can call **rivet_compute** directly to compute the file.


  
The RIVET Pipeline
------------------
The following figure illustrates RIVET’s pipeline for working with the 2-parameter persistent homology of data.

.. figure:: images/flowchart.pdf  
    :width: 400px
    :align: center
    :height: 600px
    :figclass: align-center
   
    The RIVET pipeline.  Green items can be input directly to RIVET via a file.  Yellow items can be printed to the console.  Items with red boundary are saved in a binary file which serves as as input to RIVET’s visualization.

We now explain this pipeline.  

RIVET can accept as input a data set, a  bifiltration, or an firep.  RIVET always works with a single homology degree at a time; when giving data or a bifiltration as input, specifies the degree of homology to consider.  

RIVET accepts data in the form of either a point cloud in Rn, or a finite metric space (represented as distance matrix).  Optionally, a function on each point can be given.  If a function is given, RIVET computes a function-Rips bifiltration.  If no function is given, RIVET computes the degree-Rips bifiltration.

Given a bifiltration \\(F\\), RIVET constructs an firep for \\(H_j(F)\\) in the specified degree j.  If \\(F\\) is multi-critical, RIVET uses the trick of Chacholski et al. mentioned above to obtain an firep.

Given an firep, RIVET computes a minimal presentation of the homology module.  This computation also yields the Hilbert function of the module with almost no extra work.  The 0th and 1st bigraded Betti numbers of 2-D persistent homology module \\(M\\) can be read directly off of the minimal presentation.  Given these and the Hilbert function, a simple formula yields the 2nd bigraded Betti numbers as well.  RIVET can print the minimal presentation to the console in a column-sparse format.  RIVET can print the minimal presentation to the console in a column-sparse format.  The Hilbert function and bigraded Betti numbers can also be printed.  See REFERENCE.

RIVET uses the minimal presentation and bigraded Betti numbers of \\(M\\) to compute the *augmented arrangement*, which as noted above, is a data structure which allows for fast queries of the barcodes of 1-D slices of \\(M\\).  The augmented arrangement is a line arrangement in the right half plane, together with a barcode at each face of the line arrangement.

To support its visualizations, RIVET saves the Hilbert Function, Bigraded Betti numbers, and Augmented Arrangement in a binary file which we call the “visualization data file.”  This file is also used to perform queries for barcodes of slices from the command line.
