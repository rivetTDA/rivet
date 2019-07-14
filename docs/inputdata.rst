.. _inputData:

Input Data
==========

As explained in the section “:ref:`runningRIVET`” above, RIVET requires an *input data file*.  This file can specify input of the following types:

* Point Cloud 
* Finite Metric Space 
* Bifiltration
* FIRep (i.e., short chain complex of free modules).

(The Point Cloud and Finite Metric Space may have function values associated with them. Observe that these are exactly the objects in green boxes in the figure of the section “:ref:`structure`” in this documentation.)

We now specify the formats of the input data file for each of these types of input. This is the new format that RIVET supports. If you have used the old format, then you might notice two things:

  #. You can supply any number of options in the form of command-line flags.
  #. RIVET now supports csv data.

The old input formats are supported as well and you can get more details about that in ":ref:`oldInputData`".

**NOTE**: RIVET ignores lines that begin with the symbol `#`; such lines may be used for comments.  Blank lines are also ignored.

Point Cloud (Default)
---------------------------

This format specifies a set of points :math:`X` in Euclidean :math:`n`-space.
A function :math:`\gamma:X\to \mathbb R` can be specified along with it to construct a function-Rips bifiltration.
If no function is specified, RIVET constructs the degree-Rips bifilitration.
A optional maximum scale parameter :math:`d` can also be specified. Given this, RIVET only includes simplices with diameter at most :math:`d`. If :math:`d` is not specified, all possible diameters are considered.

The file has the following format:

#. The first few lines of the file should have flags you want to specify. For a full list of flags, run :code:`rivet_console (-h | --help)`. Note that none of the flags is a requirement.
#. In order to build a function-Rips, the :code:`--function` flag should be present. If supplied, the line immediately following the :code:`--function` flag should contain the function values of the given points in order, separated by white space or commas.
#. If you want to provide a label for the function, use the :code:`--x-label` flag.
#. To indicate that the filtration direction on vertices should be in descending order, simply add the :code:`--x-reverse` flag. (This is useful, e.g.,  when taking :math:`\gamma` to be a density function.)
#. After all flags have been specified (including :code:`--function`), the data for the points need to be specified. Each line should contain the coordinates of exactly one point specified as :math:`n` decimal numbers separated by white space or commas.

Here is an example with three points in :math:`\mathbb R^2` with function values::

	--type points
	--max-dist 3.2
	--function
	3,0.5,4
	--x-label birth time
	--x-reverse
	0,0
	1.1,2
	-2,3



Finite Metric Space
---------------------------------

This format is similar to the one just described, except one specifies the entries of a symmetric distance matrix rather than the coordinates of points in :math:`\mathbb R^n`.  
As above, if function values are provided using the :code:`--function` flag, then RIVET constructs a function-Rips bifiltration from the input. Otherwise, it constructs a degree-Rips bifiltration.
The given distances are not required to satisfy the triangle inequality.
As with the point cloud data, a maximum distance :math:`d` could be specified with the :code:`--max-dist` flag.
This would only consider entries in the matrix with value less than :math:`d`.

The file has the following format:

#. The first few lines of the file should have the flags you want to specify. For a full list of flags, run :code:`rivet_console (-h | --help)`. Note that since this is not the default, the :code:`--type` flag must be specified with the argument :code:`metric` for the data to be interpreted as a metric space.
#. In order to build a function-Rips, the :code:`--function` flag should be present. If supplied, the line immediately following the :code:`--function` flag should contain the function values of the given points in order, separated by white space or commas.
#. If you want to provide a label for the function, use the :code:`--x-label` flag.
#. After all flags have been specified (including :code:`--function`), the data for the points need to be specified. Each line should contain the distance of that point from all the points in the set (including itself), separated by white space or commas.

Here is an example, for a metric space of cardinality 3::

	--type metric
	--x-label birth time
	--function
	1,1.1,-2
	--y-label geodesic distance
	--max-dist 2.5
	0,2,3.2
	2,0,1.25
	3.2,1.25,0

As above, we can reverse the filtration direction on vertices by using the :code:`--x-reverse` or :code:`--y-reverse` somewhere in the flag section of the file.


