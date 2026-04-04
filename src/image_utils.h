#pragma once

#include <map>
#include <mutex>
#include "GLOBALS.h"
#include "ThreadPool.h"
#include <execution>     
#include <variant>


// Define an enum that the user picks
enum class DftExecPolicy {
    Seq,
    Par,
    ParUnseq,
    Unseq
};

// Define a variant that can store any of the execution policies
using PolicyVariant = std::variant<
    std::execution::sequenced_policy,
    std::execution::parallel_policy,
    std::execution::parallel_unsequenced_policy,
    std::execution::unsequenced_policy
>;

/// A small struct to keep state per "name"
struct DftState
{
	bool stateInit = false;
	std::vector<float> lastComputedResult;

    // Additional: store the previous input
    std::vector<float> lastInputData;
    int lastWidth  = 0;
    int lastHeight = 0;
    bool lastUseLog = false;

    bool skipInit = false;
    std::chrono::steady_clock::time_point skipStartTime; 
    double skipTimeoutSec = 0.0; // grows from 0 -> 2 in 0.5 increments
};

// Maps from name -> mutex for that name
extern std::map<string, std::unique_ptr<std::mutex>> s_mutexes;

// Maps from name -> state for that name
extern std::map<string, std::unique_ptr<DftState>> s_states;

std::vector<float> createHistogramPlot(const std::vector<float>& data,
                                     int outWidth,
                                     int outHeight,
                                     int numBins = 256);

std::vector<float> createPixelValuePlot(const std::vector<float>& data,
                                        int outWidth,
                                        int outHeight);

void forceDFTImageFullInit();

#if (USE_FFTW3)
std::vector<float> computeFFTMagnitude(const std::vector<float> &inputData,
                                       int width, int height,
                                       bool useLog = true);
#endif

void dftMagImageGenerator(
      const std::string& name
    , const std::vector<float>& inputData
	, int width
	, int height
    , size_t numTasks = std::thread::hardware_concurrency()
    , DftExecPolicy execPolicy = DftExecPolicy::Unseq
	, bool useLog = true    
    #ifdef ASYNC_TASKING
    , bool useAync = false
    #endif
);


struct composite_factor
{
    uint32_t factor;
    uint32_t power;

    composite_factor(uint32_t f, uint32_t p) : factor(f), power(p) {}
}; 

#define SMALLEST_PRIME 2
#define SMALLEST_COMPOSITE 4
#define BASE_FACTOR 1


class factorizationEngine
{
public:
    static std::vector<uint32_t> primes;

    //returns idx of max prime <= N
    static size_t establishPrimeRangeUpTo(uint32_t N);
};
std::vector<composite_factor> factorizeN(uint32_t N);

struct compositeNumberInformation
{
    uint32_t compositeNum = BASE_FACTOR;
    std::vector<composite_factor> factors = {};


    void processFactor(uint32_t factorIdx, uint32_t power = 1);
};


std::unique_ptr<compositeNumberInformation> randomCompositeNumber(
    uint32_t minOutput = (SMALLEST_COMPOSITE), 
    uint32_t maxOutput = std::numeric_limits<uint32_t>::max(),
    uint32_t minFactor = (SMALLEST_PRIME), 
    uint32_t maxFactor = (uint32_t)(std::sqrt(std::numeric_limits<uint32_t>::max())) 
);
