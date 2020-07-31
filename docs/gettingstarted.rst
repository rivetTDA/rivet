.. _gettingStarted:


Getting Started with RIVET
==========================


The RIVET software consists of two separate but closely related executables: **rivet_console**, a command-line program, and **rivet_GUI**, a GUI application.  **rivet_console** is the computational engine of RIVET; it implements the computation pipeline described in the previous section.   
**rivet_GUI** is responsible for RIVET’s visualizations and also provides a convenient graphical front-end to the functionality of **rivet_console**.  Thanks to this front-end, RIVET's visualizations can be carried out entirely from within **rivet_GUI**.  

For new users looking to acquaint themselves with RIVET, we recommend starting by using **rivet_GUI** to explore RIVET's visualization capabilities.  In this section, we provide a simple introduction to running RIVET via **rivet_GUI**.  Later sections of the documentation provide more detail on how to use **rivet_console** and **rivet_GUI**.  Users who wish to use RIVET for purposes other than visualization (e.g. machine learning or statistics applications) will want to familiarize themselves with the command-line syntax of **rivet_console**, but we recommend all users read this introduction first.


Getting started with **rivet_GUI**
----------------------------------
When the user runs **rivet_GUI**, the following window opens:

.. image:: images/file_input_dialog.png
   :width: 482px
   :height: 393px
   :alt: The file input dialog box of rivet_GUI
   :align: center

To start a computation, first select a file by clicking the “choose file” button.    **rivet_GUI** can handle files in several formats, representing several different types of input; these formats are discussed in detail in :ref:`inputData`.  In this first introduction, we consider just one simple type of input file, a CSV file specifying a point cloud in :math:`\mathbb{R}^n`. We call this type of file a “points” file. Each line of the file gives the :math:`n` coordinates of one point; these coordinates are written as numbers separated by commas or white space. 

For concreteness, we will use the file `data/Test_Point_Clouds/circle_300pts_nofunction.csv` from the RIVET repository. This file specifies 300 points in :math:`\mathbb{R}^2`. The first five lines of the file are as follows::

	1.57,2.40
	1.21,2.70
	0.79,-2.44
	-2.44,-2.24
	-2.54,-1.25

Note that the file does not specify the values of any function, such as a density estimator, on the points. RIVET is able to compute three common density estimators on a point cloud. Alternatively, a user may supply values of a function on each point, as described in :ref:`inputData`.

The 300 points in the file `data/Test_Point_Clouds/circle_300pts_nofunction.csv` form a noisy circle in :math:`\mathbb{R}^2`, as pictured below.  [*Also cut this, perhaps*: We will illustrate how the RIVET visualization detects the dense circle of points that is evident in this point cloud.]

.. image:: images/circle300_point_plot.png
   :width: 353px
   :height: 347px
   :alt: point cloud plot
   :align: center

Upon selecting a file, RIVET activates the input selectors in the **Options** panel of the dialog box. 
We must choose appropriate values for these input selectors.
In this example, we will set the values as shown in the following figure; each selector and value is discussed briefly below the figure.

.. image:: images/file_input_selections.png
   :width: 482px
   :height: 393px
   :alt: The file input dialog box with selected options
   :align: center

The *File Type* menu allows the user to tell RIVET how to interpret the input file. This is most important for CSV files, which may specify several different types of data. The default selection, *points*, is correct for the input file mentioned above. Other options include *points_fn*, which would be used if the file contained function values in addition to the coordinates of points. Alternately, a CSV file may specify a discrete metric space, with or without function values, corresponding to to the *metric* and *metric_fn* menu items. RIVET can also accept *bifiltration* and free implicit representation (*firep*) input; these input types are not given as CSV files but have their own specifications as described in :ref:`inputData`.

The *Homology Degree* selector allows the user to choose which degree of homology RIVET will compute. Currently, RIVET computes only a single degree of homology. A user who wishes to examine homology in multiple degrees, such as :math:`H_0` and :math:`H_1` homology, will need to run multiple RIVET computations on the same input data. Since we want to discern a central hole surrounded by a circle of points, we select homology degree 1.

The *Max Distance* selector allows the user to specify the maximum length of edges that RIVET will include in the simplicial complex that it constructs from the input data. This is useful to reduce the size of the simplicial complex, which allows the RIVET computation to run faster and with less memory. Choosing an appropriate maximum distance requires knowing something about the scale of the data. We choose a max distance of 5 for our example. The max distance can be set to infinity, which includes an edge connecting every pair of points in the point cloud, by typing “inf” or clicking on the button with an infinity symbol.

Three input selectors on the right side of the box determine what filtration RIVET will build from the point cloud. The **Filtration** selector contains two options: *degree* and *function*. The *degree* option builds a degree-Rips filtration, as described in :ref:`degreeRipsBifil`. Here, we choose the *function* option to build a function-Rips filtration.

