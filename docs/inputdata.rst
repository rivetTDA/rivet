.. _inputData:

Input Data
==========

As explained in the section “:ref:`runningRIVET`” above, RIVET requires an *input data file*.  This file can specify input of the following types:

* Point cloud (with or without a real-valued function)
* Finite metric space (with or without a real-valued function)
* Bifiltration
* FIRep (i.e., a short chain complex of free modules).

(Observe that these are exactly the objects in green boxes in the figure in the “:ref:`structure`” section of this documentation.)

Starting with version 1.1 (released July 2019), RIVET allows the specification of input parameters at the top of input files using a syntax that matches the command-line flags of :code:`rivet_console`.
For example, an input file may contain flags specifying type of input, the number of bins, axis labels, and more. For a full list of flags, run :code:`rivet_console (-h | --help)`.

Note that flags provided to :code:`rivet_console` override those given in the input file.
If no flags are provided, then RIVET assumes the input file contains point-cloud data.

RIVET still supports the older, less-flexible input file formats that were required by RIVET 1.0.
Details about these file formats can be found in ":ref:`oldInputData`".
However, the use of these input formats is discouraged, and support may be discontinued in future versions of RIVET.

Finally, note that RIVET ignores lines that begin with the symbol `#`; such lines may be used for comments.  Blank lines are also ignored.

We now specify the formats of the input data file for each of these types of input.


Point Cloud (Default)
---------------------------

This format specifies a set of points :math:`X` in Euclidean :math:`n`-space.
Optionally, a function :math:`\gamma:X\to \mathbb R` can be specified, to construct a function-Rips bifiltration.
If no function is specified, RIVET constructs the degree-Rips bifilitration.
A optional maximum scale parameter :math:`d` may be specified, causing RIVET to only include simplices with diameter at most :math:`d`. If :math:`d` is not specified, then all possible simplices are considered.

The file has the following format:

* Optional flags may be specified at the top of the file. For a full list of supported flags, run :code:`rivet_console (-h | --help)`. Note that this file type does not require any flags.
* In order to build a function-Rips bifiltration, the :code:`--function` flag should be present. If supplied, the line immediately following the :code:`--function` flag must contain the function values on the given points, in the same order that the points appear later in the file. Numbers may be separated by white space or commas.
* To provide a label for the function axis, use the :code:`--xlabel` flag.
* To indicate that the function filtration direction should be descending, simply add the :code:`--xreverse` flag. (This is useful, e.g.,  when taking :math:`\gamma` to be a density function.)
* After all flags have been specified (including :code:`--function` and its values), the coordinates of each point should be given. Each line should contain the coordinates of exactly one point, specified as :math:`n` decimal numbers separated by white space or commas.

Here is an example with three points in :math:`\mathbb R^2` with function values::

	--type points
	--max-dist 3.2
	--function
	3,0.5,4
	--xlabel birth time
	--xreverse

	#data
	0,0
	1.1,2
	-2,3

In the example above, note that :code:`#data` is a comment. This line is ignored by RIVET, but improves the human-readability of the file.


Finite Metric Space
---------------------------------

This format is similar to the one just described, except that one specifies the entries of a distance matrix rather than the coordinates of points in :math:`\mathbb R^n`.
If the points are denoted :math:`p_1, \ldots, p_n`, then the entry in row :math:`i`, column :math:`j` of the matrix gives the distance between :math:`p_i` to :math:`p_j`.
Thus, the matrix is symmetric, with zeros on the diagonal.
Since it is symmetric, one may also specify the upper triangular matrix without the zeros instead of the whole matrix.
The given distances are not required to satisfy the triangle inequality.

As with point cloud data, if function values are provided using the :code:`--function` flag, then RIVET constructs a function-Rips bifiltration from the input. Otherwise, RIVET constructs a degree-Rips bifiltration.
A maximum distance :math:`d` may be specified with the :code:`--max-dist` flag, which causes RIVET to only consider entries in the matrix with value less than :math:`d`.

The file has the following format:

* Flags may be specified at the top of the file. Note that the :code:`--type` flag must be given with the argument :code:`metric`. For a full list of possible flags, run :code:`rivet_console (-h | --help)`. 
* In order to build a function-Rips bifiltration, the :code:`--function` flag must be present. If supplied, the line immediately following the :code:`--function` flag must contain the function values on the given points, in the same order that the points appear later in the file. Numbers may be separated by white space or commas.
* To provide a label for the function axis, use the :code:`--xlabel` flag.
* After all flags have been specified (including :code:`--function` and its values), the distance matrix (or an upper triangular matrix without zeros) must be given. Each row of the matrix should be provided in one line of the file, specified as :math:`n` decimal numbers separated by white space or commas.

