Mathematical Preliminaries
==========================
To prepare for a detailed explaination of what RIVET can do and how it is used, we review some basic mathematical notions and establish some terminology.

To start, we define a partial order on \\(\\mathbb R^2\\) by taking  \\(a \\leq b\\) if and only if \\(a_1 \\leq b_1\\) and \\(a_2 \\leq b_2\\).

Bifiltrations
^^^^^^^^^^^^^
A *bifiltration* \\(F\\) is a collection of finite simplicial complexes indexed by \\(\\mathbb R^2\\) such that \\(F_a\\subset F_b\\) whenever \\(a\\leq b\\). In the computational setting, the bifiltrations \\(F\\) we encounter are always *essentially finite*.  This finiteness condition can be specified succinctly in the language of category theory: \\(F\\) is essentially finite if \\(F\\) is a left Kan extension of a diagram indexed by a finite grid in \\(\\mathbb R^2\\).  (See the RIVET paper for a more elementary definition.)  Such \\(F\\) can be specified by a single simplicial complex \\(S\\) (the colimit of \\(F\\) ) together with a collection of incomparable points \\(\\mathrm{births}(\\sigma)\\subset\\mathbb R^2\\) for each simplex \\(\\sigma\\in  S\\), specifying the bigrades at which \\\(\\sigma\\) is born.  If \\(\\mathrm{births}(\\sigma)\\) contains one element for each \\(\\sigma\\in S\\), then we say \\(F\\) is *one-critical*.  Otherwise, we say \\(F\\) is *multi-critical*.

We next introduce two contructions of bifiltrations from data.

Function-Rips Bifiltraiton
^^^^^^^^^^^^^^^^^^^^^^^^^^^
For \\(P\\) a finite metric space and \\(r\\geq 0\\), let \\(N(P)_r\\) denote the \\(r\\)-neighborhood graph of \\(P\\), i.e., the vertex set of \\(N(P)_r\\) is \\(P\\), and edge \\([i,j]\\in N(P)_r\\) if and only if \\(d(i,j)\\leq r\\).  If \\(r<0\\), we define \\(N(P):=\\emptyset.\\)  We define the *Vietoris-Rips complex* \\(R(P)_r\\) to be the clique complex on \\(N(P)_r\\), i.e. the largest simplical complex with 1-skeleton \\(N(P)_r\\).

Given a finite metric space \\(P\\) and any function \\(\\gamma:P\\to \\mathbb R\\), we define the *function-Rips* bifiltration \\(FR(\\gamma)\\) as follows: \\[FR(\\gamma)_{a,b}:=R(\\gamma^{-1}(-\\infty,a])_b.\\]  \\(FR(\\gamma)\\) is always 1-critical.

\\(\\gamma\\) is often chosen to be a density estimate on \\(P\\).  Another common choice is to take \\(\\gamma\\) to be a coeccentricity function on \\(P\\), e.g., \\(\\gamma(x):= \\sum_{y\\in P} d(x,y)\\).

Degree-Rips Bifiltration
^^^^^^^^^^^^^^^^^^^^^^^^

For \\(d\\in \\mathbb R\\), let \\(N(P)_{d,r}\\) be the subgraph of \\(N(P)_r\\) obtained by removing all vertices of degree less than \\(d\\).  We define the *degree-Rips bifiltration*  \\(DR(P)\\) by taking \\(DR(P)_{d,r}:=N(P)_{d,r}.\\)  (Note that this is in fact a bifiltration indexed by \\(\\mathbb R^{\\mathrm{op}}\\times \\mathbb R\\), where \\(\\mathbb R^{\\mathrm{op}}\\) denotes the opposite poset of \\(\\mathbb R\\); that is, \\(DR(P)_{a,b}\\subset DR(P)_{a',b'}\\) whenever \\(a\\geq a'\\) and \\(b\\leq b'\\).)   If \\(P\\) has more than one point, then \\(DR(P)\\) is multi-critical.

2-Parameter Persistence Modules 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Let us fix a field \\(K\\).  A *2-parameter persistence module* (also called a 2-D persistence module) \\(M\\) is a diagram of \\(K\\)-vector spaces indexed by \\(\\mathbb R^2\\).  That is, \\(M\\) is a collection of vector spaces \\(\\{M_a\\}_{a\\in \\mathbb{R^2}}\\), together with a collection of linear maps \\[\\{M_{a,b}:M_a\\to M_b\\}_{a\\leq b}\\] such that \\(M_{a,a}=\\mathrm{Id}_{M_a}\\) and \\(M_{b,c}\\circ M_{a,b}=M_{a,c}\\) for all \\(a \\leq b\\leq c\\).

A *morphism* \\(f:M\\to N\\) of 2-D persistence modules is a collection of maps \\[\\{f_a:M_a\\to N_a\\}_{a\\in \\mathbb R^2}\\]such that \\[f_b\\circ M_{a,b}= N_{a,b} \\circ f_a\\] for all \\(a\\leq b\\in \\mathbb R^2\\).  This definition of morphism gives the 2-D persistence modules the structure of an abelian category; thanks in part to this, many usual constructions for modules from abstract algebra have analogues for 2-D persistence modules.  In particular, direct sums and quotients are well defined.  

