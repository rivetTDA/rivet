.. _oldInputData:

Old Input Data
===============

This page contains specifications for the input files used by version 1.0 of RIVET.
RIVET 1.1 introduced more user-friendly input file formats, that allow the specification of input parameters using syntax that matches the command-line flags uzed by ``rivet_console``.
Detailed specifications for the new input formats are found in ":ref:`inputData`".

The old input formats from RIVET 1.0, as described below, are still supported in RIVET 1.1. 
However, their use is discouraged, and support for these file formats might be dropped in future versions of RIVET.

Text input files for RIVET 1.0 may encode any of the following types of data:

* Point Cloud or finite metric space, with or without a real-valued function. 
* Bifiltration
* FIRep (i.e., short chain complex of free modules).

We now specify the formats of the input data file for each of these types of input.

**NOTE**: RIVET ignores lines that begin with the symbol `#`; such lines may be used for comments.  Blank lines are also ignored.


Point Cloud with a Function
---------------------------

This format specifies a set of points :math:`X` in Euclidean :math:`n`-space, a function :math:`\gamma:X\to \mathbb R`, and a maximum scale parameter :math:`d`.  Given this, RIVET builds the function-Rips bifltration :math:`R(\gamma)`, including only simplices with diameter at most :math:`d`. 

The file has the following format:

#. The first (non-empty, uncommented) line contains the word "points" and no other characters.
#. The second line specifies the dimension :math:`n` of Euclidean space in which the point cloud is embedded.
#. The third line specifies the maximum distance :math:`d` of edges constructed in the Vietoris-Rips complex. This must be a positive number (integer or decimal).
#. The fourth line gives the label for the axis along which the values of :math:`\gamma` appear.
#. The remaining lines of the file specify the points, one point per line. Each line must specify the coordinates of a point (:math:`n` decimal numbers specified by white space), followed by the value of :math:`\gamma` on the point.

Here is an example with three points in :math:`\mathbb R^2`::

	points
	2
	3.2
	birth time
	0 0 3
	1.1 2 0.5
	-2 3 4

Putting the characters ``[-]`` at the beginning of the line before the label tells RIVET to take the filtration direction on vertices to be descending rather than ascending, as in the following example::

	points
	2
	3.2
	[-] birth time 
	0 0 3
	1.1 2 0.5
	-2 3 4

This is useful, e.g.,  when taking :math:`\gamma` to be a density function.

Finite Metric Space with Function
---------------------------------

This format is similar to the one just described, except one specifies the entries of a symmetric distance matrix rather than the coordinates of points in :math:`\mathbb R^n`.  
As above, RIVET constructs a function-Rips bifiltration from the input.  
The given distances are not required to satisfy the triangle inequality.

The file has the following format:

#. The first (non-empty, uncommented) line contains the word the word "metric" and no other printed characters.
#. The second line gives the label for the function :math:`\gamma`.
#. The third line specifies :math:`\gamma`. This line consists of a list of :math:`n` decimal numbers, separated by white space.
#. The fourth line gives the label for the "distance" axis.
#. The fifth line specifies the maximum distance :math:`d` of edges constructed in the Vietoris-Rips complex. This must be a positive number (integer or decimal).
#. The remaining line(s) of the file specify the distances between pairs of points. These distances appear as :math:`\frac{n(n-1)}{2}` numbers (integer or decimal), separated by white space or line breaks. Let the points be denoted :math:`p_1, p_2, \ldots, p_n`. The first :math:`n-1` numbers are the distance from :math:`p_1` to :math:`p_2, \ldots, p_n`. The next :math:`n-2` numbers give the distances from :math:`p_2` to :math:`p_3, \ldots, p_n`, and so on. The last number gives the distance from :math:`p_{n-1}` to :math:`p_n`.

Here is an example, for a metric space of cardinality 3::

	metric
	birth time
	1 1.1 -2
	geodesic distance
	2.5
	2 3.2
	1.25

As above, we can reverse the filtration direction on vertices by placing ``[-]`` at the beginning of the appropriate label.

Point Cloud / Finite Metric Space without Function
-----------------------------------------------------------------------------

Given either a point cloud in Euclidean space or a finite metric space with no function on vertices specified, RIVET constructs the degree-Rips bifiltration.

A point cloud with no function is specified as in the following example::

	points
	2
	3.2
	no function
	0 0 
	1.1 2 
	-2 3

Given the input specification for a point cloud with a function, this variant should be self-explanatory.  

A finite metric space with no function is specified as in the following example::

	metric
	no function
	3
	Rips scale
	2.5
	2 3.2
	1.25

As above, this format is mostly self-explanatory, given the input specification for a metric space with a function.    However, the 3 appearing on the third line requires explanation: This is the number of points in the finite metric space.  
(This input convention is redundant: the number in the third line is always one greater than the number of entries on sixth line.  The reason for this choice of convention is that it made it simpler to write the code to parse this input, given what we already had.)


