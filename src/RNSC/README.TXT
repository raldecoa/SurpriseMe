PARAMETER GUIDELINES FOR rnsc, rnscconvert, and rnscfilter

%%%%%%%%%%%
RNSC
%%%%%%%%%%%

	rnsc -g GraphFilename
	     [-o OutputFilename]
	     [-i InputFilename]
	     [-c MaxNumberOfClusters]
	     [-t TabuLength]
	     [-n NaiveStoppingTolerance]
	     [-N ScaledStoppingTolerance]
	     [-e NumberOfExperiments]
	     [-D DiversificationFrequency]
	     [-d DiversificationLength]
	     [-s]


-g GraphFilename
	Required.  Sets the filename of the input graph.
	
-o OutputFilename
	Sets the output filename for the RNSC clustering.  Default is out.rnsc.
	
-i InputFilename
	If defined, sets the input filename for an RNSC clustering to use as the algorithm's
	initial clustering.  The initial clustering is random by default.
	
-c MaxNumberOfClusters
	Integer.  Sets the maximum number of clusters to be used by the algorithm.  Useful in
	forcing larger clusters when clustering a sparse graph.  Default is the number of
	vertices in the graph.
	
-t TabuLength
	Integer.  Sets the length of the tabu list.  The tabu list is a first-in-first-out list
	of recently moved vertices; a vertex will not be moved if it is in the tabu list.  n/50
	is generally an appropriate value, where n is the number of vertices in the graph.
	
-n NaiveStoppingTolerance
	Integer.  Sets the naive stopping tolerance.  When RNSC makes this many moves in the
	naive scheme without improving its optimal clustering, it will quit the naive scheme
	and move on to the scaled scheme.
	
-N ScaledStoppingTolerance
	Integer.  Sets the scaled stopping tolerance.  When RNSC makes this many moves in the
	scaled scheme without improving its optimal clustering, it will quit the scaled scheme
	and terminate the experiment.
	
-e NumberOfExperiments
	Integer.  Sets the number of experiments (i.e. runs) that RNSC will perform.  Since
	RNSC is a stochastic algorithm, more experiments will generally lead to better optimal
	solutions.  The default value is 1, but this can be improved to the user's taste and
	time constraints.
	
-D DiversificationFrequency
	Integer.  Sets the shuffling diversification period, i.e. how often the clustering is
	"shuffled", in moves.  We recommend a value of about 100.
	
-d DiversificationLength
	Integer.  Sets the number of random moves made in a diversification phase.  One tenth
	the diversification period is generally an appropriate value.
	
-s 
	Setting this flag skips the naive scheme altogether.  This should be used only if an
	input clustering is specified.
	


%%%%%%%%%%%
RNSCCONVERT
%%%%%%%%%%%

	rnscconvert -g OutputGraphFilename
	            -i InputEdgelistFilename
	            -n OutputNameFilename

These parameters specify the input/output filenames and are self-explanatory.




%%%%%%%%%%%
RNSCFILTER
%%%%%%%%%%% 


	rnscfilter -g GraphFilename
	           -c ClusteringFilename
	           [-n NameFilename]
	           [-o OutputFilename]
	           [-s SizeCutoff]
	           [-d DensityCutoff]
	           [-p PValueCutoff]


The input parameters for filter are simple and fall into two categories.  The
first type specifies input and output filenames.  The second type specifies the
size, density, and functional homogeneity P-values.  

The following two parameters are required:

-g filename	
	The graph filename specifies the RNSC input graph whose clustering we are analyzing.
	
-c filename	
	The clustering filename specifies the file where the RNSC output clustering is saved.


The remaining parameters are optional:
-n filename
	The name filename specifies the location of the file in which protein names and
	functions are stored.  If none is specified, filter will use the original RNSC numerical
	node labels and each protein will have function “U”, or unknown.
	
-o filename
	The output filename specifies the name of the file to which predicted complexes are
	written.  Default is output.txt.
	
-s integer
	The size cut-off is an integer; clusters smaller than this integer will not be output.
	Default value is 0.
	
-d float
  	The density cut-off is a floating point number (i.e., decimal).  Clusters with density
	less than this number will not be output.  Default value is 0.
	
-p float
  	The P-value cut-off is a floating point number.  It is possible to use a decimal
	(e.g. 0.001) or scientific notation (e.g., 1e 3).  Clusters with functional homogeneity
	P-value above this parameter will not be output.  Default is 1.
	
Note: If no name file is specified, every protein will have function “U” and therefore every
cluster will have P value 1, so this flag should not be used unless a name file is specified.

