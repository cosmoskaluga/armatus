armatus
=======

Installation
------------

Dependencies:

* C++11
* Boost (Graph, Test, ublas, program options, system)

Note: Although this should be conceptually easy, properly installing these dependencies can be tricky since on a system like OS X you can choose to use different compilers and standard libraries.  Please make sure that boost is installed with the same standard libary and compiler that you are using to compile Armatus. For this reason, binaries of Armatus will also be released.

Here is an example of how to compile boost with Clang++ and use Clang++ to compile Armatus on OS X:

Boost-specific details:

    ./bootstrap --prefix=$HOME/boost
    ./b2 clean
    ./b2 install toolset=clang cxxflags="-stdlib=libc++" linkflags="-stdlib=libc++"

Using CMake to compile Armatus:

    cmake -DCMAKE_CXX_COMPILER=clang++ -DBOOST_ROOT=$HOME/boost -DBoost_NO_SYSTEM_PATHS=true ..
    make
    export DYLD_FALLBACK_LIBRARY_PATH=$HOME/boost/lib

Make sure you substitute `$HOME/boost` with the installation path you desire.

Example Run
-----------

The main inputs into Armatus are the matrix file (in the format of Dixon et al.: http://chromosome.sdsc.edu/mouse/hi-c/download.html) and the gammaMax parameter which determines the highest resolution at which domains are to be generated.

An example run on chromosome 1 of a human fibroblast:

    time armatus -i IMR90/40kb/combined/chr1.nij.comb.40kb.matrix.gz -g .5 -o test -m

    Multiresoultion ensemble will be written to files
    Reading input from IMR90/40kb/combined/chr1.nij.comb.40kb.matrix.gz.
    chr1 at resolution 40000bp
    line 1000
    line 2000
    line 3000
    line 4000
    line 5000
    line 6000
    MatrixParser read matrix of size: 6182 x 6182
    gamma=0
    gamma=0.05
    gamma=0.1
    gamma=0.15
    gamma=0.2
    gamma=0.25
    gamma=0.3
    gamma=0.35
    gamma=0.4
    gamma=0.45
    gamma=0.5
    Writing consensus domains to: test.consensus.txt
    Writing multiscale domains

    real    1m35.923s
    user    1m34.461s
    sys 0m1.401s

The first `-i` parameter is a gzipped HiC matrix for chromosome 1 as obtained from Dixon et al. the second `-g` parameter is the maximum gamma at which to sample at.  Output files are all written with a prefix `test`.

Other options allow for sampling multiple near-optimal solutions and considering finer levels of step sizes. These are 'idealized' parameters in the sense that ideally we would sample as many resolutions as possible and consider the most solutions that are reasonably close to the optimal solution.

Components
----------

* armatus executable: (Boost Program Options): Armatus.cpp

ArmatusUtil.{cpp,hpp}:

* parseMatrix(File): 3C matrix parser (Dixon et al. format)
* outputDomains(set of domains)
* consensusDomains(set of domains)
* multiscaleDomains(gammaMax, stepSize, k)

Data structures {cpp,hpp}:

* ArmatusParams(Matrix, gamma): parameters to DP and pre-computable quantities such as the quality function
* ArmatusDAG(Params): encodes structure of dynamic program
    * BackPointers and SubProblem classes: see below
    * build(): Build the DAG for the dynamic program (Boost Graph)
    * computeTopK(k): At every node, store 'SubProblem': k 3-tuples: (edge, child solution, score of kth best solution for this subproblem)
    * extractTopK(k): Returns a set of domains
* IntervalScheduling
    * WeighedInterval
    * IntervalScheduler
        * previousDisjointInterval()
        * computeScheduling()
        * extractIntervals()

Testing:

* Boost test

Pipeline:

* Code and data links to generate results in application note