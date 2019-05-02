Frequently Asked Questions
==========================

How large a bifiltration can RIVET handle?
------------------------------------------

The computation time and memory footprint of RIVET are controlled by coarsening the bifiltration, so that the bigrades of appearance of simplices in the bifiltration live on a :math:`k_x \times k_y` grid for some small values of :math:`k_x` and :math:`k_y`.

Coarsening our bifiltrations at bit (say :math:`k_x = k_y = 50`), we have been able to comfortably handle bifiltrations coming from real data with more than 150 million simplices.  
For example, we recently used RIVET to analyze the first persistent homology of a full Vietoris-Rips bifiltration with 1000 points, and the 0th persistent homology of a full Vietoris-Rips bifiltration with 15000 points.
On a computer with a lot of RAM, the current version RIVET may be able to handle significantly larger examples than that; we have not yet invested much time in exploring the limits.  Whatever those limits are, we expect RIVET to continue to become much faster and more memory efficient in the near future, as we continue to optimize the code.

(If one is only interested in computing minimal presentations, Hilbert functions, or bigraded Betti numbers, one can get away with much less coarsening; RIVET’s augmented arrangement computation is the part that is most sensitive to coarsening.)

Visualizing 0th persistent homology modules in RIVET is often much less expensive than for higher degree homology modules, and 0-th persistent homology is often more interesting in the two-parameter setting than in the ordinary one-parameter setting.  
Thus, if your data set is large, you might try RIVET first with a 0th homology persistence computation. 


What's next for RIVET?
----------------------

* Python and RUST API’s are on their way.  These provide functions which compute distances between bipersistence modules.  
* We’d like to have code to natively support a wider range of input data types, e.g. bifiltered cubical complexes.  If you are interested in helping us with this, please let us know.
* Other improvements and extensions are coming soon. 



