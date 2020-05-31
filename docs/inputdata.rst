.. _inputData:

Input Data Files
==========
As explained in :ref:`runningRIVET`, RIVET requires an *input data file*.  

Starting with version 1.1 (released in 2020), the format specification for input data files has been redesigned; this page describes the new format.  [RIVET still supports the older, less-flexible input file formats required by RIVET 1.0.  Details about these file formats can be found in ":ref:`oldInputData`".  However, the use of these input formats is discouraged, and support may be discontinued in future versions of RIVET.]

RIVET accepts six types of such files; the flag :code:`--datatype` specifies the type of the input file.  The six file types are listed below, together with the associated value of the flag :code:`--datatype` in parentheses:

* Point cloud (:code:`--datatype points`)
* Point cloud with function (:code:`--datatype points_fn`)
* Metric space (:code:`--datatype metric`)
* Metric space with function (:code:`--datatype metric_fn`)
* Bifiltration (:code:`--datatype bilfiltration`)
* FIRep, i.e., a short chain complex of free modules (:code:`--datatype firep`)

(Observe that these are exactly the objects in green boxes in the figure in the “:ref:`structure`” section of this documentation.)

The default value of the flag :code:`--datatype` is :code:`points`.  As noted in :ref:`runningRIVET`, any of the command-line flags described in :ref:`_flags` can be placed either in an input data file or given directly on the command line, and this is true in particular for the flag :code:`--datatype`.  Flags provided to **rivet_console** override those given in the input file.  [MUST EACH FLAG APPEAR ON ITS OWN LINE?].  

RIVET ignores lines in an input data file that begin with the symbol `#`; such lines may be used for comments.  Blank lines are also ignored.

We now specify the formats of the input data file for each input type, and provide type-specific details about the use of flags.

Point Cloud (Default)
---------------------------
This format specifies a set of points in Euclidean :math:`n`-space.  By default, when given input of this type, RIVET constructs the degree-Rips bifiltration.

The file has the following format:

* Flags may be specified at the top of the file. 
* Following the flags, each line contains the coordinates of exactly one point, specified as :math:`n` decimal numbers separated by white space or commas.

Flag Usage:

* :code:`--maxdist <distance>` sets the maximum scale parameter.
* :code:`--bifil function` tells RIVET to construct a function-Rips bifiltration from the Point Cloud input.
* :code:`--function <fn>` makes RIVET use the function `<fn>`.  

Here is an example specifying three points in :math:`\mathbb R^2`::
	
	#the following flag is optional, because "points" is the default value of data type
	--datatype points

        #optional flags
	--maxdist 0.5

	#data
	0,0
	1.1,2
	-2,3


Point Cloud with Function
---------------------------

This format specifies a set of points :math:`X` in Euclidean :math:`n`-space, together with a function :math:`\gamma:X\to \mathbb R`.  By default, when given input of this type, RIVET constructs the a function-Rips bifiltration using the specified function.  

The file has the following format:

* Flags may be specified at the top of the file.
* The first line following any flags lists the function values on the points, in the same order that the points appear later in the file.  The function values are specified by decimal numbers separated by white space or commas.
* Each subsequent line contains the coordinates of exactly one point, specified as :math:`n` decimal numbers separated by white space or commas.

Flag Usage:

* :code:`--maxdist <distance>` sets the maximum scale parameter.
* :code:`--bifil degree` tells RIVET to constructs a degree-Rips bifiltration rather than a function-Rips bifiltration, thereby ignoring the function values given in the file.  
* :code:`--function <fn>` tells RIVET to use the function `<fn>` to construct the function-Rips bifiltration, only if `--bifil function` has also been specified.    
* :code:`--xreverse` indicates that the function filtration direction should be descending. (This is useful, e.g.,  when taking :math:`\gamma` to be a density function.)
* When computing an MI-file, :code:`--xlabel <label>` provides a label for the function axis, for use by **rivet_GUI**.

Here is an example specifying three points in :math:`\mathbb R^2`, together with a function on these points::

	#required flag (can be given instead on the command line)
	--datatype points_fn

        #optional flags
	--xlabel birth time
	--xreverse 

        #function
        3,0.5,4

	#data
	0,0
	1.1,2
	-2,3


