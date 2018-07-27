About
=====================================

RIVET is a tool for topological data analysis, and more specifically, for the visualization and analysis of two-parameter persistent homology.  RIVET was initially designed with interactive visualization foremost in mind, and this continues to be a central focus.  But RIVET also provides functionality for two-parameter persistence computation that should be useful for other purposes as well.  

Many of the mathematical and algorithmic ideas behind RIVET are explained in detail in the paper “|RIVET_Paper|.”  Additional papers  describing other aspects of RIVET are in preparation.

.. |RIVET_Paper| raw:: html  

   <a href="http://arxiv.org/abs/1512.00180"  target="_blank" rel="noopener">Interactive Visualization of 2-D Persistence Modules</a>


RIVET is written in C++, and depends on the |qt_Link|, |Boost_Link|, and |MessagePack_Link| libraries.  In addition, RIVET now incorporates some modified code from |PHAT_Link|.  


.. |qt_Link| raw:: html 

   <a href="https://www.qt.io/" target="_blank" rel="noopener">qt</a> 

.. |Boost_Link| raw:: html 

   <a href="http://www.boost.org/" target="_blank" rel="noopener">Boost</a>

.. |MessagePack_Link| raw:: html

   <a href="https://msgpack.org/index.html" target="_blank">MessagePack</a>

.. |PHAT_Link| raw:: html 

   <a href="https://bitbucket.org/phat-code/phat/src/master/" target="_blank">PHAT</a>

Overview
--------

The basic idea of 2-parameter persistent homology is simple: Given a data set (for example, a point cloud in \\(\\mathbb R^n\\), one constructs a 2-parameter family of simplicial complexes called a *bifiltration* whose topological structure captures some geometric structure of interest about the data.  For example, the bifiltration may encode information about the presence of clusters, holes, or tendrils in the data.  Applying simplicial homology to the bifiltration gives a diagram of vector spaces called a *2-parameter persistence module*, which algebraically encodes information about the topological structure of the bifiltration.  In contrast to the 1-parameter case, 2-parameter persistence modules can have very delicate algebraic structure.  As result, there is (in a sense that can be made precise) no good definition of a barcode for 2-parameter persistence modules.

Nevertheless, one can define invariants of 2-parameter persistence modules that capture aspects of their structure relevant for data analysis.  RIVET computes and visualizes three such kinds of invariants, the *Hilbert function*, the *bigraded Betti numbers*, and the *fibered barcode.*  

The fibered barcode is a parameterized family of barcodes obtained by restricting a 2-parameter persistence module to various lines in parameter space.  A key feature of RIVET is that it computes a data structure called the *augmented arrangement* on which fast queries of these barcodes can be performed.  These queries are used by RIVET's GUI to provide an interactive visualization of the fibered barcode.

RIVET also computes *minimal presentations* of 2-parameter persistence modules.  These are specifications of the full algebraic structure of a persistence module which are as small as possible, in a certain sense.  Those who wish to study invariants of  2-parameter persistent homology not computed by RIVET may find it useful to take the minimal presentations output by RIVET as a starting point.  In practice, when working with real data, these presentations are often surprisingly small.

Contributors
------------

RIVET project was founded by |Lesnick_Link| and |Wright_Link|, who designed and developed a first version of RIVET in 2013-2015.  Matthew was the sole author of the code during this time. Since 2016, several others have made valuable contributions to RIVET, some of which will be incorporated into later releases.

Here is a list of contributors, with a brief, incomplete summary of contributions:

* Madkour Abdel-Rahman : Error handling 	
* |Keller_Link| : Parallel-friendly code organization, command line interactivity, API, Python wrappers, software design leadership.
* |Lesnick_Link| : Design, performance optimizations, computation of minimal presentations
* Phil Nadolny : Error handling, code for constructing path through dual graph
* |Wright_Link| : Design, primary developer
* Simon Segert : Major improvements to the GUI, extensions of RIVET 
* David Turner : Parallel minimization of a presentation
* Alex Yu : Extensions of RIVET 
* |Zhao_Link| : Multicritical bifiltrations and Degree-Rips bifiltrations, performance optimizations 

.. |Lesnick_Link| raw:: html 

   <a href="http://www.princeton.edu/~mlesnick/" target="_blank">Michael Lesnick</a>

.. |Wright_Link| raw:: html 

   <a href="http://www.mrwright.org/" target="_blank">Matthew Wright</a>

.. |Keller_Link| raw:: html

   <a href="http://www.xoltar.org/" target="_blank">Bryn Keller</a>

.. |Zhao_Link| raw:: html

   <a href="https://math.berkeley.edu/~rhzhao/" target="_blank">Roy Zhao</a>

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

The RIVET project is supported in part by the National Science Foundation under grant |NSF_Link|.  Any opinions, findings, and conclusions or recommendations expressed in this material are those of the author(s) and do not necessarily reflect the views of the National Science Foundation.

Additional support has been been provided by the Institute for Mathematics and its Applications, Columbia University, Princeton Neuroscience Institute, St. Olaf College, and the NIH (grant U54-CA193313-01).

.. |NSF_Link| raw:: html

   <a href="https://www.nsf.gov/awardsearch/showAward?AWD_ID=1606967" target="_blank” rel="noopener">DMS-1606967</a>

Special thanks to Jon Cohen at Princeton for his support of the RIVET project.  Thanks also to Ulrich Bauer for many enlightening conversations about computation of 1-parameter persistent homology, which have influenced our thinking about 2-parameter persistence.  

License
-------

RIVET is made available under the under the terms of the GNU General Public License, available |GPL_Link|. The software is provided "as is," without warranty of any kind, even the implied warranty of merchantability or fitness for a particular purpose. See the GNU General Public License for details.

.. |GPL_Link| raw:: html

   <a href="https://www.gnu.org/licenses/gpl-3.0.en.html" target="_blank"  rel="noopener">here</a>
   

Documentation Todos
-------------------
This new version of the documentation is a draft, and still needs a lot of polish.

More important todos:
* The flow chart is not displaying properly on the ReadTheDocs site.
* A lot of displayed math is not displaying properly on the .pdf provided by read the docs.
* The text is not updated to explain how the persistence diagram window works in Simon's improvements to the visualization.  (Right now there is a disclamier about this.)
* some language in rivet_console --h needs to be edited to sync properly with the new changes to the documentation.
* The documentation is missing a specification of the output formats of the minimal presentation, Hilbert Function, and bigraded Betti numbers.
* We need to add some good examples.

Minor Todos:
* The naming and italicization of the "Input data" and "Module Invariant" files needs to fixed in several places.
* Is the name Hilbert Function used throughout?
* The hyperlinks in about.rst don't yet take advantage of the simple syntax made possible by Matthew's Javascript solution.
* It's a small thing, but the .png of the the file input dialog looks a little off center.

   
