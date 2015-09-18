[![Build Status](https://travis-ci.org/kingsfordgroup/armatus.svg?branch=master)](https://travis-ci.org/kingsfordgroup/armatus)

armatus
=======

<img src="logo/ArmatusLogo.png", width=300/>

Multiresolution domain calling software for chromosome conformation capture interaction matrices.

Current release: Armatus 2.1
------------

The latest version of Armatus now supports parsing high resolution matrices generated by Rao et al. at a 5kbp resolution.  An example of how to run it on their data in a shell script:

    ARMATUS=armatus/build/src/armatus
    for i in `seq 1 22`
    do
        INPUT=GM12878_combined/5kb_resolution_intrachromosomal/chr$i/MAPQGE30/chr${i}_5kb
        time ./$ARMATUS -m -r 5000 -R -c $i -i $INPUT -g 1.0 -s 0.05 -o raodomains/chr$i
    done

This version will run Armatus in serial on all 22 chromosomes on a cell type for the Rao et al. data set.  The -m option outputs the domains at different resolutions, the -r option specifies the resolution, the -R option uses the parser for the Rao et al. data.  It will generate domains between gamma=0.0 to 1.0 at steps of 0.05.

Also note that the Dixon et al. format for the matrices on their web site has changed since the original release of Armatus.  Specifically, its first three columns were chromosome number, fragment start, and fragment end.  We have now changed our parser to accept their updated format: i.e. there are not three columns preceeding the actual matrix data and you must feed in the resolution as a parameter to Armatus.

The current version uses dense matrices instead of sparse matrices since populating sparse matrices at high resolutions increases the run time substantially. As an example, chromosome 10 of Rao et al. currently consumes approximately 8GB of memory and each resolution completes in approximately 2-3 minutes on a single thread using an Intel(R) Xeon(R) CPU E5-2690 v2 @ 3.00GHz.

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

Here is one-liner to compile Armatus with G++ given you have the proper dependencies:

     g++ -std=c++11 -w -lboost_system -lboost_iostreams -lboost_program_options -L /opt/local/lib/ -I include/ -I /opt/local/include/ -O3 -o binaries/armatus-linux-x64 src/*.cpp 

Note that this assumes you have Boost installed in "/opt/local".

WARNING
-------

We have noticed that occassionally, the *order* of the arguments passed can result in a memory bug that is yet unexplained since we are using the Boost argument parser.  We are working to resolve this issue.  If you experience this issue, place the "-m" argument before the "-k" argument.

Example Run
-----------

The main inputs into Armatus are the matrix file (in the format of Dixon et al.: http://chromosome.sdsc.edu/mouse/hi-c/download.html) and the gammaMax parameter which determines the highest resolution at which domains are to be generated.  *Note*: we recently noticed that the format of the matrices of Dixon et al. in the link above has changed.
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

Other options allow for sampling multiple near-optimal solutions and considering finer levels of step sizes. These are 'idealized' parameters in the sense that ideally we would sample as many resolutions as possible and consider all solutions that are reasonably close to the optimal solution.

We have also added a simple example in the "examples/" directory for your convenience.


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