The function-Rips filtration depends on the choice of a real-valued function on the point cloud, which is specified in the **Function** selector. In this selector, a choice of *user* selects user-provided function values; since our input file does not contain such values, we must choose a different option. The other three options cause RIVET to compute density estimators on the points; these are explained in [SECTION REFERENCE]. For the present example, we choose the “balldensity” option. 

The density estimators each require the choice of a parameter, which must be provided in the **Parameter** selector. The “Parameter” label changes, depending on the selected function, to provide additional context. Specifically, the ball density estimator requires the specification of a radius. RIVET computes the number of neighbors within this radius for each point in the point cloud. Here, we choose a radius of 2. 

The selectors in the lower portion of the **Options** box deal with the axes. The user may specify the number of **Bins**, which are used to coarsen the bipersistence module. The bin values limit the number of distinct grades that occur in the module, as described in :ref:`coarsening`. Specifying smaller bin values will speed the RIVET computation, but will result in less precise output. For the present example, we set both bin values to 30. 

Next, the user may specify the labels for each axis in the RIVET visualization. For a function-Rips filtration, RIVET presents the function values along the x-axis. Since we are computing a density estimator, we enter “density” for the x-axis label. We keep the default “distance” label for the y-axis.

Lastly, the **Reverse** checkboxes allow the user to reverse axis directions. For example, when using a density estimator, we typically want points with larger density values to enter the filtration before points with smaller density values; thus, we check the **Reverse** box for the x-axis. It is not possible to reverse the distance axis for a Rips filtration, so the y-axis reverse box is unavailable.

We now click **Compute**. This starts the RIVET computational pipeline, as described in :ref:`structure`. A progress box appears, as shown below.

.. image:: images/RIVET_progress_box.png
   :width: 302px
   :height: 187px
   :alt: The RIVET computation progress box
   :align: center


Key Features of the RIVET Visualization
---------------------------------------

When the computation finishes, RIVET displays the following visualization.
This page gives a brief overview of the visualization elements; much more detail is found in :ref:`visualization`.

The RIVET visualization contains two main windows, the *Line Selection Window* and the *Persistence Diagram Window*, shown in the screenshot below.

.. image:: images/RIVET_screenshot_circle300_balldensity.png
   :width: 600px
   :height: 449px
   :alt: The file input dialog box with selected options
   :align: center


Line Selection Window
^^^^^^^^^^^^^^^^^^^^^

The *Line Selection Window* not only visualizes the Hilbert function values and the bigraded Betti numbers of a bipersistence module, but also allows the user choose linear slices along which barcodes are displayed. 
The viewable region is chosen as described in :ref:`visualization`, and can be adjusted using the controls at the bottom of the window.

The Hilbert function values are shown as grayscale shading, and points in the supports of :math:`\xi_0^M`, :math:`\xi_1^M`, and :math:`\xi_2^M` are marked with green, red, and yellow dots, respectively. 
The dots are translucent, so overlapping dots may be discerned, and the area of each dot is proportional to the corresponding function value. 
Hovering the mouse over a pixel in the window gives a popup box with the value of the Hilbert function or the bigraded Betti numbers at that point.

A key feature of the RIVET visualization is the ability to interactively select the line :math:`L` via the mouse and have the barcode :math:`\mathcal B(M^L)` update in real time.
The *Line Selection Window* contains a blue line :math:`L` of non-negative slope, with endpoints on the boundary of the displayed region of :math:`\mathbb{R}^2`. 
RIVET displays a barcode for :math:`M^L` in the line selection window, provided the "show barcode" box is checked below. 
The intervals in the barcode for :math:`M^L` are displayed in purple, perpendicularly offset from the line :math:`L`.
Click and drag the blue line with the mouse to change the choice of line :math:`L`; for details, see :ref:`visualization`.
As the line moves, both the barcode in the Line Selection Window and its persistence diagram representation in the Persistence Diagram Window are updated in real time. 


Persistence Diagram Window
^^^^^^^^^^^^^^^^^^^^^^^^^^

The Persistence Diagram Window displays a persistence diagram representation of the barcode for :math:`M^L`.
The multiplicity of a point in the persistence diagram is indicated by the area of the corresponding dot. 
Additionally, hovering the mouse over a dot produces a popup that displays the multiplicity of the dot.

The bounds for the square viewable region (surrounded by dashed lines) in this window are chosen automatically. 
They depend on the bounds of the viewable region in the slice diagram window, but not on :math:`L`.
Some points in the persistence diagram may have coordinates that fall outside of the viewable region. 
These points are indicated by dots or numbers along the left and top edges of the persistence diagram.
For details, see :ref:`visualization`.


Customizing the Visualization
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The look of the visualization can be customized by choosing *RIVET → Preferences* on Mac, or *Edit → Configure* on Linux, and adjusting the settings there.  