Bifiltration
------------

RIVET can accept as input any essentially finite bifiltration.  (Multicritical bifiltrations are allowed.)


Let :math:`v_1, v_2, \ldots, v_n` denote the vertices (0-simplices) of the bifiltration. 
Specifying the bifiltration requires specifying each simplex (given as a subset of :math:`v_1, v_2, \ldots, v_n`) and its birth indices. 
Simplices are specified, one simplex per line, in the bifiltration input file.

The user must ensure that the input file specifies a valid bifiltration, in the sense that a simplex is never born before its faces; RIVET does not error-check this.

A file in the bifiltration format must have the following format:

#. The first few lines of the file should have the flags you want to specify. For a full list of flags, run :code:`rivet_console (-h | --help)`. Note that since this is not the default, the :code:`--type` flag must be specified with the argument :code:`bifiltration` for the data to be interpreted as a bifiltration.
#. Other flags that are usually supplied with a bifiltration file are :code:`--x-label`, :code:`--y-label`, :code:`--x-reverse` and :code:`--y-reverse`.
#. The remaining lines of the file each specify a simplex and its bigrades of appearance.  A line specifying a :math:`j`-simplex with :math:`n` grades of appearance must have :math:`j+1` non-negative integers (separated by white space), followed by a semicolon, followed by :math:`2n` numbers (which may be integers or decimals.  The semicolon must be surrounded by spaces.  The first :math:`j+1` integers give the vertices of the simplex. The remaining numbers specify the bigrades at which the simplex appears.

A sample multicritical bifiltration file appears below. This consists of: the boundary of a triangle born at :math:`(0,0)`; the interior of the triangle born at both :math:`(1,0)` and :math:`(0,1)`; two edges that complete the boundary of a second triangle adjacent to the first, born at :math:`(1,1)`::

	--type bifiltration
	--x-label time of appearance
	--y-label network distance
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

One can also take the filtration direction for either of the axes to be decreasing, by using the :code:`--x-reverse` or :code:`--y-reverse` flags.

.. _firep:

FIRep (Algebraic Input) 
-----------------------

An FIRep 

.. math::
   :nowrap:

   \[ C_2 \xrightarrow{f} C_1 \xrightarrow{g} C_0. \]

is specified as follows:

#. The first few lines of the file should have the flags you want to specify. For a full list of flags, run :code:`rivet_console (-h | --help)`. Note that since this is not the default, the :code:`--type` flag must be specified with the argument :code:`firep` for the data to be interpreted as a firep.
#. Other flags that are usually supplied with a firep file are :code:`--x-label`, :code:`--y-label`, :code:`--x-reverse` and :code:`--y-reverse`.
#. After all flags have been specified, the data must be provided. The first line of this data is of the form ``t s r``, where ``t``, ``s``, and ``r`` are, repsectively, the number of generators in bases for :math:`C_2`, :math:`C_1`, and :math:`C_0`.
#. Each of the next ``t`` lines specifies the bigrade of appearance of a basis element for :math:`C_2`, together with the corresponding column of the matrix representing :math:`f`: the format for such a line is: ``x y ; b1 b2 b3``, where the ``bi`` are the row indices of nonzero column entries.  (Recall that we work with :math:`\mathbb{Z}/2\mathbb{Z}` coefficients.) 
#. Each of the next ``s`` lines specifies the bigrade of appearance of a basis element for :math:`C_1`, together with the corresponding column of the matrix representing :math:`g`.
   
An example FIRep input is shown below::

	--type firep
	--x-label parameter 1
	--y-label parameter 2
	2 3 3 
	1 0 ; 0 1 2
	0 1 ; 0 1 2  
	0 0 ; 1 2
	0 0 ; 0 2
	0 0 ; 0 1

This example has a natural geometric interpretation.  
The boundary of a triangle is born at :math:`(0,0)`, and the triangle is filled in at both :math:`(1,0)` and :math:`(0,1)`. 
The input gives the portion of the resulting chain complex required to compute the 1st persistent homology module. 

