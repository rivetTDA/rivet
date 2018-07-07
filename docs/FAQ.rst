Frequently Asked Questions
==========================


Can RIVET handle arbitrary bifiltrations?
-----------------------------------------

Yes. The most flexible option for RIVET input is the :ref:`firep` input type.


How large a bifiltration can RIVET handle?
------------------------------------------

The computation time and memory footprint of RIVET are controlled by coarsening the bifiltration, so that the bigrades of appearance of simplices in the bifiltration live on a \\(k_x \\times k_y\\) grid for some small values of \\(k_x\\) and \\(k_y\\).

Coarsening our bifiltrations at bit (say \\(k_x = k_y = 50\\)), we have been able to comfortably handle bifiltrations coming from real data with more than 150 million simplices.  
For example, we recently used RIVET to analyze the first persistent homology of a full Vietoris-Rips bifiltration with 1000 points, and the 0th persistent homology of a full Vietoris-Rips bifiltration with 15000 points.  
On a computer with a lot of RAM, the current version RIVET may be able to handle significantly larger examples than that; we have not yet invested much time in exploring the limits. 

Whatever those limits are, we expect RIVET to continue to become much faster and more memory efficient in the near future, as we continue to optimize the code.

Visualizing 0th persistent homology modules in RIVET is often much less expensive than for higher degree homology modules, and 0-th persistent homology is often more interesting in the two-parameter setting than in the ordinary one-parameter setting.  
Thus, if your data set is large, you might try RIVET first with a 0th homology persistence computation. 


Can I be involved with RIVET development?
-----------------------------------------

If you are interested in working on the development of RIVET or, more generally on the computational aspects of multi-D persistent homology, please let us know.
There is a huge amount to do, both on the math side and the implementation side.  
We'd be happy to talk to you about this.


What's next for RIVET?
----------------------

To start, better documentation, with some worked examples.
