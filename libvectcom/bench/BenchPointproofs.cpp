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
#include <libfqfft/evaluation_domain/domains/basic_radix2_domain.hpp>

using namespace std;
using namespace libfqfft;
using namespace libvectcom;

// Takes the KZG public parameters as input (which we pretend to be PDH public parms) and
// outputs the circulant matrix's vector representation, which is used to compute all proofs 
// fast.
std::vector<G1> publicParamsToCirculant(const KatePublicParameters& kpp, size_t N) {
    std::vector<G1> c;

    // c = [ g_1^0 ];
    c.push_back(G1::zero());
    //loginfo << "g_1^0" << endl;
    
    // c = [ g_1^0, g_1^N, \dots, g_1^2 ];
    for(size_t i = 1; i <= N-1; i++) {
        //loginfo << "N - " << (i-1) << endl;
        c.push_back(kpp.g1si[N - (i-1)]);
    }

    // c = [ g_1^0, g_1^N, \dots, g_1^2, g_1^0];
    c.push_back(G1::zero());
    //loginfo << "g_1^0" << endl;

    // c = [ g_1^0, g_1^N, \dots, g_1^2, g_1^0, g_1^{2N}, \dots, g_1^{N+2}];
    for(size_t i = 1; i <= N-1; i++) {
        //loginfo << "2N - " << (i-1) << endl;
        c.push_back(kpp.g1si[2*N - (i-1)]);
    }

    testAssertEqual(c.size(), 2*N);

    return c;
}

// NOTE: We pass these by values since we need copies of them
std::vector<G1> circulantMultiply(const std::vector<G1>& cn, std::vector<Fr> x) {
    assertEqual(cn.size(), x.size()); 
    size_t N = x.size();
    // TODO: generalize like in FK, but don't need this for now
    assertTrue(Utils::isPowerOfTwo(N));

    // Do an FFT on the field elements in the vector x
    Fr omega = libff::get_root_of_unity<Fr>(N);
    // TODO: libfqfft supports other fft's of different sizes too
    libfqfft::_basic_serial_radix2_FFT(x, omega);

    // Do an FFT on the group elements in the matrix representation of C_N stored in cn
    std::vector<G1> y = cn;
    FFT<G1, Fr>(y);

    // Do a Hadamard product
    for(size_t i = 0; i < N; i++) {
        y[i] = x[i] * y[i];
    }
    
    // Inverse FFT of the result
    invFFT<G1, Fr>(y);

    return y;
}

std::vector<G1> pointproofsProveAll(const KatePublicParameters& kpp, const std::vector<Fr>& m) {
    // m' = m with N extra zeros
    std::vector<Fr> mprime = m;
    size_t N = m.size();
    testAssertEqual(mprime.size(), N);
    testAssertEqual(mprime, m);

    for(size_t i = 0; i < N; i++)
        mprime.push_back(Fr::zero());
    testAssertEqual(mprime.size(), 2*N);

    std::vector<G1> c2n = publicParamsToCirculant(kpp, N);
    testAssertEqual(c2n.size(), 2*N);

    std::vector<G1> piprime = circulantMultiply(c2n, mprime);
    piprime.resize(N);

    return piprime;
}

G1 pointproofsCommit(const KatePublicParameters& kpp, const std::vector<Fr>& m) {
    G1 c = G1::zero(); // zero() is the group's identity (in libff's additive notation)
    size_t N = m.size();

    for(size_t j = 1; j <= N; j++) {
        G1 base = kpp.g1si.at(j);
        c = c + (m.at(j-1) * base);
    }
    return c;
}

G1 pointproofsProveOne(const KatePublicParameters& kpp, size_t i, const std::vector<Fr>& m) {
    G1 pi = G1::zero(); // zero() is the group's identity (in libff's additive notation)
    size_t power;
    size_t N = m.size();

    for(size_t j = 1; j <= N; j++) {
        if (j != i) {
            power = j + (N+1) - i; 
            G1 base = kpp.g1si.at(power);

            pi = pi + (m.at(j-1) * base);
        }
    }
    return pi;
}

