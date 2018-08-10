About
=====================================

RIVET is a tool for topological data analysis, and more specifically, for the visualization and analysis of two-parameter persistent homology.  RIVET was initially designed with interactive visualization foremost in mind, and this continues to be a central focus.  But RIVET also provides functionality for two-parameter persistence computation that should be useful for other purposes.  

Many of the mathematical and algorithmic ideas behind RIVET are explained in detail in the paper `Interactive Visualization of 2-D Persistence Modules <https://arxiv.org/pdf/1512.00180v1.pdf>`_.  
Additional papers describing other aspects of RIVET are in preparation.

RIVET is written in C++, and depends on the `Qt <https://www.qt.io/>`_, `Boost <http://www.boost.org/>`_, and `MessagePack <https://msgpack.org/index.html>`_ libraries.  
In addition, RIVET now incorporates some modified code from `PHAT <https://bitbucket.org/phat-code/phat/src/master/>`_.  


Overview
--------

The basic idea of 2-parameter persistent homology is simple: Given a data set (for example, a point cloud in \\(\\mathbb R^n\\)), one constructs a 2-parameter family of simplicial complexes called a *bifiltration* whose topological structure captures some geometric structure of interest about the data.  For example, the bifiltration may encode information about the presence of clusters, holes, or tendrils in the data.  Applying simplicial homology to the bifiltration gives a diagram of vector spaces called a *bipersistence module*, which algebraically encodes information about the topological structure of the bifiltration.  In contrast to the 1-parameter case, bipersistence modules can have very delicate algebraic structure.  As result, there is (in a sense that can be made precise) no good definition of a barcode for bipersistence modules.

Nevertheless, one can define invariants of bipersistence modules that capture aspects of their structure relevant for data analysis.  RIVET computes and visualizes three such kinds of invariants, the *Hilbert function*, the *bigraded Betti numbers*, and the *fibered barcode.*  

The fibered barcode is a parameterized family of barcodes obtained by restricting a bipersistence module to various lines in parameter space.  A key feature of RIVET is that it computes a data structure called the *augmented arrangement*, on which fast queries of these barcodes can be performed.  These queries are used by RIVET's GUI to provide an interactive visualization of the fibered barcode.

RIVET also computes *minimal presentations* of bipersistence modules.  These are specifications of the full algebraic structure of a persistence module which are as small as possible, in a certain sense.  Those who wish to study invariants of  2-parameter persistent homology not computed by RIVET may find it useful to take the minimal presentations output by RIVET as a starting point.  In practice, when working with real data, these presentations are often surprisingly small.


Contributors
------------

RIVET project was founded by `Michael Lesnick`_ and `Matthew Wright`_, who designed and developed a first version of RIVET in 2013-2015.  Matthew was the sole author of the code during this time. Since 2016, several others have made valuable contributions to RIVET, some of which will be incorporated into later releases.

Here is a list of contributors, with a brief, incomplete summary of contributions:

* Madkour Abdel-Rahman : Error handling 	
* `Bryn Keller`_ : Parallel-friendly code organization, command line interactivity, API, Python wrappers, software design leadership.
* `Michael Lesnick`_ : Design, performance optimizations, computation of minimal presentations
* Phil Nadolny : Error handling, code for constructing path through dual graph
* `Matthew Wright`_ : Design, primary developer
* Simon Segert : Major improvements to the GUI, extensions of RIVET 
* David Turner : Parallel minimization of a presentation
* Alex Yu : Extensions of RIVET 
* `Roy Zhao`_ : Multicritical bifiltrations and Degree-Rips bifiltrations, performance optimizations 

.. _Michael Lesnick: http://www.princeton.edu/~mlesnick/

.. _Matthew Wright: https://www.mlwright.org/

.. _Bryn Keller: http://www.xoltar.org/

.. _Roy Zhao: https://math.berkeley.edu/~rhzhao/

The RIVET development team can be reached by email at info@rivet.online.


Contributing
------------

We welcome your contribution! Code, documentation, unit tests, interesting sample data files are all welcome!

Before submitting your branch for review, please run the following from the top level RIVET folder you cloned from Github::

	clang-format -i **/*.cpp **/*.h


This will format the source code using the project's established source code standards (these are captured in the ``.clang-format`` file in the project root directory).

Issues
------

A full list of bugs and todos can be found on the `Github issue tracker <https://github.com/rivetTDA/rivet/issues>`_.
Please feel free to add any bugs/issues you discover, if not already listed.


Acknowledgments
---------------

The RIVET project is supported in part by the National Science Foundation under grant `DMS-1606967 <https://www.nsf.gov/awardsearch/showAward?AWD_ID=1606967>`_.  Any opinions, findings, and conclusions or recommendations expressed in this material are those of the author(s) and do not necessarily reflect the views of the National Science Foundation.

Additional support has been been provided by the Institute for Mathematics and its Applications, Columbia University, Princeton Neuroscience Institute, St. Olaf College, and the NIH (grant U54-CA193313-01).

Special thanks to Jon Cohen at Princeton for his support of the RIVET project.  Thanks also to Ulrich Bauer for many enlightening conversations about computation of 1-parameter persistent homology, which have influenced our thinking about 2-parameter persistence.  


License
-------

RIVET is made available under the under the terms of the `GNU General Public License <https://www.gnu.org/licenses/gpl-3.0.en.html>`_. The software is provided "as is," without warranty of any kind, even the implied warranty of merchantability or fitness for a particular purpose. See the GNU General Public License for details.


Citation
--------

For convenient citation of RIVET in your own work, we provide the following BibTex entry::

	@software{rivet
		author = {{The RIVET Developers}},
		title = {RIVET},
		url = {http://rivet.online},
		version = {1.0},
		year = {2018}
	}



Documentation Todos
-------------------
This new version of the documentation is a draft, and still needs a lot of polish.

Major Formatting todos:   

* A lot of displayed math is not displaying properly on the .pdf provided by read the docs.
* The caption for the figure on the "Computation Pipeline" page does not display properly in the downloaded .html. MW: It seems that the downloaded HTML is using a slightly different style sheet than the online version. However, nearly everyone will use the online version rather than downloading a zip archive containing all of the HTML and supporting files.

Content Todos:  

* The text is not updated to explain how the persistence diagram window works in Simon's improvements to the visualization.  (Right now there is a disclaimier about this.)
* I suggest to not print out xi_0, xi_1, and xi_2 when —Betti is called.
* The example could use some polish. More examples are desirable.

Minor Todos:  

* There is a formatting problem in the “cases” environment used in the definition of a free module.
* Is the name Hilbert Function used throughout?
* It's a small thing, but the .png of the the file input dialog looks a little off center.
* Some .rst files are no longer used in the documentation and can be removed (unless we decide to add them back in).

   