Bifiltration
------------
RIVET can accept as input any essentially finite bifiltration.  (Multicritical bifiltrations are allowed.)


Let :math:`v_1, v_2, \ldots, v_n` denote the vertices (0-simplices) of the bifiltration. 
Specifying the bifiltration requires specifying each simplex (given as a subset of :math:`v_1, v_2, \ldots, v_n`) and its birth indices. 
Simplices are specified, one simplex per line, in the bifiltration input file.

The user must ensure that the input file specifies a valid bifiltration, in the sense that a simplex is never born before its faces; RIVET does not error-check this.

A file in the bifiltration format must have the following format:

#. The first (non-empty, uncommented) line contains the word "bifiltration" and no other printed characters.
#. The second line gives a label for the first filtration parameter.
#. The third line gives a label for the second filtration parameter.
#. The remaining lines of the file each specify a simplex and its bigrades of appearance.  A line specifying a :math:`j`-simplex with :math:`n` grades of appearance must have :math:`j+1` non-negative integers (separated by white space), followed by a semicolon, followed by :math:`2n` numbers (which may be integers or decimals.  The semicolon must be surrounded by spaces.  The first :math:`j+1` integers give the vertices of the simplex. The remaining numbers specify the bigrades at which the simplex appears.

A sample multicritical bifiltration file appears below. This consists of: the boundary of a triangle born at :math:`(0,0)`; the interior of the triangle born at both :math:`(1,0)` and :math:`(0,1)`; two edges that complete the boundary of a second triangle adjacent to the first, born at :math:`(1,1)`::

	bifiltration
	time of appearance
	network distance
	0 ; 0 0
	1 ; 0 0
	2 ; 0 0
	3 ; 0 0
	0 1 ; 0 0
	0 2 ; 0 0
	1 2 ; 0 0
	0 1 2 ; 0 1 1 0
	1 3 ; 1 1
	2 3 ; 1 1

The minimal grades of appearance of a given simplex may be given in arbitrary order.  For example, it is also valid to take the seventh of the above input file to be::

	0 1 2 ; 1 0 0 1

Moreover, the code can handle non-minimial bigrades of appearance; it simply removes them.  (However, in the current code, non-minimal bigrades of appearance may change the coarsening behavior, as the :math:`x`- and :math:`y`-grades of such bigrades are currently not ignored when performing coarsening.)

One can also take the filtration direction for either of the axes to be decreasing, by placing ``[-]`` in front of an axis label. 
For instance, the following variant of the last example replaces the y-coordinate of each bigrade with its negative, and takes the filtration direction for the :math:`y`-coordinate to be descending::

	bifiltration
	time of appearance
	[-] network distance
	0 ; 0 0
	1 ; 0 0
	2 ; 0 0
	3 ; 0 0
	0 1 ; 0 0
	0 2 ; 0 0
	1 2 ; 0 0
	0 1 2 ; 0 -1 1 0
	1 3 ; 1 -1
	2 3 ; 1 -1

.. _firep:

FIRep (Algebraic Input) 
-----------------------

An FIRep 

.. math::
   :nowrap:

   \[ C_2 \xrightarrow{f} C_1 \xrightarrow{g} C_0. \]

is specified as follows:

#. The first (non-empty, uncommented) line says "firep".
#. The second line is the :math:`x`-label.
#. The third line is the :math:`y`-label.
#. The fourth line is of the form ``t s r``, where ``t``, ``s``, and ``r`` are, repsectively, the number of generators in bases for :math:`C_2`, :math:`C_1`, and :math:`C_0`.
#. Each of the next ``t`` lines specifies the bigrade of appearance of a basis element for :math:`C_2`, together with the corresponding column of the matrix representing :math:`f`: the format for such a line is: ``x y ; b1 b2 b3``, where the ``bi`` are the row indices of nonzero column entries.  (Recall that we work with :math:`\mathbb{Z}/2\mathbb{Z}` coefficients.) 
#. Each of the next ``s`` lines specifies the bigrade of appearance of a basis element for :math:`C_1`, together with the corresponding column of the matrix representing :math:`g`.
   
An example FIRep input is shown below::

	firep
	parameter 1
	parameter 2
	2 3 3 
	1 0 ; 0 1 2
	0 1 ; 0 1 2  
	0 0 ; 1 2
	0 0 ; 0 2
	0 0 ; 0 1

This example has a natural geometric interpretation.  
The boundary of a triangle is born at :math:`(0,0)`, and the triangle is filled in at both :math:`(1,0)` and :math:`(0,1)`. 
The input gives the portion of the resulting chain complex required to compute the 1st persistent homology module. 

