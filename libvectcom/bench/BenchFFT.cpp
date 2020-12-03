#include <vectcom/Configuration.h>

#include <vectcom/PolyCrypto.h>
#include <vectcom/PolyOps.h>
#include <vectcom/NtlLib.h>

#include <xassert/XAssert.h>
#include <xutils/Log.h>
#include <xutils/Timer.h>

using namespace libvectcom;

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    libvectcom::initialize(nullptr, 0);

    size_t count = 10;
    if(argc < 3) {
        cout << "Usage: " << argv[0] << " <deg> <n> [<count>]" << endl;
        cout << endl;
        cout << "Measures an FFT of size <n> on poly of degree <deg> <count> times." << endl;
        return 1;
    }

    size_t d = static_cast<size_t>(std::stoi(argv[1]));
    size_t n = static_cast<size_t>(std::stoi(argv[2]));
    if(argc > 3)
        count = static_cast<size_t>(std::stoi(argv[3]));

    AveragingTimer 
        df("FFT");

    // Step 0: Pick random polynomial
    vector<Fr> p = random_field_elems(d+1), vals;

    for(size_t i = 0; i < count; i++) {
        loginfo << "d = " << d << ", n = " << n << endl;

        // Step 1: Do an FFT
        vector<Fr> q, r, rhs;
        df.startLap();
        poly_fft(p, n, vals);
        df.endLap();
    }

    logperf << df << endl;

    return 0;
}
