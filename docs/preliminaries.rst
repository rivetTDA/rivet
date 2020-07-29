.. _preliminaries:


Mathematical Preliminaries
==========================
To prepare for a detailed explaination of what RIVET can do and how it is used, we review some basic mathematical notions and establish some terminology.

To start, we define a partial order on :math:`\mathbb R^2` by taking :math:`a \leq b` if and only if :math:`a_1 \leq b_1` and :math:`a_2 \leq b_2`.

Bifiltrations
^^^^^^^^^^^^^
A *bifiltration* :math:`F` is a collection of finite simplicial complexes indexed by :math:`\mathbb R^2` such that :math:`F_a\subset F_b` whenever :math:`a\leq b`. In the computational setting, the bifiltrations :math:`F` we encounter are always *essentially finite*.  This finiteness condition can be specified succinctly in the language of category theory: :math:`F` is essentially finite if :math:`F` is a left Kan extension of a diagram indexed by a finite grid in :math:`\mathbb R^2`.  (See the RIVET paper for a more elementary definition.)  Such :math:`F` can be specified by a single simplicial complex :math:`S` (the colimit of :math:`F` ) together with a collection of incomparable points :math:`\mathrm{births}(\sigma)\subset\mathbb R^2` for each simplex :math:`\sigma\in  S`, specifying the bigrades at which :math:`\sigma` is born.  If :math:`\mathrm{births}(\sigma)` contains one element for each :math:`\sigma\in S`, then we say :math:`F` is *one-critical*.  Otherwise, we say :math:`F` is *multi-critical*.

We next introduce two contructions of bifiltrations from data.

.. _funRipsBifil:

Function-Rips Bifiltration
^^^^^^^^^^^^^^^^^^^^^^^^^^^
For :math:`P` a finite metric space and :math:`r\geq 0`, let :math:`N(P)_r` denote the :math:`r`-neighborhood graph of :math:`P`, i.e., the vertex set of :math:`N(P)_r` is :math:`P`, and edge :math:`[i,j]\in N(P)_r` if and only if :math:`d(i,j)\leq r`.  If :math:`r<0`, we define :math:`N(P):=\emptyset.`  We define the *Vietoris-Rips complex* :math:`R(P)_r` to be the clique complex on :math:`N(P)_r`, i.e. the largest simplical complex with 1-skeleton :math:`N(P)_r`.

Given a finite metric space :math:`P` and any function :math:`\gamma:P\to \mathbb R`, we define the *function-Rips* bifiltration :math:`FR(\gamma)` as follows:

.. math::
   :nowrap: 

   \[FR(\gamma)_{a,b}:=R(\gamma^{-1}(-\infty,a])_b.\] 


:math:`FR(\gamma)` is always 1-critical.

We mention three natural choices of :math:`\gamma`, each of which is implemented in RIVET:

* A **ball density function**, defined by 

