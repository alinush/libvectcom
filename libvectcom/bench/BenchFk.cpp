#include <vectcom/Configuration.h>

#include <vectcom/PolyCrypto.h>
#include <vectcom/PolyOps.h>
#include <vectcom/Utils.h>
#include <vectcom/KZG.h>
#include <vectcom/FFT.h>
#include <vectcom/KatePublicParameters.h>

#include <cmath>
#include <ctime>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>

#include <xassert/XAssert.h>
#include <xutils/Log.h>
#include <xutils/Utils.h>
#include <xutils/NotImplementedException.h>
#include <xutils/Timer.h>

#include <libff/algebra/fields/field_utils.hpp>

using namespace std;
using namespace libfqfft;
using namespace libvectcom;

int main(int argc, char *argv[]) {
    libvectcom::initialize(nullptr, 0);
    srand(static_cast<unsigned int>(time(NULL)));
    
    if(argc < 6) {
        cout << "Usage: " << argv[0] << "<deg> <n> <num-samples> <out-file>" << endl;
        cout << endl;
        cout << "OPTIONS: " << endl;
        cout << "   <pp-file>       the Kate public parameters file" << endl;
        cout << "   <deg>           the degree of the evaluated polynomial (i.e., t-1)" << endl;
        cout << "   <n>             the # of points to construct proofs for" << endl;  
        cout << "   <num-samples>   the # of times to repeat the benchmark" << endl;
        cout << "   <out-file>      output CSV file to write results in" << endl;
        cout << endl;

        return 1;
    }
    
    std::string ppFile = argv[1];
    size_t deg = static_cast<size_t>(std::stoi(argv[2]));
    size_t n = static_cast<size_t>(std::stoi(argv[3]));
    size_t r = static_cast<size_t>(std::stoi(argv[4]));
    std::string fileName = argv[5];
    size_t N = Utils::smallestPowerOfTwoAbove(n);

    bool exists = Utils::fileExists(fileName);
    ofstream fout(fileName, std::ofstream::out | std::ofstream::app);

    if(fout.fail()) {
        throw std::runtime_error("Could not open " + fileName + " for writing");
    }
   
    if(!exists)
        fout << "n,scheme,num_samples,h_usec,dft_usec,total_usec,h_hum,dft_hum,total_hum,date" << endl;

    // TODO: what happens if deg >= n?
    if (deg >= n) {
        logerror << "Uhm, not sure if this case is handled yet." << endl;
        return 1;
    }

    std::unique_ptr<Dkg::KatePublicParameters> kpp(
        new Dkg::KatePublicParameters(ppFile, deg));

    loginfo << endl;
    loginfo << "Degree " << deg << " poly, evaluated at n = " << n << " points, iters = " << r << endl;

    AveragingTimer ht("Computing h[i]'s");
    AveragingTimer ft("FFT on h[i]'s");

    for(size_t i = 0; i < r; i++) {
        std::vector<Fr> f = random_field_elems(deg + 1);
        std::chrono::microseconds::rep mus;
        std::vector<G1> H;

        ht.startLap();
        H = kpp->computeAllHis(f);
        mus = ht.endLap();
        
        logperf << " - Computing h[i]'s (iter " << i << "): " << Utils::humanizeMicroseconds(mus, 2) << endl;

        // resize H to size n, since the degree of f < n
        H.resize(N, G1::zero());
        
        ft.startLap();
        FFT<G1, Fr>(H);
        mus = ft.endLap();
        
        logperf << " - FFT on h[i]'s (iter " << i << "): " << Utils::humanizeMicroseconds(mus, 2) << endl;
    }

    logperf << endl;
    logperf << ht << endl;
    logperf << ft << endl;
    auto at = ht.averageLapTime() + ft.averageLapTime();
    auto ht_hum = Utils::humanizeMicroseconds(ht.averageLapTime(), 2);
    auto ft_hum = Utils::humanizeMicroseconds(ft.averageLapTime(), 2);
    auto at_hum = Utils::humanizeMicroseconds(at, 2);
    logperf << "Overall time: " << at_hum << endl;
    logperf << endl;

    fout 
        << n << ","
        << "fk" << ","
        << r << ","
        << ht.averageLapTime() << ","
        << ft.averageLapTime() << ","
        << at << ","
        << ht_hum << ","
        << ft_hum << ","
        << at_hum << ","
        << timeToString()
        << endl; 

    loginfo << "Exited successfully!" << endl;

    return 0;
}
