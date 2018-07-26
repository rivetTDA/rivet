..    include:: <isonum.txt>
.. _structure:  

RIVET’s Computation Pipeline
====================================
The following figure illustrates RIVET’s pipeline for working with the 2-parameter persistent homology of data.

.. figure:: images/flowchart.pdf  
    :width: 400px
    :align: center
    :height: 600px
    :figclass: align-center
   
    The RIVET pipeline.  Green items can be input directly to RIVET via a file.  Yellow items can be printed to the console.  Items with red boundary are saved in a file called the *output binary*, which serves as input to RIVET’s visualization.

We now explain this pipeline:

RIVET can accept as input a data set, a  bifiltration, or an firep.  RIVET always works with a single homology degree at a time; when giving data or a bifiltration as input, specifies the degree of homology to consider.  

.. The data is input to RIVET as a file; the allowed formats of the file are specified inn ADD HERE.   

RIVET accepts data in the form of either a point cloud in \\(R^n\\), or a finite metric space (represented as distance matrix).  Optionally, a function on each point can also be given.  If a function is given, RIVET computes a function-Rips bifiltration.  If no function is given, it computes the degree-Rips bifiltration.

Given a bifiltration \\(F\\), RIVET constructs an FIrep for \\(H_j(F)\\) in the specified degree \\(j\\).  If \\(F\\) is multi-critical, RIVET uses the trick of Chacholski et al. LINK to obtain the FIrep.

Given an FIrep, RIVET computes a minimal presentation of its homology module.  This computation also yields the Hilbert function of the module with almost no extra work.  The 0th and 1st bigraded Betti numbers of a 2-D persistent homology module \\(M\\) can be read directly off of the minimal presentation.  Given these and the Hilbert function, a simple formula yields the 2nd bigraded Betti numbers as well.

RIVET uses the minimal presentation of \\(M\\) to compute the *augmented arrangement* of \\(M\\).  This is a line arrangement in the right half plane, together with a barcode at each face of the line arrangement.  The augmented arrangement is used to perform fast queries of the fibered barcode of \\(M\\).



