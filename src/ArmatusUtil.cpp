/* 
	Authors: Darya Filippova, Geet Duggal, Rob Patro
	dfilippo | geet | robp @cs.cmu.edu
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <boost/range/irange.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/io.hpp>
#include "ArmatusUtil.hpp"
#include "ArmatusParams.hpp"
#include "IntervalScheduling.hpp"
#include "ArmatusParams.hpp"
#include "ArmatusDAG.hpp"

MatrixProperties parseGZipMatrix(string path) {
	MatrixProperties prop;

    prop.matrix = make_shared<SparseMatrix>();

	ifstream file(path, ios_base::in | ios_base::binary);
    assert(file.good());
    boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
    in.push(boost::iostreams::gzip_decompressor());
    in.push(file);

    string line;
    std::istream incoming(&in);
    bool firstLine = true;
    size_t i = 0;
    double tot = 0.0;
    size_t nedge = 0;
    while ( getline(incoming, line) ) {
    	vector<string> parts;
        boost::trim(line);
		boost::split(parts, line, boost::is_any_of("\t"));
        
        if (firstLine) { 
            prop.matrix->resize(parts.size() - 3, parts.size() - 3, false);
            prop.chrom = parts[0];
            prop.resolution = atoi(parts[2].c_str())-atoi(parts[1].c_str());
            cerr << prop.chrom << " at resolution " << prop.resolution << "bp" << endl;
            firstLine = false;
        }
        
        for (size_t j : boost::irange(3 + i, parts.size())) {
            double e  = stod(parts[j]);
            if (e > 0.0) {
                size_t row = j - 3;
                prop.matrix->push_back(i, row, e);
                tot += e;
                nedge++;
            }   
        }

    	++i;
        if ( i % 1000 == 0 ) { std::cerr << "line " << i << "\n"; }
        if (incoming.eof()) break;
    }
    //std::cerr << "Avg edge " << tot / nedge << " sum " << tot  << " cnt " << nedge << "\n";
    return prop;
	// SparseSymmetricMatrix symMat(m);
 //    std::cerr << "M is " << symMat.size1() << " x " << symMat.size2() << ", with " << m.nnz() << " non-zero entries\n";
}

Domain::Domain(size_t s, size_t e) : start(s), end(e) { }

double Domain::score(ArmatusParams& p) {
    size_t d = end-start+1;
    return std::max((p.sums(start, end)/ std::pow(static_cast<double>(d),p.gamma)) - p.mu[d], 0.0);
}

DomainSet consensusDomains(WeightedDomainEnsemble& dEnsemble) {
    using PersistenceMap = map<Domain, double>;
    PersistenceMap pmap;

    for (auto dSetIdx : boost::irange(size_t{0}, dEnsemble.domainSets.size())) {
        auto& dSet = dEnsemble.domainSets[dSetIdx];
        auto weight = dEnsemble.weights[dSetIdx];
        for (auto& domain : dSet) {
            if ( pmap.find(domain) == pmap.end() ) pmap[domain] = 0;
            pmap[domain] += weight;
        }
    }

    Intervals ivals;

    for (auto domainPersistence : pmap) {
        auto domain = domainPersistence.first;
        auto persistence = domainPersistence.second;
        ivals.push_back(WeightedInterval(domain.start, domain.end, persistence));      
    }

    IntervalScheduler scheduler(ivals);
    scheduler.computeSchedule();

    DomainSet dSet;
    for (auto ival : scheduler.extractIntervals()) {
        dSet.push_back(Domain(ival.start, ival.end));
    }

    sort(dSet.begin(), dSet.end());

    return dSet;
}

WeightedDomainEnsemble multiscaleDomains(std::shared_ptr<SparseMatrix> A, float gammaMax, double stepSize, int k) {

    WeightedDomainEnsemble dEnsemble;
    double eps = 1e-5;

    for (double gamma=0; gamma <= gammaMax+eps; gamma+=stepSize) {

        cerr << "gamma=" << gamma << endl;
 
        ArmatusParams params(A, gamma);
        ArmatusDAG G(params);
        G.build();
        G.computeTopK(k);
        //auto domains = G.viterbiPath();
        //dEnsemble.push_back(domains);

        auto domainEnsemble = G.extractTopK(k);
        auto& domains = domainEnsemble.domainSets;
        auto& weights = domainEnsemble.weights;
        dEnsemble.domainSets.insert(dEnsemble.domainSets.end(), domains.begin(), domains.end());
        dEnsemble.weights.insert(dEnsemble.weights.end(), weights.begin(), weights.end());
    }

    return dEnsemble;
}

void outputDomains(DomainSet dSet, string fname, MatrixProperties matProp) {
    ofstream file;
    file.open(fname);
    int res = matProp.resolution;
    for (auto d : dSet) {
        file << matProp.chrom << "\t" << (d.start+1)*res << "\t" << (d.end+1)*res << endl;
    }
    file.close();
}



