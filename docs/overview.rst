..    include:: <isonum.txt>

Overview of RIVET
=====================================

Mathematical Preliminaries
--------------------------
To explain what RIVET does, we must first review some basic mathematical notions.

For \\(a,b \\in \\mathbb{R}^2\\), we write \\(a \\leq b\\) if \\(a_1 \\leq b_1\\) and \\(a_2 \\leq b_2\\).

A *bifiltration* \\(F\\) is a diagram of finite simplicial complexes indexed by $R^2$ such that \\(F_a\\subset F_b\\) whenever \\(a\leq b\\). In the computational setting, the bifiltrations $F$ we encounter always *essentially finite*, i.e., $F$ is a left Kan extension of a diagram indexed by a finite grid in $R^2$.  Such F can be specified by a single simplicial complex S (the colimit of F) together with a collection of incomparable points births(\sigma) in R^2 for each simplex sigma\in  S, specifying the bigrades at which sigma is born.  If births(\sigma) contains one element for each $\sigma\in S$, then we say F is one-critical.  Otherwise, we say F is multi-critical.

For P a finite metric space and $r\geq 0$, let $N(P)_r$ denote the $r$-neighborhood graph of $P$, i.e., the vertex set of $N(P)_r$ is $P$, and edge $[i,j]\in N(P)_r$ if and only if d(i,j)\leq r.  If $r<0$ we define N(P)=\emptyset$.  We define the Vietoris-Rips filtration R(P)_r$ to be the clique complex on $N(P)_r$, i.e. the largest simplical complex with 1-skeleton $N(P)_r$.

Given a finite metric space $P$ and any function $\gamma:P\to \R$, we define the *function-Rips* bifiltration R(\gamma) as follows: $R(\gamma)_a,b=VR(\gamma^{-1}(-\inft,a])_b$.  R(\gamma) is alway 1-critical.

For $d\in \R$ Let N(P)_{d,r} be the subgraph of N(P)_r obtained by removing all vertices of degree less than d.  We define the *degree-Rips bifiltration*  DR(P) by taking $DR(P)_{DKr}=N(P)_{d,r}$.  (Note that this is in fact a bifiltration indexed by $\R^op\times \R$, i.e., the complexes get larger.  $DR(P)$ is multi-critical when P has more than one point.

Let us fix a field \\(K\\).  REMOVE (Currently RIVET only supports \\(K= \\mathbb Z\2 \mathbb Z\\), but only relatively minor changes are required to support other finite fields.). A *2-D persistence module* \\(M\\) is a diagram of \\(K\\)-vector spaces indexed by $$R^n$.  That is, M is a collection of vector spaces \\(\\{M_a\\}_{a\\in \\mathbb{R^2}}\\), together with a collection of linear maps \\(\\{M_{a,b}:M_a\\to M_b\\}_{a\\leq b}\\), such that \\(M_{a,a}=\\mathrm{Id}_{M_a}\\) for all \\(a\\) and \\(M_{b,c}\\circ M_{a,b}=M_{a,c}\\) for all \\(a \\leq b\\leq c\\).  2-D persistence modules are the basic objects of study in RIVET.

A *morphism* f:M\to N of 2-D persistence modules is a collection of maps $\{f_a:M_a\to N_a}_{a\in \R^2} such that f_b\circ M_{a,b}= N_{a,b} \circ f_a for all a\leq b in $\R^2$.  This definition of morphism gives the 2-D persistence modules the structure of an abelian category; thanks in part to this, many usual constructions for modules from abstract algebra have analogues for 2-D persistence modules.  In particular, direct sums and quotients are well defined.  

In particular, we have a well defined notion of a free 2-D persistence module; we refer the reader to the RIVET paper for the precise definition.  Such a module is specified up to isomorphism by a list of elements of $\R^2$.  There is a natural definition of basis for free modules, generalizing the definition of bases for vector spaces in linear algebra.  In close analogy with linear algebra, a morphism $f:M\to N$ of finitely generated free modules can be represented by a matrix.

A *presentation* of a 2-D persistence module M is a map $f:F\to G$ such that M\cong G/im f.  If M is finitely presented then, up to isomorphism, there is always a unique smallest 
9ie., minimal) choice of both F and G.  If both F and G are chosen to be minimal, we say $f:F\to G$ is a minimal presentation.  Note that a minimal presentation needn’t be unique.

We define an *firep* to be chain complex of free 2-D persistence modules of length 3.  Explicitly, then, an firep is a sequence of free 2-D persistence modules such that

Associated to an firep is a unique homology module SDFSDF.  A presentation of a 2-parameter persistence module can be thought of as a special case of an FIRep, where the last module is trivial.