.. math::
   :nowrap:

   \[\gamma(x)=C[\# \text{ points in } P \text{ within distance }r \text{ of }x],\]

where :math:`r>0` is a fixed parameter, the "radius", and :math:`C` is a normalization constant, chosen so that :math:`\sum_{x\in P} \gamma(x)=1`.  

* A **Gaussian density function**, given by 

.. math::
   :nowrap:

   \[\gamma(x)=C\sum_{y\in P} e^{\frac{-d(x,y)^2}{2\sigma}},\]

where :math:`\sigma>0` is a parameter, the "standard deviation," and :math:`C` is a normalization constant.

* An **eccentricity function**, i.e.,

.. math::
   :nowrap:

   \[\gamma(x):= \left(\frac{\sum_{y\in P} d(x,y)^q}{|P|}\right)^{\frac{1}{q}},\]
where :math:`q\in [1,\infty)` is a parameter.

 

.. _degreeRipsBifil:

Degree-Rips Bifiltration
^^^^^^^^^^^^^^^^^^^^^^^^

For :math:`d\in \mathbb R`, let :math:`N(P)_{d,r}` be the subgraph of :math:`N(P)_r` obtained by removing all vertices of degree less than :math:`d`.  We define the *degree-Rips bifiltration*  :math:`DR(P)` by taking :math:`DR(P)_{d,r}:=N(P)_{d,r}.`  (Note that this is in fact a bifiltration indexed by :math:`\mathbb R^{\mathrm{op}}\times \mathbb R`, where :math:`\mathbb R^{\mathrm{op}}` denotes the opposite poset of :math:`\mathbb R`; that is, :math:`DR(P)_{a,b}\subset DR(P)_{a',b'}` whenever :math:`a\geq a'` and :math:`b\leq b’`.). If :math:`P` has more than one point, then :math:`DR(P)` is multi-critical.

Bipersistence Modules 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Let us fix a field :math:`K`.  A *bipersistence module* (also called a 2-D persistence module or 2-parameter persistence module in the literature) :math:`M` is a diagram of :math:`K`-vector spaces indexed by :math:`\mathbb R^2`.  That is, :math:`M` is a collection of vector spaces :math:`\{M_a\}_{a\in \mathbb{R^2}}`, together with a collection of linear maps 

.. math::
   :nowrap: 

   \[\{M_{a,b}:M_a\to M_b\}_{a\leq b}\] 

such that :math:`M_{a,a}=\mathrm{Id}_{M_a}` and :math:`M_{b,c}\circ M_{a,b}=M_{a,c}` for all :math:`a \leq b\leq c`.

A *morphism* :math:`f:M\to N` of bipersistence modules is a collection of maps

.. math::
   :nowrap: 
   
   \[\{f_a:M_a\to N_a\}_{a\in \mathbb R^2}\]


such that 

.. math::
   :nowrap: 

   \[f_b\circ M_{a,b}= N_{a,b} \circ f_a\] 

for all :math:`a\leq b\in \mathbb R^2`.  This definition of morphism gives the bipersistence modules the structure of an abelian category; thanks in part to this, many usual constructions for modules from abstract algebra have analogues for bipersistence modules.  In particular, direct sums and quotients are well defined.  

Free Persistence Modules
^^^^^^^^^^^^^^^^^^^^^^^^
For :math:`c \in \mathbb R^2`, define the bipersistence module :math:`\mathcal I^c` by

.. math::
   :nowrap: 

   \[\mathcal I^c_a=
   \begin{cases}
   K &\mathrm{if }\ a\geq c,\\ 0 & \mathrm{otherwise.}
   \end{cases}
   \qquad
   \mathcal I^c_{a,b}=
   \begin{cases}
   \mathrm{Id}_K &\mathrm{if }\ a\geq c,\\ 0 & \mathrm{otherwise.}
   \end{cases}\]

Note that the support of  :math:`\mathcal I^a` is the closed upper quadrant in :math:`\mathbb R^2` with lower left corner at :math:`a`.

A *free bipersistence module* is one isomorphic to :math:`\displaystyle\oplus_{c\in \mathcal B}\ \mathcal I^c` for some multiset :math:`\mathcal B` of points in :math:`\mathbb R^2`.  
There is a natural definition of basis for free modules, generalizing the definition of bases for vector spaces in linear algebra.  In close analogy with linear algebra, a morphism :math:`f:M\to N` of finitely generated free modules can be represented by a matrix, with respect to a choice of ordered bases for :math:`M` and :math:`N`.  Thus, to encode the isomorphism type of :math:`f`, it enough to store a matrix, together with a bigrade label for each row and each column of the matrix; the labels specify :math:`M` and :math:`N` up to isomorphism.

Presentations
^^^^^^^^^^^^^
A *presentation* of a bipersistence module :math:`M` is a map :math:`f:F\to G` such that :math:`M\cong G/\mathrm{im}\ f`.  We say :math:`M` is finitely presented if :math:`F` and :math:`G` can be chosen to be finitely generated.  If :math:`M` is finitely presented then there exists a presentation :math:`f:F\to G` for :math:`M` such that both :math:`F` and :math:`G` are minimial, i.e., for any other presentation :math:`f':F'\to G'`,  :math:`F` is a summand of :math:`F'` and :math:`G` is a summand of  :math:`G'`.  We call such a presentation *minimal*.  Minimal presentations are unique up to isomorphism, but importantly, their matrix representations are non-unique.

FIReps (Short Chain Complexes of Free Modules)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
We define a *FIRep* to be chain complex of free bipersistence modules of length 3.  Explicitly, then, an firep is a sequence of free bipersistence modules

.. math::
   :nowrap: 

   \[ C_2 \xrightarrow{f} C_1 \xrightarrow{g} C_0. \]

such that :math:`g\circ f=0`.  Associated to an firep is a unique homology module :math:`\ker g/\mathrm{im}\ f`.  A presentation of a bipersistence module can be thought of as a special case of an FIRep, where the last module is trivial.

Homology of a Bifiltration
^^^^^^^^^^^^^^^^^^^^^^^^^^
Applying :math:`i^{\mathrm{th}}` simplicial homology with coefficients in :math:`K` to each simplicial complex and each inclusion map in a bifiltration :math:`F` yields a bipersistence module :math:`H_i(F)`.  If :math:`F` is essentially finite, then :math:`H_i(F)` is finitely presented.


:math:`H_i(F)` is in fact the :math:`i^{\mathrm{th}}` homology module of a chain complex :math:`C(F)` of bipersistence modules whose value at each point in :math:`a\in \mathbb R^2` is the simplical chain complex of :math:`F_a`.  If :math:`F` is one-critical, each module of :math:`C(F)` is free.  In general, :math:`C(F)` needn’t be free, but given the portion of :math:`C(F)` at indexes :math:`i-1,` :math:`i`, and :math:`i+1`, it is easy to construct an FIRep whose homology is :math:`H_i(F)`; this is `an observation of Chacholski et al. <https://arxiv.org/abs/1409.7936>`_




Invariants of a Bipersistence Module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
As mentioned above, RIVET computes and visualizes three simple invariants of a bipersistence module :math:`M`:

* The *fibered barcode*, i.e., the function sending each affine line :math:`L\subset \mathbb R^2` with non-negative slope to the barcode :math:`\mathcal B(M^L)`, where :math:`M^L` denotes the restriction of :math:`M` along :math:`L`.
* The *Hilbert function*, i.e., the function :math:`\mathbb R^2\to \mathbb N` which sends :math:`a` to :math:`\dim M_a`.
* The *bigraded Betti numbers* :math:`\xi_i^M`. These are functions :math:`\mathbb{R}^2 \to \mathbb{N}` that, respectively, count the number of births, deaths, and "relations amongst deaths" at each bigrade. Formally, given :math:`r \in \mathbb{R}^2` and a minimal free resolution 

.. math::
   :nowrap: 

   \[0 \to F^2\to F^1\to F^0\]

for :math:`M`, :math:`\xi_i^M(r)` is the number of elements at bigrade :math:`r` in a basis for :math:`F^i`.

.. _coarsening:

Coarsening a Persistence Module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Given a finitely presented bipersistence module :math:`M`, we can *coarsen* :math:`M` to obtain an algebraically simpler module carrying approximately the same persistence information as :math:`M`.  As we will describe it here, the coarsening operation depends on a choice of finite grid :math:`G\subset\mathbb R^2`, such that :math:`G` contains some element ordered after all points in the support of the Betti numbers of :math:`M`.  The coarsened module, denoted :math:`M^G`, is defined by taking :math:`M^G_a:= M_g`, where :math:`g\in G` is the minimum grid element such that :math:`a\leq g`.  The internal maps of :math:`M^G` are induced by those of :math:`M` in the obvious way.

.. We can describe the coarsening operation succinctly in the language of category theory: Let :math:`G\subset\mathbb R^2` be a finite grid.  First, we take the restriction of :math:`M` along :math:`G`, and then take the left (or right) Kan extension of this along the inclusion of :math:`G\hookrightarrow \mathbb R^2`.  Currently, RIVET uses the right Kan extension.
