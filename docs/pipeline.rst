..    include:: <isonum.txt>
.. _structure:  

Computation Pipeline
====================================
The following figure illustrates RIVET’s pipeline for working with the 2-parameter persistent homology of data.

.. figure:: images/flowchart.*
        :width: 400px
        :align: center
        :height: 600px
        :figclass: align-center
       
        The RIVET pipeline.  Green items can be input directly to RIVET via a file.  Yellow items can be printed to the console. Items with red boundary can be saved in a *module invariants file*, which serves as input to RIVET’s visualization.


We now explain this pipeline:

RIVET can accept as input a data set, a bifiltration, or an FIRep.  RIVET always works with a single homology degree at a time; when giving data or a bifiltration as input, one specifies the degree of homology to consider.

RIVET accepts data in the form of either a point cloud in :math:`\mathbb R^n`, or a finite metric space (represented as distance matrix).  Optionally, a function on the points can also be given by the user or precomputed by RIVET. If a function is specified, then RIVET computes a function-Rips bifiltration.  If no function is specified, then RIVET computes the degree-Rips bifiltration.

Given a bifiltration :math:`F`, RIVET constructs an FIRep for :math:`H_j(F)` in the specified degree :math:`j`.  If :math:`F` is multi-critical, RIVET uses the trick of Chacholski et al. LINK to obtain the FIRep.

Given an FIRep, RIVET computes a minimal presentation of its homology module.  This computation also yields the Hilbert function of the module with almost no extra work.  The 0th and 1st bigraded Betti numbers of a bipersistence module :math:`M` can be read directly off of the minimal presentation.  Given these and the Hilbert function, a simple formula yields the 2nd bigraded Betti numbers as well.

RIVET uses the minimal presentation of :math:`M` to compute the *augmented arrangement* of :math:`M`.  This is a line arrangement in the right half plane, together with a barcode at each face of the line arrangement.  The augmented arrangement is used to perform fast queries of the fibered barcode of :math:`M`.



