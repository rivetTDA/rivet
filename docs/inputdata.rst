.. _inputData:

Input Data Files
================

As explained in :ref:`rivetconsole`, RIVET requires an *input data file*.  

Starting with version 1.1 (released in 2020), the format for input data files has been redesigned to be more flexible; this page describes the new format.  [RIVET still supports the older, less-flexible input file formats required by RIVET 1.0; details about these can be found in ":ref:`oldInputData`".  However, the use of the old input formats is discouraged, and support may be discontinued in future versions of RIVET.]

RIVET accepts six types of input data files; the flag :code:`--datatype` tells RIVET which file type to expect.  The six file types are listed below, together with the associated value of the flag :code:`--datatype` in parentheses:

* Point cloud (:code:`--datatype points`)
* Point cloud with function (:code:`--datatype points_fn`)
* Metric space (:code:`--datatype metric`)
* Metric space with function (:code:`--datatype metric_fn`)
* Bifiltration (:code:`--datatype bilfiltration`)
* FIRep, i.e., a short chain complex of free modules (:code:`--datatype firep`)

(Observe that these are exactly the objects in green boxes in the figure in the “:ref:`structure`” section of this documentation.)

If the flag :code:`--datatype` is not given, RIVET uses the default value of :code:`points`.  As noted in :ref:`rivetconsole`, any of the command-line flags described in :ref:`flags` can be placed either in an input data file or given directly on the command line, and this is true in particular for the flag :code:`--datatype`.

In general, flags in the input data file must be provided in the top lines of the file, one flag per line, before the data is given. As noted in :ref:`flags`, flags provided to **rivet_console** override those given in the input file. 

RIVET ignores lines in an input data file that begin with the symbol `#`; such lines may be used for comments.  Blank lines are also ignored.

We next specify the file format for each input type, and provide type-specific details about how flags are used.

Point Cloud (Default)
---------------------------
This format specifies a set of points in Euclidean :math:`n`-space.  By default, when given input of this type, RIVET constructs the degree-Rips bifiltration.  

The file has the following format:
 
* Following any flags, each line contains the coordinates of exactly one point, specified as :math:`n` decimal numbers separated by white space or commas.

Flag usage:

* :code:`--maxdist <distance>` sets the maximum scale parameter.
* :code:`--bifil function` tells RIVET to construct a function-Rips bifiltration. Alternately, :code:`--bifil degree` tells RIVET to construct a degree-Rips bifiltration.
* :code:`--function <fn>` tells RIVET to construct a function-Rips bifiltration using the function :code:`<fn>`.  See :ref:`flags` for details.

Here is an example specifying three points in :math:`\mathbb R^2`::
	
	#the following flag is optional, because "points" is the default value of datatype
	#however, it is not a bad idea to include the flag for readability
	--datatype points

        #optional flags
	--maxdist 0.5

	#data
	0,0
	1.1,2
	-2,3

..
    [TODO: ADD AN EXAMPLE USING A BUILT-IN RIVET FUNCTION?]

Point Cloud with Function
---------------------------

This format specifies a set of points :math:`X=\{x_1,\ldots,x_k\}` in Euclidean :math:`n`-space, together with a function :math:`\gamma:X\to \mathbb R`.  By default, when given input of this type, RIVET constructs the a function-Rips bifiltration using the given function.  

The file has the following format:

* The first line following any flags lists the function values :math:`\gamma(x_1),\gamma(x_2),\ldots \gamma(x_k)`.  The function values are specified by decimal numbers separated by white space or commas.
* Each subsequent line contains the coordinates of exactly one point, specified as :math:`n` decimal numbers separated by white space or commas.

Flag usage:

* :code:`--datatype points_fn` must be provided.
* :code:`--maxdist <distance>` sets the maximum scale parameter.
* :code:`--bifil function` tells RIVET to construct a function-Rips bifiltration. Alternately, :code:`--bifil degree` tells RIVET to construct a degree-Rips bifiltration.
* :code:`--function <fn>` tells RIVET to construct a function-Rips bifiltration using the function :code:`<fn>`.  See :ref:`flags` for details. Choosing a function other than :code:`user` will cause RIVET to ignore the function values given in the input file.
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

This format specifies a symmetric :math:`n\times n` matrix, with zeros on the diagonal, which we think of as representing a (semi-pseudo-)metric on a finite set :math:`\{p_1, \ldots, p_n\}`: the entry in row :math:`i`, column :math:`j` of the matrix gives the distance between :math:`p_i` and :math:`p_j`.  The given distances are not required to satisfy the triangle inequality, and off-diagonal entries may be zero.

By default, when given input of this type, RIVET constructs the degree-Rips bifiltration.  

The file has the following format:

* Following the flags, the distance matrix is given in either of two formats; RIVET automatically detects the format.
  + Format 1: The full matrix is explicitly provided, one row per line. Each row is specified as a list of decimal numbers separated by white space or commas.
  + Format 2: The matrix is given in triangular format, specifying only the entries above the diagonal of the distance matrix. The first line of data contains :math:`n-1` numbers, which give the distances from :math:`p_1` to :math:`p_2, \ldots, p_n`. The next line contains :math:`n-2` numbers, which give the distances from :math:`p_2` to :math:`p_3, \ldots, p_n`, and so on. The last line of data gives only the distance from :math:`p_{n-1}` to :math:`p_n`.


