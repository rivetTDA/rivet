..    include:: <isonum.txt>

Overview of RIVET
=====================================

Mathematical Preliminaries
--------------------------
To explain what RIVET does, we must first review some basic mathematical notions.

For \\(a,b \\in \\mathbb{R}^2\\), we write \\(a \\leq b\\) if \\(a_1 \\leq b_1\\) and \\(a_2 \\leq b_2\\).

Let us fix a field \\(K\\).  REMOVE (Currently RIVET only supports \\(K= \\mathbb Z\2 \mathbb Z\\), but only relatively minor changes are required to support other finite fields.)

A *2-D persistence module* \\(M\\) is a diagram of \\(K\\)-vector spaces indexed by $$R^n$.  That is, M is a collection of vector spaces \\(\\{M_a\\}_{a\\in \\mathbb{R^2}}\\), together with a collection of linear maps \\(\\{M_{a,b}:M_a\\to M_b\\}_{a\\leq b}\\), such that \\(M_{a,a}=\\mathrm{Id}_{M_a}\\) for all \\(a\\) and \\(M_{b,c}\\circ M_{a,b}=M_{a,c}\\) for all \\(a \\leq b\\leq c\\).  2-D persistence modules are the basic objects of study in RIVET.  We call the function $\mathbb R^2\to \mathbb N$ which sends $a$ to $\dim M_a$ the *Hilbert Function* of $a$.  

A *morphism* f:M\to N of 2-D persistence modules is a collection of maps $\{f_a:M_a\to N_a}_{a\in \R^2} such that f_b\circ M_{a,b}= N_{a,b} \circ f_a for all a\leq b in $\R^2$.  This definition of morphism gives the 2-D persistence modules the structure of an abelian category; thanks in part to this, many usual constructions for modules from abstract algebra have analogues for 2-D persistence modules.  In particular, direct sums and quotients are well defined.  

In particular, we have a well defined notion of a free 2-D persistence module; we refer the reader to the RIVET paper for the precise definition.  Such a module is specified up to isomorphism by a list of elements of $\R^2$.  There is a natural definition of basis for free modules, generalizing the definition of bases for vector spaces in linear algebra.  In close analogy with linear algebra, a morphism $f:M\to N$ of finitely generated free modules can be represented by a matrix.

A *presentation* of a 2-D persistence module M is a map $f:F\to G$ such that M\cong G/im f.  If M is finitely presented then, up to isomorphism, there is always a unique smallest 
9ie., minimal) choice of both F and G.  If both F and G are chosen to be minimal, we say $f:F\to G$ is a minimal presentation.  Note that a minimal presentation needn’t be unique.

We define an *firep* to be chain complex of free 2-D persistence modules of length 3.  Explicitly, then, an firep is a sequence of free 2-D persistence modules such that

Associated to an firep is a unique homology module SDFSDF.  A presentation of a 2-parameter persistence module can be thought of as a special case of an FIRep, where the last module is trivial.


A *bifiltration* \\(F\\) is a diagram of finite simplicial complexes indexed by $R^2$ such that \\(F_a\\subset F_b\\) whenever \\(a\leq b\\). In the computational setting, the bifiltrations $F$ we encounter always *essentially finite*, i.e., $F$ is a left Kan extension of a diagram indexed by a finite grid in $R^2$.  Such F can be specified by a single simplicial complex S (the colimit of F) together with a collection of incomparable points births(\sigma) in R^2 for each simplex sigma\in  S, specifying the bigrades at which sigma is born.  If births(\sigma) contains one element for each $\sigma\in S$, then we say F is one-critical.  Otherwise, we say F is multi-critical.

