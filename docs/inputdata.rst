.. _inputData:

Input Data
==========

As explained in the section “:ref:`runningRIVET`” above, RIVET takes as input a *raw data* file.  This file can specify input of the following types:

* Point Cloud or finite metric space, with or without a real-valued function. 
* Bifiltration
* FIRep (i.e., short chain complex of free modules).

(These are exactly the objects in green boxes in the figure of the section “:ref:`structure`” in this documentation.)

We now specify the formats of the *raw data* file for each of these types of input.

**NOTE**: When reading text files, RIVET ignores lines that begin with the symbol `#`; such lines may be used for comments in *raw data* files.  RIVET also ignores blank lines.

Point Cloud with a Function
---------------------------

This format specifies a set of points \\(X\\) in Euclidean \\(n\\)-space, a function \\(\\gamma:X\\to \\mathbb R\\), and a maximum scale parameter \\(d\\).  Given this, RIVET builds the function-Rips bifltration \\(R(\\gamma)\\), including only simplices with diameter at most \\(d\\). 

The file has the following format:

#. The first (non-empty, uncommented) line contains the word "points" and no other characters.
#. The second line specifies the dimension \\(n\\) of Euclidean space in which the point cloud is embedded.
#. The third line specifies the maximum distance \\(d\\) of edges constructed in the Vietoris-Rips complex. This must be a positive number (integer or decimal).
#. The fourth line gives the label for the axis along which the values of \\(\\gamma\\) appear.
#. The remaining lines of the file specify the points, one point per line. Each line must specify the coordinates of a point (\\(n \\) decimal numbers specified by white space), followed by the value of \\(\\gamma\\) on the point.

Here is an example with three points in \\(\\mathbb R^2\\)::

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

This is useful, e.g.,  when taking \\(\\gamma\\) to be a density function.

Finite Metric Space with Function
---------------------------------

This format is similar to the one just described, except one specifies the entries of a symmetric distance matrix rather than the coordinates of points in \\(\\mathbb R^n\\).  
As above, RIVET constructs a function-Rips bifiltration from the input.  
The given distances are not required to satisfy the triangle inequality.

The file has the following format:

#. The first (non-empty, uncommented) line contains the word the word "metric" and no other printed characters.
#. The second line gives the label for the function \\(\\gamma\\).
#. The third line specifies \\(\\gamma\\). This line consists of a list of \\(n\\) decimal numbers, separated by white space.
#. The fourth line gives the label for the "distance" axis.
#. The fifth line specifies the maximum distance \\(d\\) of edges constructed in the Vietoris-Rips complex. This must be a positive number (integer or decimal).
#. The remaining line(s) of the file specify the distances between pairs of points. These distances appear as \\(\\frac{n(n-1)}{2}\\) numbers (integer or decimal), separated by white space or line breaks. Let the points be denoted \\(p_1, p_2, \\ldots, p_n\\). The first \\(n-1\\) numbers are the distance from \\(p_1\\) to \\(p_2, \\ldots, p_n\\). The next \\(n-2\\) numbers give the distances from \\(p_2\\) to \\(p_3, \\ldots, p_n\\), and so on. The last number gives the distance from \\(p_{n-1}\\) to \\(p_n\\).

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


Let \\(v_1, v_2, \\ldots, v_n\\) denote the vertices (0-simplices) of the bifiltration. 
Specifying the bifiltration requires specifying each simplex (given as a subset of \\(v_1, v_2, \\ldots, v_n\\)) and its birth indices. 
Simplices are specified, one simplex per line, in the bifiltration input file.

The user must ensure that the input file specifies a valid bifiltration, in the sense that a simplex is never born before its faces; RIVET does not error-check this.

A file in the bifiltration format must have the following format:

#. The first (non-empty, uncommented) line contains the word "bifiltration" and no other printed characters.
#. The second line gives a label for the first filtration parameter.
#. The third line gives a label for the second filtration parameter.
#. The remaining lines of the file each specify a simplex and its bigrades of appearance.  A line specifying a \\(j\\)-simplex with \\(n\\) grades of appearance must have \\(j+1\\) non-negative integers (separated by white space), followed by a semicolon, followed by \\(2n\\) numbers (which may be integers or decimals.  The semicolon must be surrounded by spaces.  The first \\(j+1\\) integers give the vertices of the simplex. The remaining numbers specify the bigrades at which the simplex appears.

A sample multicritical bifiltration file appears below. This consists of: the boundary of a triangle born at \\((0,0)\\); the interior of the triangle born at both \\((1,0)\\) and \\((0,1)\\); two edges that complete the boundary of a second triangle adjacent to the first, born at \\((1,1)\\)::

	bifiltration
	time of appearance
	network distance
	0 ; 0 0
	1 ; 0 0
	2 ; 0 0
	0 1 ; 0 0
	0 2 ; 0 0
	1 2 ; 0 0
	0 1 2 ; 0 1 1 0
	1 3 ; 1 1
	2 3 ; 1 1

The minimal grades of appearance of a given simplex may be given in arbitrary order.  For example, it is also valid to take the seventh of the above input file to be::

	0 1 2 ; 1 0 0 1

Moreover, the code can handle non-minimial bigrades of appearance; it simply removes them. 
 (However, in the current code, non-minimal bigrades of appearance may change the coarsening behavior, as the \\(x\\)- and \\(y\\)-grades of such bigrades are currently not ignored when performing coarsening.)

One can also take the filtration direction for either of the axes to be decreasing, by placing ``[-]`` in front of an axis label. 
For instance, the following variant of the last example replaces the y-coordinate of each bigrade with its negative, and takes the filtration direction for the \\(y\\)-coordinate to be descending::

	bifiltration
	time of appearance
	[-] network distance
	0 ; 0 0
	1 ; 0 0
	2 ; 0 0
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
\\[ C_2 \\xrightarrow{f} C_1 \\xrightarrow{g} C_0. \\]
is specified as follows:

#. The first (non-empty, uncommented) line says "firep".
#. The second line is the \\(x\\)-label.
#. The third line is the \\(y\\)-label.
#. The fourth line is of the form ``t s r``, where ``t``, ``s``, and ``r`` are, repsectively, the number of generators in bases for \\(C_2\\), \\(C_1\\), and \\(C_0\\).
#. Each of the next ``t`` lines specifies the bigrade of appearance of a basis element for \\(C_2\\), together with the corresponding column of the matrix representing \\(f\\): the format for such a line is: ``x y ; b1 b2 b3``, where the ``bi`` are the row indices of nonzero column entries.  (Recall that we work with \\(\\mathbb{Z}/2\\mathbb{Z}\\) coefficients.) 
#. Each of the next ``s`` lines specifies the bigrade of appearance of a basis element for \\(C_1\\), together with the corresponding column of the matrix representing \\(g\\).
   
An example FIRep input is shown below::

	firep
	parameter 1
	parameter 2
	2 3 3 
	1 0 ; 1 1 1 
	0 1 ; 1 1 1  
	0 0 ; 1 2
	0 0 ; 0 2
	0 0 ; 0 1

This example has a natural geometric interpretation.  
The boundary of a triangle is born at \\((0,0)\\), and the triangle is filled in at both \\((1,0)\\) and \\((0,1)\\). 
The input gives the portion of the resulting chain complex required to compute the 1st persistent homology module. 