Flag Usage:

* :code:`--datatype metric` must be provided.
* :code:`--maxdist <distance>` sets the maximum scale parameter.
* :code:`--bifil function` tells RIVET to construct a function-Rips bifiltration. Alternately, :code:`--bifil degree` tells RIVET to construct a degree-Rips bifiltration.
* :code:`--function <fn>` tells RIVET to construct a function-Rips bifiltration using the function :code:`<fn>`.  See :ref:`flags` for details.


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

	# distance matrix (upper triangular format)
	2,3.2
	1.25


Metric Space with Function
-----------------------------

This format is similar to the one just described above, except that this file contains function values associated with the points in the matrix.
By default, when given input of this type, RIVET constructs the function-Rips bifiltration.

The file has the following format:

* The first line following any flags lists the function values on the points, in the same order that the points appear later in the file.  The function values are specified by decimal numbers separated by white space or commas.
* The remaining lines specify the distance matrix, in either of the two formats specified above for the Metric data type.

Flag Usage:

* :code:`--datatype metric_fn` must be provided.
* :code:`--maxdist <distance>` sets the maximum scale parameter.
* :code:`--bifil function` tells RIVET to construct a function-Rips bifiltration. Alternately, :code:`--bifil degree` tells RIVET to construct a degree-Rips bifiltration.
* :code:`--function <fn>` tells RIVET to construct a function-Rips bifiltration using the function :code:`<fn>`. See :ref:`flags` for details. Choosing a function other than :code:`user` will cause RIVET to ignore the function values given in the input file.

Here is an example, for a metric space of cardinality 3::

	#required flag:
	--datatype metric_fn

        #optional flags:
	--xlabel birth time
	--ylabel geodesic distance

	#function values
	1,1.1,-2
	# distance matrix, given in upper triangular format
	2,3.2
	1.25


Bifiltration
------------

RIVET can accept as input any essentially finite bifiltration.  (Multicritical bifiltrations are allowed.)

Let :math:`v_1, v_2, \ldots, v_n` denote the vertices (0-simplices) of the bifiltration. 
Specifying the bifiltration requires specifying each simplex (given as a subset of :math:`v_1, v_2, \ldots, v_n`) and its birth indices. 
Simplices are specified, one simplex per line, in the bifiltration input file.

The file has the following format:

* After all flags are specified, each remaining line of the file specifies a simplex and its bigrades of appearance.  A line specifying a :math:`j`-simplex with :math:`n` grades of appearance must have :math:`j+1` non-negative integers (separated by white space), followed by a semicolon, followed by :math:`2n` numbers (which may be integers or decimals.  The semicolon must be surrounded by spaces.  The first :math:`j+1` integers give the vertices of the simplex. The remaining numbers specify the bigrades at which the simplex appears.

The user must ensure that the input file specifies a valid bifiltration, in the sense that a simplex is never born before its faces; RIVET does not error-check this.

Flag Usage:

* :code:`--datatype bifiltration` must be provided.
* The flags :code:`--xreverse` and :code:`--yreverse` specify that the filtration is to be constructed with respect to descending x-coordinates or y-coordinates.  These flags cannot be used (or omitted) freely; the coordinate directions specified must be compatible with given bigrades of simplices, so that no simplex before one of its faces.  The code does not detect the correct flags  automatically, and the user is responsible for supplying them.

An example appears below. This consists of: the boundary of a triangle born at :math:`(0,0)`; the interior of the triangle born at both :math:`(1,0)` and :math:`(0,1)`; two edges that complete the boundary of a second triangle adjacent to the first, born at :math:`(1,1)`::

	--datatype bifiltration
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

is specified in the following format:

* Following any flags, the first line must be of the form ``t s r``, where ``t``, ``s``, and ``r`` are, repsectively, the ranks of :math:`C_2`, :math:`C_1`, and :math:`C_0`.
* Each of the next ``t`` lines specifies the bigrade of appearance of a basis element for :math:`C_2`, together with the corresponding column of the matrix representing :math:`f`.  The format for such a line is (e.g. if the column has three non-zero entries): ``x y ; b1 b2 b3``, where (x,y) is the bigrade and the ``bi`` are the row indices of nonzero column entries.  (Recall that we work with :math:`\mathbb{Z}/2\mathbb{Z}` coefficients.) 
* Each of the next ``s`` lines specifies the bigrade of appearance of a basis element for :math:`C_1`, together with the corresponding column of the matrix representing :math:`g`.
 
As with the Bifiltration input format, the user must ensure that the input file specifies a valid FIRep.  [Does this need to be capitalized?]

Flag Usage:

* :code:`--datatype firep` must be provided.
* The flags :code:`--xreverse` and :code:`--yreverse` specify that the filtration is to be constructed with respect to descending x-coordinates or y-coordinates.  The flags behave for FIRep input in essentially the same way as for bifiltration input, and the user must be sure to supply flags in a way that is compatible with the bigrades of the input.

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

This example has a natural geometric interpretation: The boundary of a triangle is born at :math:`(0,0)`, and the triangle is filled in at both :math:`(1,0)` and :math:`(0,1)`.  The input gives the portion of the resulting chain complex required to compute the 1st persistent homology module. 