Applying ith simplicial homology to each simplicial complex and each inclusion map in a bifiltration $F$ yields a 2-D persistence module H_i(F).  $H_i(F)$ is in fact the $i^th$ homology module of a chain complex of 2-D persistence modules
$\cdots \xrightarrow{\partial_3} C_2(F)\xrightarrow{\partial_2} C_{1}{F) \xrightarrow{\partial_{1}} C_{0}{F), where $C_i(F)_z$ is the usual simpilcial chain vector space $C_i(F_z)$.  If F is one-critical, each $C_i(F)$ is free.  In general, $C_i(F)$ needn’t be free, but it is easy to construct an firep whose homology is H_i(F)$ from the short chain complex $C_{i+1}(F)\xrightarrow{\partial_{i+1} C_{i}{F) \xrightarrow{\partial_{i}} C_{i-1}{F}; this is an observation of Chacholski et al.

For P a finite metric space and $r\geq 0$, let $N(P)_r$ denote the $r$-neighborhood graph of $P$, i.e., the vertex set of $N(P)_r$ is $P$, and edge $[i,j]\in N(P)_r$ if and only if d(i,j)\leq r.  If $r<0$ we define N(P)=\emptyset$.  We define the Vietoris-Rips filtration R(P)_r$ to be the clique complex on $N(P)_r$, i.e. the largest simplical complex with 1-skeleton $N(P)_r$.

Given a finite metric space $P$ and any function $\gamma:P\to \R$, we define the *function-Rips* bifiltration R(\gamma) as follows: $R(\gamma)_a,b=VR(\gamma^{-1}(-\inft,a])_b$.

For $d\in \R$ Let N(P)_{d,r} be the subgraph of N(P)_r obtained by removing all vertices of degree less than d.  We define the *degree-Rips bifiltration*  DR(P) by taking $DR(P)_{DKr}=N(P)_{d,r}$.  (Note that this is in fact a bifiltration indexed by $\R^op\times \R$, i.e., the complexes get larger.


The RIVET Pipeline
------------------
Here is the basic RIVET for pipeline studying the 2-parameter persistent homology of data in degree $I$:

Data |rarr|  
Bifiltration |rarr| firep |rarr| Minimal Presentation of Homology Module |rarr| Invariants [rarr] Visualization.  


We now explain this pipeline, referring the reader to the RIVET paper and related literature for more details.  

RIVET can accept as input a data set, a (simplicial) bifiltration, or an FIRep.  RIVET always works with a single homology degree at a time; when giving data or a bifiltration as input, one must specify the degree of homology to consider.  Thus RIVET needs only to consider chain complexes of length at most 3 (i.e., those specifying a single homology module.)  

RIVET accepts data in the form of either a point cloud in Rn, or a finite metric space (represented as distance matrix).  Optionally, a function on each point can be given.  If a function is given, RIVET computes a Vietoris-Rips bifiltration of the data.  If no function is given, RIVET computes the degree-Rips bifiltration of the data.  See the  RIVET paper for the definitions of these bifiltration.  (We hope that in the future RIVET will build a wider class of bifiltrations for a wider range of data types.  In the meantime, the user should be able to work with any type of data and bifiltration (subject to complexity constraints) by directly passing a bifiltration or FIRep as input.)

RIVET can handle multi-critical bifiltrations, i.e., bifiltrations where simplices are born at multiple incomparable grades in R^2.  In particular, the degree-Rips bifiltration is multi-critical.  As noted by Chacholski et al., the simplicial chain complex of a multi-critical bififiltration needn’t be free, but for fixed j >= 0, it is easy to obtain an FIRep whose homology is isomorphic to the homology in degree j.  This is what RIVET does. 

RIVET computes and visualizes three invariants of a 2-parameter persistent homology module:  

* Bigraded Betti numbers. 

* Hilbert Functions (i.e., the pointwise dimension function).  

* Barcodes of the restriction of the module along affine lines.  

In addition, RIVET can directly output a minimal presentation.  Those who wish to work with 2-parameter persistent homology in ways not supported by RIVET may find it useful to take the minimal presentations output by RIVET as a starting point.  In practice, these presentations are often surprisingly small, particularly when one coarsens the module (see SECTION).