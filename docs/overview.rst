..    include:: <isonum.txt>

Overview of RIVET
=====================================

The RIVET Pipeline
------------------
Here is the basic pipeline for computing 2-parameter persistent homology of data in RIVET:

Data |rarr| Bifiltration |rarr| Chain complex of free modules |rarr| Minimal Presentation of Homology Module |rarr| Invariants.

We give give a brief explanation of this pipeline here, referring the reader to the RIVET paper and related literature for more details.  

We define an FIRep to be a chain complex of free 2-parameter persistence modules of length 3.  A presentation of a 2-parameter persistence module can be thought of as a special case of an FIRep, where the last module is trivial.

RIVET can accept as input a data set, a (simplicial) bifiltration, or an FIRep.  RIVET always works with a single homology degree at a time; when giving data or a bifiltration as input, one must specificy the degree of homology to consider.  Thus RIVET needs only to consider chain complexes of length at most 3 (i.e., those specifying a single homology module.)  

RIVET accepts data in the form of either a point cloud in Rn, or a finite metric space (represented as distance matrix).  Optionally, a function on each point can be given.  If a function is given, RIVET computes the Vietoris-Rips bifiltration of the data.  If no function is given, RIVET computes the degree-Rips bifiltration of the data.  See the  RIVET paper for the definitions of these bifiltration.  (We hope that in the future RIVET will build a wider class of bifiltrations for a wider range of data types.  In the meantime, the user should be able to work with any type of data and bifiltration (subject to complexity constraints) by directly passing a bifiltration or FIRep as input.)

RIVET can handle multi-critical bifiltrations, i.e., bifiltrations where simplices are born at multiple incomparable grades in R^2.  In particular, the degree-Rips bifiltration is multi-critical.  As noted by Chacholski et al., the simplicial chain complex of a multi-critical bififiltration needn’t be free, but for fixed j >= 0, it is easy to obtain an FIRep whose homology is isomorphic to the homology in degree j.  This is what RIVET does. 

RIVET computes and visualizes three invariants of a 2-parameter persistent homology module:  

* Bigraded Betti numbers. 

* Hilbert Functions (i.e., the pointwise dimension function).  

* Barcodes of the restriction of the module along affine lines.  

In addition, RIVET can directly output a minimal presentation.  Those who wish to work with 2-parameter persistent homology in ways not supported by RIVET may find it useful to take the minimal presentations output by RIVET as a starting point.  In practice, these presentations are often surprisingly small, particularly when one coarsens the module (see SECTION).