Metric Space
-----------------------------

This format is similar to `points`, except that one specifies the entries of a distance matrix rather than the coordinates of points in :math:`\mathbb R^n`.
If the points are denoted :math:`p_1, \ldots, p_n`, then the entry in row :math:`i`, column :math:`j` of the matrix gives the distance between :math:`p_i` to :math:`p_j`.
Thus, the matrix is symmetric, with zeros on the diagonal.
The given distances are not required to satisfy the triangle inequality.
By default, when given input of this type, RIVET constructs the degree-Rips bifiltration.

The file has the following format:

* Flags may be specified at the top of the file.
* Following the flags, each line contains the distance of the point from all other points, specified in the two possible ways described below, separated by white space or commas.

Flag Usage:

* :code:`--maxdist <distance>` sets the maximum scale parameter.
* :code:`--bifil function` tells RIVET to construct a function-Rips bifiltration from the Point Cloud input.
* :code:`--function <fn>` makes RIVET use the function `<fn>`. 
* After all flags have been specified, the distance matrix  must be given. RIVET supports two formats for specifying the distance matrix:
  + The matrix may be given as a :math:`n \times n` matrix. Each of the :math:`n` rows of the matrix must be provided as one line of the file, specified as :math:`n` decimal numbers separated by white space or commas.
  + The matrix may be given in a triangular format, specifying only the entries above the diagonal of the distance matrix. The first line of data contains :math:`n-1` numbers, which give the distances from :math:`p_1` to :math:`p_2, \ldots, p_n`. The next line of data contains :math:`n-2` numbers, which give the distances from :math:`p_2` to :math:`p_3, \ldots, p_n`, and so on. The last line of data gives only the distance from :math:`p_{n-1}` to :math:`p_n`.

Here is an example, for a metric space of cardinality 3::

	#required flag:
	--datatype metric

        #optional flags:
	--xlabel birth time
	--ylabel geodesic distance

	# distance matrix (symmetric matrix, with zeros on the diagonal)
	0,2,3.2
	2,0,1.25
	3.2,1.25,0

The same distance data can be given in the following upper triangular format:

	# upper triangular distance matrix
	2,3.2
	1.25


Metric Space with Function
-----------------------------

This format is similar to the one just described above, except that this file contains function values associated with the points in the matrix.
By default, when given input of this type, RIVET constructs the function-Rips bifiltration.

The file has the following format:

* Flags may be specified at the top of the file. 
* The first line following any flags lists the function values on the points, in the same order that the points appear later in the file.  The function values are specified by decimal numbers separated by white space or commas.
* Each subsequent line contains the distance of the point from all other points, specified in the two possible ways described below, separated by white space or commas.

Flag Usage:

* :code:`--maxdist <distance>` sets the maximum scale parameter.
* :code:`--bifil degree` tells RIVET to constructs a degree-Rips bifiltration rather than a function-Rips bifiltration, thereby ignoring the function values given in the file.  
* :code:`--function <fn>` tells RIVET to use the function `<fn>` to construct the function-Rips bifiltration, only if `--bifil function` has also been specified.
* After all flags have been specified, the distance matrix  must be given. RIVET supports two formats for specifying the distance matrix:
  + The matrix may be given as a :math:`n \times n` matrix. Each of the :math:`n` rows of the matrix must be provided as one line of the file, specified as :math:`n` decimal numbers separated by white space or commas.
  + The matrix may be given in a triangular format, specifying only the entries above the diagonal of the distance matrix. The first line of data contains :math:`n-1` numbers, which give the distances from :math:`p_1` to :math:`p_2, \ldots, p_n`. The next line of data contains :math:`n-2` numbers, which give the distances from :math:`p_2` to :math:`p_3, \ldots, p_n`, and so on. The last line of data gives only the distance from :math:`p_{n-1}` to :math:`p_n`.

Here is an example, for a metric space of cardinality 3::

	#required flag:
	--datatype metric_fn

        #optional flags:
	--xlabel birth time
	--ylabel geodesic distance

	#function values
	1,1.1,-2
	# distance matrix (symmetric matrix, with zeros on the diagonal)
	0,2,3.2
	2,0,1.25
	3.2,1.25,0

The same distance data can be given in the following upper triangular format:

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