Here is an example, for a metric space of cardinality 3::

	--type metric
	--xlabel birth time
	--function
	1,1.1,-2
	--ylabel geodesic distance
	--max-dist 2.5

	# distance matrix
	0,2,3.2
	2,0,1.25
	3.2,1.25,0

The same data can be written as the following upper triangular matrix::

	# upper triangular distance matrix
	2,3.2
	1.25


Bifiltration
------------

RIVET can accept as input any essentially finite bifiltration.  (Multicritical bifiltrations are allowed.)

Let :math:`v_1, v_2, \ldots, v_n` denote the vertices (0-simplices) of the bifiltration. 
Specifying the bifiltration requires specifying each simplex (given as a subset of :math:`v_1, v_2, \ldots, v_n`) and its birth indices. 
Simplices are specified, one simplex per line, in the bifiltration input file.

The user must ensure that the input file specifies a valid bifiltration, in the sense that a simplex is never born before its faces; RIVET does not error-check this.

A file in the bifiltration format must have the following format:

* Flags may be specified at the top of the file. Note that the :code:`--type` flag must be given with the argument :code:`bifiltration`. For a full list of possible flags, run :code:`rivet_console (-h | --help)`. 
* The :code:`--xlabel` and :code:`--ylabel` flags are often specified for bifiltration input. To reverse either (or both) axis directions, provide the flags :code:`--xreverse` or :code:`--yreverse`.
* After all flags are specified, the remaining lines of the file each specify a simplex and its bigrades of appearance.  A line specifying a :math:`j`-simplex with :math:`n` grades of appearance must have :math:`j+1` non-negative integers (separated by white space), followed by a semicolon, followed by :math:`2n` numbers (which may be integers or decimals.  The semicolon must be surrounded by spaces.  The first :math:`j+1` integers give the vertices of the simplex. The remaining numbers specify the bigrades at which the simplex appears.

A sample multicritical bifiltration file appears below. This consists of: the boundary of a triangle born at :math:`(0,0)`; the interior of the triangle born at both :math:`(1,0)` and :math:`(0,1)`; two edges that complete the boundary of a second triangle adjacent to the first, born at :math:`(1,1)`::

	--type bifiltration
	--xlabel time of appearance
	--ylabel network distance

	#data
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

The minimal grades of appearance of a given simplex may be given in arbitrary order.  For example, the line specifying a 2-simplex in the sample above may be equivalently written as:

	0 1 2 ; 1 0 0 1

Moreover, the code can handle non-minimial bigrades of appearance; it simply removes them.  (However, in the current code, non-minimal bigrades of appearance may change the coarsening behavior, as the :math:`x`- and :math:`y`-grades of such bigrades are currently not ignored when performing coarsening.)

One can also take the filtration direction for either of the axes to be decreasing, by using the :code:`--xreverse` or :code:`--yreverse` flags.

.. _firep:


FIRep (Algebraic Input) 
-----------------------

An FIRep 

.. math::
   :nowrap:

   \[ C_2 \xrightarrow{f} C_1 \xrightarrow{g} C_0. \]

is specified as follows:

* Flags may be specified at the top of the file. Note that the :code:`--type` flag must be given with the argument :code:`firep`. For a full list of possible flags, run :code:`rivet_console (-h | --help)`. 
* The :code:`--xlabel` and :code:`--ylabel` flags are often specified for bifiltration input. To reverse either (or both) axis directions, provide the flags :code:`--xreverse` or :code:`--yreverse`.
* After all flags are specified, the remaining lines of the file give the FIRep data. The first line of this data must be of the form ``t s r``, where ``t``, ``s``, and ``r`` are, repsectively, the number of generators in bases for :math:`C_2`, :math:`C_1`, and :math:`C_0`.
* Each of the next ``t`` lines specifies the bigrade of appearance of a basis element for :math:`C_2`, together with the corresponding column of the matrix representing :math:`f`: the format for such a line is: ``x y ; b1 b2 b3``, where the ``bi`` are the row indices of nonzero column entries.  (Recall that we work with :math:`\mathbb{Z}/2\mathbb{Z}` coefficients.) 
* Each of the next ``s`` lines specifies the bigrade of appearance of a basis element for :math:`C_1`, together with the corresponding column of the matrix representing :math:`g`.
   
An example FIRep input is shown below::

	--type firep
	--xlabel parameter 1
	--ylabel parameter 2

	# data
	2 3 3 
	1 0 ; 0 1 2
	0 1 ; 0 1 2  
	0 0 ; 1 2
	0 0 ; 0 2
	0 0 ; 0 1

This example has a natural geometric interpretation.  
The boundary of a triangle is born at :math:`(0,0)`, and the triangle is filled in at both :math:`(1,0)` and :math:`(0,1)`. 
The input gives the portion of the resulting chain complex required to compute the 1st persistent homology module. 