Free Persistence Modules
^^^^^^^^^^^^^^^^^^^^^^^^
For \\(c \\in \\mathbb R^2\\), define the 2-D persistence module \\(\\mathcal I^c\\) by
\\[\\mathcal I^c_a=
\\begin{cases}
K &\\mathrm{if }\\ a\\geq c,\\\\ 0 & \\mathrm{otherwise.}
\\end{cases}
\\qquad
\\mathcal I^c_{a,b}=
\\begin{cases}
\\mathrm{Id}_K &\\mathrm{if }\\ a\\geq c,\\\\ 0 & \\mathrm{otherwise.}
\\end{cases}
\\]
Note that the support of  \\(\\mathcal I^a\\) is the closed upper quadrant in \\(\\mathbb R^2\\) with lower left corner at \\(a\\).

A *free 2-D persistence module* is one isomorphic to \\[\\oplus_{c\\in \\mathcal B}\\ \\mathcal I^c\\] for some multiset \\(\\mathcal B\\) of points in \\(\\mathbb R^2\\).  
There is a natural definition of basis for free modules, generalizing the definition of bases for vector spaces in linear algebra.  In close analogy with linear algebra, a morphism \\(f:M\\to N\\) of finitely generated free modules can be represented by a matrix, with respect to a choice of bases for \\(M\\) and \\(N\\).

Presentations
^^^^^^^^^^^^^
A *presentation* of a 2-D persistence module M a map \\(f:F\\to G\\) such that \\(M\\cong G/\\mathrm{im}\\ f\\).  We say \\(M\\) is finitely presented if \\(F\\) and \\(G\\\) can be chosen to be finitely generated.  If \\(M\\) is finitely presented then, up to isomorphism, there exists a presentation \\(f:F\\to G\\) such that both \\(F\\) and \\(G\\) are minimial, i.e., for any other presentation \\(f':F'\\to G'\\),  \\(F\\) is a summand of \\(F'\\) and \\(G\\) is a summand of  \\(G'\\).  We call such a presentation *minimal*.  Minimal presentations are unique up to isomorphism, but importantly, their matrix representations are non-unique.

FIReps (Short Chain Complexes of Free Modules)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
We define a *FIRep* to be chain complex of free 2-D persistence modules of length 3.  Explicitly, then, an firep is a sequence of free 2-D persistence modules
\\[ C_2 \\xrightarrow{f} C_1 \\xrightarrow{g} C_0. \\]
such that \\(g\\circ f=0\\).  Associated to an firep is a unique homology module \\(\\ker g/\\mathrm{im}\\ f\\).  A presentation of a 2-parameter persistence module can be thought of as a special case of an FIRep, where the last module is trivial.

Homology of a Bifiltration
^^^^^^^^^^^^^^^^^^^^^^^^^^
Applying \\(i^{\\mathrm{th}}\\) simplicial homology with coefficients in \\(K\\) to each simplicial complex and each inclusion map in a bifiltration \\(F\\) yields a 2-D persistence module \\(H_i(F)\\).  If \\(F\\) is essentially finite, then \\(H_i(F)\\) is finitely presented


\\(H_i(F)\\) is in fact the \\(i^{\\mathrm{th}}\\) homology module of a chain complex 
\\( C(F)\\) of 2-D persistence modules whose value at each point in \\(a\\in \\mathbb R^2\\) is the simplical chain complex of \\(F_a\\).  If \\(F\\) is one-critical, each module of \\(C(F)\\) is free.  In general, \\(C(F)\\) needn’t be free, but given the portion of \\(C(F)\\) at indexes \\(i-1,\\) \\(i\\), and \\(i+1\\), it is easy to construct an FIRep whose homology is \\(H_i(F)\\); this is `an observation of Chacholski et al. <https://arxiv.org/abs/1409.7936>`_




Invariants of a 2-D Persistence Module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
As mentioned above, RIVET computes and visualizes three simple invariants of a 2-D persistence module \\(M\\):

* The *fibered barcode*, i.e. the function sending each affine line \\(L\\subset \\mathbb R^2\\) with non-negative slope to the barcode \\(\\mathcal B(M^L)\\) of the restriction \\(M^L\\) of \\(M\\) along \\(L\\).
* The *Hilbert function*, i.e., the function \\(\\mathbb R^2\\to \\mathbb N\\) which sends \\(a\\) to \\(\\dim M_a\\).
* The *bigraded Betti numbers* \\(\\xi_i(M)\\). These are functions \\(\\mathbb{R}^2 \\to \\mathbb{N}\\) that, respectively, count the number of births, deaths, and "relations amongst deaths" at each bigrade. Formally, given \\(r \\in \\mathbb{R}^2\\) and a minimal free resolution $$0 \\to F^2\\to F^1\\to F^0$$ for \\(M\\), \\(\\xi_i(M)(r)\\) is the number of elements at bigrade \\(r\\) in a basis for \\(F^i\\).

Coarsening a Persistence Module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Given a finitely presented 2-D persistence module \\(M\\), we can *coarsen* \\(M\\) to obtain an algebraically simpler module carrying approximately the same persistence information as \\(M\\).  As we will describe it here, the coarsening operation depends on a choice of finite grid \\(G\\subset\\mathbb R^2\\), such that \\(G\\) contains some element ordered ater all bigrades of generators and relations in a minimal presentation for \\(M\\).  The coarsened module, denoted \\(M^G\\), is defined by taking \\(M^G_a:= M_g\\), where \\(g\\in G\\) is the minimum grid element such that \\(a\\leq g\\).  The internal maps in \\(M^G\\) are induced by those in \\(M\\) in the obvious way.

.. We can describe the coarsening operation succinctly in the language of category theory: Let \\(G\\subset\\mathbb R^2\\) be a finite grid.  First, we take the restriction of \\(M\\) along \\(G\\), and then take the left (or right) Kan extension of this along the inclusion of \\(G\\hookrightarrow \\mathbb R^2\\).  Currently, RIVET uses the right Kan extension.