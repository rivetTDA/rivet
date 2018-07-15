About RIVET
=====================================

RIVET is a tool for topological data analysis, and more specifically, for the visualization and analysis of two-parameter persistent homology.  RIVET was initially designed with interactive visualization foremost in mind, and this continues to be a central focus.  But RIVET also provides functionality for two-parameter persistence computation that should be useful regardless of whether the user wishes to do any visualization.  

Many of the mathematical and algorithmic ideas behind RIVET are explained in detail in the paper “|RIVET_Paper|.”  Additional papers  describing other aspects of RIVET are in preparation.

.. |RIVET_Paper| raw:: html  

   <a href="http://arxiv.org/abs/1512.00180"  target="_blank" rel="noopener">Interactive Visualization of 2-D Persistence Modules</a>


RIVET is written in C++, and depends on the |qt_Link|, |Boost_Link|, and |MessagePack_Link| libraries.  In addition, RIVET now incorporates some code from |PHAT_Link|.  


.. |qt_Link| raw:: html 

   <a href="https://www.qt.io/" target="_blank" rel="noopener">qt</a> 

.. |Boost_Link| raw:: html 

   <a href="http://www.boost.org/" target="_blank" rel="noopener">Boost</a>

.. |MessagePack_Link| raw:: html

   <a href="https://msgpack.org/index.html" target="_blank">MessagePack</a>

.. |PHAT_Link| raw:: html 

   <a href="https://bitbucket.org/phat-code/phat/src/master/" target="_blank">PHAT</a>


Contributors
------------

RIVET project was founded by |Lesnick_Link| and |Wright_Link|.  In 2013-2015 they designed and developed a first version of RIVET.  Matthew was the sole author of the code during this time. Since 2016, several others have made valuable contributions to RIVET, some of which will be incorporated into later releases.

Here is a list of contributors, with a brief, incomplete summary of contributions:

* Madkour Abdel-Rahman (St. Olaf College): Error handling 	
* |Keller_Link| (Intel Labs): Parallel-friendly code organization, command line interactivity, API, Python wrappers, software design leadership.
* |Lesnick_Link| (Princeton): Design, performance optimizations, computation of minimal presentations
* Phil Nadolny (St. Olaf College): Error Handling, code for constructing path through dual graph
* |Wright_Link| (St. Olaf College): Design, primary developer
* Simon Segert (Princeton) Major improvements to the GUI, extensions of RIVET 
* David Turner (Princeton) Parallel minimization of a presentation
* Alex Yu (Princeton) Extensions of RIVET 
* |Zhao_Link| (Berkeley): Handling of multicritical bifiltrations and Degree-Rips bifiltrations, performance optimizations 

.. |Lesnick_Link| raw:: html 

   <a href="http://www.princeton.edu/~mlesnick/" target="_blank">Michael Lesnick</a>

.. |Wright_Link| raw:: html 

   <a href="http://www.mrwright.org/" target="_blank">Matthew Wright</a>

.. |Keller_Link| raw:: html

   <a href="http://www.xoltar.org/" target="_blank">Bryn Keller</a>

.. |Zhao_Link| raw:: html

   <a href="https://math.berkeley.edu/~rhzhao/" target="_blank">Roy Zhao</a>


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

Additional support has been been provided by the Institute for Mathematics and its Applications, Columbia University, Princeton University, St. Olaf College, and the NIH (grant U54-CA193313-01).

.. |NSF_Link| raw:: html

   <a href="https://www.nsf.gov/awardsearch/showAward?AWD_ID=1606967" target="_blank” rel="noopener">DMS-1606967</a>


License
-------

RIVET is made available under the under the terms of the GNU General Public License, available |GPL_Link|. The software is provided "as is," without warranty of any kind, even the implied warranty of merchantability or fitness for a particular purpose. See the GNU General Public License for details.

.. |GPL_Link| raw:: html

   <a href="https://www.gnu.org/licenses/gpl-3.0.en.html" target="_blank"  rel="noopener">here</a>