Applying ith simplicial homology to each simplicial complex and each inclusion map in a bifiltration $F$ yields a 2-D persistence module H_i(F).  $H_i(F)$ is in fact the $i^th$ homology module of a chain complex of 2-D persistence modules
$\cdots \xrightarrow{\partial_3} C_2(F)\xrightarrow{\partial_2} C_{1}{F) \xrightarrow{\partial_{1}} C_{0}{F), where $C_i(F)_z$ is the usual simplicial chain vector space $C_i(F_z)$.  If F is one-critical, each $C_i(F)$ is free.  In general, $C_i(F)$ needn’t be free, but it is easy to construct an firep whose homology is H_i(F)$ from the short chain complex $C_{i+1}(F)\xrightarrow{\partial_{i+1} C_{i}{F) \xrightarrow{\partial_{i}} C_{i-1}{F}; this is an observation of Chacholski et al.


For algebraic reasons, there is no good way to define the barcode of a 2-parameter persistence module.  However, one can define many invariants of a 2-D persistence module \\(M\\), which capture partial information about the algebraic structure of $M$.  RIVET computes and visualizes three such invariants:

* The barcode \\(\\mathcal B(M^L)\\) of the restriction \\(M^L\\) of \\(M\\) along \\(L\\), for \\(L\\) any affine line with non-negative slope.
* The Hilbert function, i.e., the function $\mathbb R^2\to \mathbb N$ which sends $a$ to $\dim M_a$.
* The *bigraded Betti numbers* \\(\\xi_i(M)\\). These are functions \\(\\mathbb{R}^2 \\to \\mathbb{N}\\) that, respectively, count the number of births, deaths, and "relations amongst deaths" at each bigrade. Formally, given \\(r \\in \\mathbb{R}^2\\) and a minimal free resolution $$0 \\to F^2\\to F^1\\to F^0$$ for \\(M\\), \\(\\xi_i(M)(r)\\) is the number of elements at bigrade \\(r\\) in a basis for \\(F^i\\).

The RIVET Pipeline
------------------
The following figure illustrates RIVET’s pipeline for working with the 2-parameter persistent homology of data.

.. figure:: images/flowchart.pdf  
    :width: 100px
    :align: center
    :height: 200px
    :figclass: align-center
   
    The RIVET pipeline.  Green items are data types that can be input directly to RIVET via a file; the format of such files is specified on the LINK page.  Yellow items are data types that can be printed to the console.  Items with red boundary are stored in a binary file which serves as as input to RIVET’s visualization.

We now explain this pipeline.  

RIVET can accept as input a data set, a  bifiltration, or an firep.  RIVET always works with a single homology degree at a time; when giving data or a bifiltration as input, one must specify the degree of homology to consider.  

RIVET accepts data in the form of either a point cloud in Rn, or a finite metric space (represented as distance matrix).  Optionally, a function on each point can be given.  If a function is given, RIVET computes a function-Rips bifiltration.  If no function is given, RIVET computes the degree-Rips bifiltration.

Given a bifiltration F, RIVET constructs an firep for the H_j(F) in the specified degree j.  If F is multi-critical, RIVET uses the trick of Chacholski et al. mentioned above to obtain an firep.

Given an firep, RIVET computes a minimal presentation of the homology module.  This computation also yields the Hilbert function of the module with almost no extra work.  RIVET can print the minimal presentation to the console in a column-sparse format.  (TODO: Need more detail on the output format in the documentation.)  Those who wish to work with 2-parameter persistent homology in ways not supported by RIVET may find it useful to take the minimal presentations output by RIVET as a starting point.  In practice, these presentations are often surprisingly small, particularly when one coarsens the module (see SECTION).  The algorithm for computing the minimal presentation is novel and is the subject of a paper in preparation.

The 0th and 1st bigraded Betti numbers of 2-D persistent homology module $M$ can be read off of the minimal presentation directly.  Given these and the Hilbert function, a simple formula yields the 2nd bigraded Betti numbers as well.

RIVET uses the minimal presentation and bigraded Betti numbers of $M$ to compute a data structure called an *augmented arrangement*, which allows for fast queries of the barcodes of 1-D slices of a 2-D persistence module.  The augmented arrangement is a line arrangement in the right half plane, together with a barcode at each face of the line arrangement.
The fast queries play an important role in the RIVET visualization, but can also be performed directly from the command line.

To support its visualizations, RIVET saves the Hilbert Function, Bigraded Betti numbers, and Augmented Arrangement in a file which we call the “visualization data file.”  This file is written in a custom binary format and is intended to be read only by RIVET’s visualization.  The Hilbert function and Bigraded Betti numbers can also be output directly to the console.
