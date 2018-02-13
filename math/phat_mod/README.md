In order to take advantage of PHAT's lazy heap 
representation of column-sparse matrices, RIVET uses three 
files from the PHAT software package for persistent homology computation:
vector_heap.h (this is heavily modified/extended and is now called vector_heap_mod.h.)
misc.h
thead_local_storage.h

## Original README for PHAT ##
-------------------------
# PHAT (Persistent Homology Algorithm Toolbox), v1.5 #

Copyright 2013-2017 IST Austria

## Project Founders ##

Ulrich Bauer, Michael Kerber, Jan Reininghaus

## Contributors ##

Hubert Wagner, Bryn Keller

## Downloads ##
* [PHAT, v1.5](https://bitbucket.org/phat-code/phat/get/v1.5.zip)
* [PHAT, v1.4.1](https://bitbucket.org/phat-code/phat/get/v1.4.1.zip)
* [PHAT, v1.3](https://drive.google.com/uc?id=0B7Yz6TPEpiGEMGFNQ3FPX3ltelk&export=download)
* [PHAT, v1.2.1](https://drive.google.com/uc?id=0B7Yz6TPEpiGENE9KUnhUSFdFQUk&export=download)
* [benchmark data](https://drive.google.com/uc?id=0B7Yz6TPEpiGERGZFbjlXaUt1ZWM&export=download)
* [benchmark data 2](https://drive.google.com/uc?id=0B7Yz6TPEpiGEWE55X3RuM3JjZ3M&export=download)

## Description ##

This software library contains methods for computing the persistence pairs of a 
filtered cell complex represented by an ordered boundary matrix with Z<sub>2</sub> coefficients. 
For an introduction to persistent homology, see the textbook `[1]`. This software package
contains code for several algorithmic variants:

  * The "standard" algorithm (see `[1]`, p.153)
  * The "row" algorithm from `[2]` (called pHrow in that paper)
  * The "twist" algorithm, as described in `[3]` (default algorithm)
  * The "chunk" algorithm presented in `[4]` 
  * The "spectral sequence" algorithm (see `[1]`, p.166)

All but the standard algorithm exploit the special structure of the boundary matrix
to take shortcuts in the computation. The chunk and the spectral sequence algorithms
make use of multiple CPU cores if PHAT is compiled with OpenMP support.

All algorithms are implemented as function objects that manipulate a given 
`boundary_matrix` (to be defined below) object to reduced form. 
From this reduced form one can then easily extract the persistence pairs. 
Alternatively, one can use the `compute_persistence_pairs function` which takes an 
algorithm as a template parameter, reduces the given `boundary_matrix` and stores the 
resulting pairs in a given `persistence_pairs` object.

The `boundary_matrix` class takes a "Representation" class as template parameter. 
This representation defines how columns of the matrix are represented and how 
low-level operations (e.g., column additions) are performed. The right choice of the 
representation class can be as important for the performance of the program as choosing
the algorithm. We provide the following choices of representation classes:

  * `vector_vector`: Each column is represented as a sorted `std::vector` of integers, containing the indices of the non-zero entries of the column. The matrix itself is a `std::vector` of such columns.
  * `vector_heap`: Each column is represented as a heapified `std::vector` of integers, containing the indices of the non-zero entries of the column. The matrix itself is a `std::vector` of such columns.
  * `vector_set`: Each column is a `std::set` of integers, with the same meaning as above. The matrix is stored as a `std::vector` of such columns.
  * `vector_list`: Each column is a sorted `std::list` of integers, with the same meaning as above. The matrix is stored as a `std::vector` of such columns.
  * `sparse_pivot_column`: The matrix is stored as in the vector_vector representation. However, when a column is manipulated, it is first  converted into a `std::set`, using an extra data field called the "pivot column".  When another column is manipulated later, the pivot column is converted back to  the `std::vector` representation. This can lead to significant speed improvements when many columns  are added to a given pivot column consecutively. In a multicore setup, there is one pivot column per thread.
  * `heap_pivot_column`: The same idea as in the sparse version. Instead of a `std::set`, the pivot column is represented by a `std::priority_queue`. 
  * `full_pivot_column`: The same idea as in the sparse version. However, instead of a `std::set`, the pivot column is expanded into a bit vector of size n (the dimension of the matrix). To avoid costly initializations, the class remembers which entries have been manipulated for a pivot column and updates only those entries when another column becomes the pivot.
  * `bit_tree_pivot_column` (default representation): Similar to the `full_pivot_column` but the implementation is more efficient. Internally it is a bit-set with fast iteration over nonzero elements, and fast access to the maximal element. 
  
There are two ways to interface with the library:

* using files: 
    * write the boundary matrix / filtration into a file "input" (see below for the file format). 
    * compile `src/phat.cpp` and run it:
    `
    phat [--ascii] input output
    ` 
    * read the resulting persistence pairs into your program 
* using the C++ library interface:
    * include all headers found in `src/phat.cpp`
    * define a boundary matrix object, e.g. 
`
phat::boundary_matrix< bit_tree_pivot_column > boundary_matrix;
`
    * set the number of columns:
`
boundary_matrix.set_num_cols(...);
`
    * initialize each column using 
`
boundary_matrix.set_col(...)
boundary_matrix.set_dim(...)
`
    * define an object to hold the result:
`
phat::persistence_pairs pairs;
`
    * run an algorithm like this:
`
phat::compute_persistence_pairs< phat::twist_reduction >( pairs, boundary_matrix );
`
    * examine the result: 
`
pairs.get_num_pairs()
pairs.get_pair(...)
` 
 	
A simple example that demonstrates this functionality can be found in `src/simple_example.cpp`

## File Formats ##

The library supports input and output in ascii and binary format
through the methods `[load|save]_[ascii|binary]` in the classes `boundary_matrix` 
and `persistence_pairs`. The file formats are defined as follows:

* `boundary_matrix` - ascii:
	The file represents the filtration of the cell complex, containing one cell 
	per line (empty lines and lines starting with "#" are ignored). A cell is given by 
	a sequence of integers, separated by spaces, where the first integer denotes the
	dimension of the cell, and all following integers give the indices
	of the cells that form its boundary (the index of a cell is its position 
	in the filtration, starting with 0). 
	A sample file `single_triangle.dat` can be found in the examples folder.

* `boundary_matrix` - binary:
	In binary format, the file is simply interpreted as a sequence of 64 bit signed integer 
	numbers. The first number is interpreted as the number of cells of the complex. The 	
	descriptions of the cells is expected to follow, with the first number representing the 
	dimension of the cell, the next number, say N, representing the size of the boundary, 
	followed by N numbers denoting the indices of the boundary cells. 
	A sample file `single_triangle.bin` can be found in the examples folder.

* `persistence_pairs` - ascii: 
	The file contains the persistence pairs, sorted by birth index. The first integer in the
	file is equal to the number of pairs. It is followed by pairs of integers encode the 
	respective birth and death indices. 
	A sample file `single_triangle_persistence_pairs.dat` can be found in the examples folder.

* `persistence_pairs` - binary: 
	Same as ascii format, see above. Only now the integers are encoded as 64bit signed integers.
	A sample file `single_triangle_persistence_pairs.bin` can be found in the examples folder.

Supported Platforms:

* Visual Studio 2008 and 2012 (2010 untested)
* GCC version 4.4. and higher

## Python Bindings ##
We provide bindings for Python 3.x and 2.7.12+, which are installable using `pip`. Please see
the Python-specific README.rst in the `python` folder of this repository for details.

## References ##

1. H.Edelsbrunner, J.Harer: Computational Topology, An Introduction. American Mathematical Society, 2010, ISBN 0-8218-4925-5
2. V.de Silva, D.Morozov, M.Vejdemo-Johansson: Dualities in persistent (co)homology. Inverse Problems 27, 2011
3. C.Chen, M.Kerber: Persistent Homology Computation With a Twist. 27th European Workshop on Computational Geometry, 2011.
4. U.Bauer, M.Kerber, J.Reininghaus: Clear and Compress: Computing Persistent Homology in Chunks. [http://arxiv.org/pdf/1303.0477.pdf](arXiv:1303.0477)
5. U.Bauer, M.Kerber, J.Reininghausc, H.Wagner: Phat - Persistent Homology Algorithms Toolbox. Journal of Symbolic Computation 78, 2017, p. 76-90.