bool pointproofsVerifyOne(const KatePublicParameters& kpp, const G1& comm, size_t i, const Fr& m_i, const G1& pi, size_t N) {
    // Cheat and compute g_T^{\alpha^{n+1}}
    auto gtn1 = ReducedPairing(kpp.g1si[N + 1], kpp.g2si[0]);

    // verify proof
    return ReducedPairing(comm, kpp.g2si[(N + 1) - i]) == (ReducedPairing(pi, kpp.g2si[0]) * (gtn1^m_i));
}

int main(int argc, char *argv[]) {
    libvectcom::initialize(nullptr, 0);
    srand(static_cast<unsigned int>(time(NULL)));
    
    if(argc < 5) {
        cout << "Usage: " << argv[0] << "<deg> <n> <num-samples> <out-file> [<naive>]" << endl;
        cout << endl;
        cout << "OPTIONS: " << endl;
        cout << "   <pp-file>       the Kate public parameters file" << endl;
        cout << "   <n>             the # of points to construct proofs for" << endl;  
        cout << "   <num-samples>   the # of times to repeat the benchmark" << endl;
        cout << "   <out-file>      output CSV file to write results in" << endl;
        cout << "   <naive>         1 if should compute proofs naively (default: 0)" << endl;
        cout << endl;

        return 1;
    }
    
    std::string ppFile = argv[1];
    size_t n = static_cast<size_t>(std::stoi(argv[2]));
    size_t r = static_cast<size_t>(std::stoi(argv[3]));
    std::string fileName = argv[4];
    bool naively = false;
    if (argc > 5 && std::stoi(argv[5]) == 1)
        naively = true;
    size_t N = Utils::smallestPowerOfTwoAbove(n);
    size_t deg = n*2;

    if (N != n) {
        logerror << "For now, sticking to powers of two for <n>. You gave n = " << n << "." << endl;
        return 1;
    }
    
    bool exists = Utils::fileExists(fileName);
    ofstream fout(fileName, std::ofstream::out | std::ofstream::app);

    if(fout.fail()) {
        throw std::runtime_error("Could not open " + fileName + " for writing");
    }
    
    if(!exists)
        fout << "n,scheme,num_samples,total_usec,total_hum,date" << endl;

    std::unique_ptr<Dkg::KatePublicParameters> kpp(
        new Dkg::KatePublicParameters(ppFile, deg));

    if (kpp->g1si.size() > 0 && n > (kpp->g1si.size() - 1) / 2) {
        logerror << "Need 2N public parameters for vectors of size N" << endl;
        return 1;
    }

    loginfo << endl;
    loginfo << "Computing all n = " << n << " Pointproofs, naive = " << naively << ", iters = " << r << endl;
    
    AveragingTimer t("Prove all");
    for(size_t iter = 0; iter < r; iter++) { 
        // f is the vector being committed to
        std::vector<Fr> m = random_field_elems(n);
        auto c = pointproofsCommit(*kpp, m);
        //loginfo << "Commitment: " << c << endl;

        std::vector<G1> pi;
        t.startLap();
        if (naively) {
            for(size_t i = 1; i <= N; i++) {
                pi.push_back(pointproofsProveOne(*kpp, i, m));
            }
        } else {
            pi = pointproofsProveAll(*kpp, m);
        }
        t.endLap();

        // sanity check proofs
        for(size_t i = 1; i <= N; i++) {
            auto pi_i = pi.at(i-1);

            if(!pointproofsVerifyOne(*kpp, c, i, m.at(i-1), pi_i, N)) {
                logerror << "Proof \\pi_" << i << " = " << pi_i << " for m_" << i << " did NOT verify!" << endl;
                return 1;
            } else {
                //loginfo << "Proof \\pi_" << i << " = " << pi_i << " for m_" << i << " = " << m.at(i-1) << " verified!" << endl;
            }
        }
        loginfo << "All proofs verified!" << endl;
    }

    logperf << t << endl;

    fout 
        << n << ","
        << "pointproofs-"
        << (naively ? "naive" : "fast") << ","
        << r << ","
        << t.averageLapTime() << ","
        << Utils::humanizeMicroseconds(t.averageLapTime(), 2) << ","
        << timeToString()
        << endl; 

    loginfo << "Exited successfully!" << endl;

    return 0;
}
