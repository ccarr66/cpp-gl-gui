#include "image_utils.h"
#include <complex>
#include <numeric>        // for std::transform_reduce
#include <vector>
#include <cmath>          // for std::cos, std::sin
#include "TaskMonitor.h"
#include <climits>
#include <concepts>
#include <cstdint>

#if (USE_THREADPOOL)
#include "ThreadPool.h"
#else
#include <thread>
#endif

#ifdef ASYNC_TASKING
#include <future>        // std::async, std::future
#include <execution>     // Parallel execution policies
#endif



// 3) A helper function that converts DftExecPolicy -> PolicyVariant
PolicyVariant makePolicyVariant(DftExecPolicy policy) {
    switch (policy) {
    case DftExecPolicy::Seq:      return std::execution::seq;
    case DftExecPolicy::Par:      return std::execution::par;
    case DftExecPolicy::ParUnseq: return std::execution::par_unseq;
    case DftExecPolicy::Unseq:    return std::execution::unseq;
    }
    // fallback
    return std::execution::seq;
}

const double PI = 3.14159265358979323846;

// Maps from name -> mutex for that name
std::map<string, std::unique_ptr<std::mutex>> s_mutexes;

// Maps from name -> state for that name
std::map<string, std::unique_ptr<DftState>> s_states;   

// A global mutex to protect creation of new entries in s_mutexes/s_states
static std::mutex s_fftw3Mutex;

std::vector<float> createHistogramPlot(const std::vector<float>& data,
                                     int outWidth,
                                     int outHeight,
                                     int numBins)
{
    if (data.empty() || outWidth <= 0 || outHeight <= 0 || numBins <= 0)
    {
	    ErrorHandler::FatalError(string("createHistogramPlot: Invalid input parameters."));
    }

    // Find min and max values of the data
    float minVal = *std::min_element(data.begin(), data.end());
    float maxVal = *std::max_element(data.begin(), data.end());

    // Edge case: if maxVal == minVal, everything is in one bin
    if (maxVal == minVal) {
        maxVal = minVal + 1.0f; // Avoid divide-by-zero
    }

    // Allocate bins
    std::vector<int> histogram(numBins, 0);

    // Fill the histogram
    // Map each data value into one of [0..numBins-1]
    for (auto val : data)
    {
        int binIndex = static_cast<int>((val - minVal) / (maxVal - minVal) * (numBins - 1));
        ++histogram[binIndex];
    }

    // Find the maximum histogram count to scale the bar heights
    int maxCount = *std::max_element(histogram.begin(), histogram.end());
    if (maxCount == 0) {
        maxCount = 1; // Avoid divide-by-zero
    }

    // Prepare the output image buffer; start all pixels white
    // (You can use grayscale or color, here we'll do white background with black bars)
    std::vector<float> image(outWidth * outHeight, 1.0);

    // Determine how many output pixels wide each bin should be
    double binWidth = static_cast<double>(outWidth) / numBins;

    // For each bin, draw a vertical black bar in the image
    for (int b = 0; b < numBins; ++b)
    {
        // Height of this bin's bar, in pixels, scaled to [0..outHeight]
        int barHeight = static_cast<int>(
            (static_cast<double>(histogram[b]) / maxCount) * (outHeight - 1)
        );

        // The x-range in the output image this bin occupies
        int xStart = static_cast<int>(b * binWidth);
        int xEnd   = static_cast<int>((b + 1) * binWidth);

        // Clamp xEnd to not exceed outWidth
        if (xEnd > outWidth) {
            xEnd = outWidth;
        }

        // Fill pixels from the bottom up
        for (int x = xStart; x < xEnd; ++x)
        {
            // The top of the bar (counting from the bottom of the image)
            for (int y = outHeight - 1; y >= outHeight - barHeight; --y)
            {
                image[y * outWidth + x] = 0.0; // black
            }
        }
    }

    return image;
}

std::vector<float> createPixelValuePlot(
    const std::vector<float>& data,
    int outWidth,
    int outHeight
)
{
    if (data.empty() || outWidth <= 0 || outHeight <= 0)
    {
        ErrorHandler::FatalError("createPixelValuePlot: Invalid input parameters.");
    }

    // Find min and max values of the data
    float minVal = *std::min_element(data.begin(), data.end());
    float maxVal = *std::max_element(data.begin(), data.end());

    // Edge case: if maxVal == minVal, normalize to avoid divide-by-zero
    if (maxVal == minVal) {
        maxVal = minVal + 1.0f;
    }

    // Prepare the output image buffer; start all pixels white
    std::vector<float> image(outWidth * outHeight, 0.0f);

	size_t blackPixelCount = 0;
    // Draw the pixel value plot   
    for (int x = 0; x < outWidth; ++x)
    { 
        // Map x-coordinate to corresponding index in the data
        int dataIdx = static_cast<int>(
			(static_cast<float>(x) / static_cast<float>(outWidth)) * static_cast<float>(data.size() - 1));

        // Map the data value to the y-coordinate in the image
        int y = static_cast<int>((data[dataIdx] - minVal) / (maxVal - minVal) * (outHeight - 1));

        // Ensure y is within bounds
        y = std::clamp(y, 0, outHeight - 1);

        // Plot the point in black
        int pixelIdx = ((outHeight - 1 - y) * outWidth + x);
        image[pixelIdx] = 1.0f;     // 
		
		blackPixelCount++;
    }

	if (blackPixelCount < outWidth)
	{
		ErrorHandler::FatalError("Sanity check failed");
	}

    return image;
}


/**
 * @brief Compute the 1D FFT  
 * 
 *  - Input:  1D complex array to perform FFT on. Input data length should satisfy
 *            length = 2^N where N is an integer, or the routine will have to
 *            internally extend the buffer, forcing reallocation.
 *  - Output: 1D complex array containing the complex frequency spectrum
 *
 * @param inputData  size = 2^N
 * @return std::vector<float>  length
 */
 void FFT1D_2N (
    const std::shared_ptr<std::vector<float>> inputData,
    std::vector<std::complex<float>>& freqData
)
{
}

std::unsigned_integral auto nextPowerOfTwoExponent (
    std::integral auto n
) 
{
    using integralType = decltype(n);
    using unsignedType = std::make_unsigned_t<integralType>;

    unsignedType un = static_cast<unsignedType>(n);
    return ((uint64_t)std::bit_width(n));
}

bool isPowerOfTwo(uint64_t n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

 void FFT1D_2N_Start(
    std::shared_ptr<std::vector<float>> inputData,
    std::vector<std::complex<float>>& freqData
)
{
    const auto N = (uint64_t)(inputData->size());
    if (!isPowerOfTwo(N))
    {
        const auto newN = 1ull << nextPowerOfTwoExponent(N);
        try 
        {
            inputData->resize(newN);
        }
        catch (const std::bad_alloc& e) 
        {
            ErrorHandler::FatalError(string("BadAlloc: Failed to resize vector: ") + e.what());
        }
        catch (...) 
        {
            ErrorHandler::FatalError(string("Failed to resize vector."));
        }
    }
    FFT1D_2N(inputData, freqData);
}

enum class FFTLaunchTypes
{
    FFT2D,
    INVALID
};

void FFTLauncher(
    FFTLaunchTypes FFTType,
    std::shared_ptr<std::vector<float>> inputData,
    std::vector<std::complex<float>>& freqData
)
{
    if (inputData)
    {
        switch (FFTType)
        {
            case FFTLaunchTypes::FFT2D:
                FFT1D_2N_Start(inputData, freqData);
                break;
            default:
                break;
        }
    }  
}

/**
 * @brief Compute the 2D DFT magnitude (naïve implementation) of an image.
 *
 *  - Input:  1D float array representing a 2D image in row-major order.
 *  - Output: 1D float array containing the magnitude of the frequency spectrum
 *            in row-major order, size = width * height.
 *
 * @param inputData  size = width * height
 * @param width
 * @param height
 * @return std::vector<float>  (same width, same height)
 */

void DFTMag2D(
    const std::string& name,
    std::vector<std::complex<double>>& freqData,
    const std::shared_ptr<std::vector<float>> inputData,
    int width, int height,
    size_t startRow, size_t endRow, size_t taskIdx, 
    DftExecPolicy execPolicy = DftExecPolicy::Unseq
)
{
    const size_t uwidth = static_cast<size_t>(width);
    const size_t uheight = static_cast<size_t>(height);

    if (width < 1 || height < 1) {
        ErrorHandler::FatalError("Invalid DFT Image dimensions");
    }

    // The total number of frequency points in the block:
    size_t numFreqPoints = (endRow - startRow) * uwidth;

    // Create a vector of flattened frequency indices [0, 1, 2, ... numFreqPoints-1]
    std::vector<size_t> freqIndices(numFreqPoints);
    std::iota(freqIndices.begin(), freqIndices.end(), 0);

    // Precompute the spatial indices (for the inner transform_reduce)
    const size_t numSpatial = static_cast<size_t>(width) * static_cast<size_t>(height);
    std::vector<size_t> spatialIndices(numSpatial);
    std::iota(spatialIndices.begin(), spatialIndices.end(), 0);

    PolicyVariant variantPolicy = makePolicyVariant(execPolicy);

    std::atomic<uint32_t> freqCompleted{0};
    std::visit ( 
        [&](auto&& policy)  
        {
            // Process each frequency point, unsequenced
            std::for_each (policy,
                freqIndices.begin(), freqIndices.end(),
                [&](size_t localFreqIndex) 
                {
                    constexpr uint32_t incAmount = 50;
                    if (freqCompleted.fetch_add(1, std::memory_order_relaxed) % incAmount == 0)
                    {
                        IncSubTaskStats(name, taskIdx, incAmount);
                    }

                    // Map the flat frequency index to (u,v).
                    // v runs from startRow to endRow-1.
                    const size_t u = localFreqIndex % uwidth;
                    const size_t v = startRow + localFreqIndex / uwidth;

                    // Compute the DFT sum at (u, v) by reducing over the spatial domain.
                    const std::complex<double> sumVal = std::transform_reduce(
                        policy,       
                        spatialIndices.begin(), 
                        spatialIndices.end(),
                        std::complex<double>(0.0, 0.0), // initial value
                        std::plus<>(),                 // reduction operation
                        [&](size_t i) -> std::complex<double> 
                        {
                            // Convert the flat spatial index i into (x,y)
                            const size_t x = i % static_cast<size_t>(width);
                            const size_t y = i / static_cast<size_t>(width);
            
                            // Compute the angle
                            const double angle = -2.0 * PI * (
                                (static_cast<double>(u) * x / width) +
                                (static_cast<double>(v) * y / height)
                            );
                            const double realPart = std::cos(angle);
                            const double imagPart = std::sin(angle);
            
                            const double f_xy = static_cast<double>((*inputData)[i]);
                            return std::complex<double>(f_xy * realPart, f_xy * imagPart);
                        }
                    );

                    // Write the computed value into the frequency data array.
                    // We assume that freqData has been preallocated with size at least (height * width).
                    freqData[v * uwidth + u] = sumVal;
                }
            );
        }, 
        variantPolicy  
    ); //End of std::visit
}

void  forceDFTImageFullInit()
{
    // 1) Lock the global map-level mutex so we can safely iterate `s_states`
    std::lock_guard<std::mutex> mapLock(s_fftw3Mutex);

    // 2) Iterate over each entry in `s_states`
    for (auto& pair : s_states)
    {
        // The key is the name, the value is the DftState
        const std::string& name = pair.first;
        DftState& state         = *(pair.second);

        // 3) To safely access/modify the DftState, also lock the name-specific mutex
        std::lock_guard<std::mutex> nameLock(*(s_mutexes[name]));

        // Now we can do something with `state`
        // We will now do full image comparsion when deciding to skip DFT update regardless of set timeout
        state.skipInit = false;
        state.lastInputData = {};
        state.lastWidth = {};
        state.lastHeight = {};
        state.lastUseLog = {};
    }
}

#if (USE_FFTW3)


/**
 * @brief Compute the 2D FFT magnitude of an image and return the result as a float vector.
 * 
 * The input is a 1D array of floats representing a 2D grayscale image (row-major order).
 * 
 * @param inputData  1D array of floats, size = width * height
 * @param width      image width
 * @param height     image height
 * @param useLog     if true, apply log-scaling to the magnitude for better visibility
 * @return std::vector<float> containing the magnitude image (row-major order), size = width * height
 */
std::vector<float> computeFFTMagnitude(const std::vector<float> &inputData,
                                       int width, int height,
                                       bool useLog)
{
	fftw_plan plan;
	fftw_complex* in = nullptr;
	fftw_complex* out = nullptr;
	{
		std::lock_guard<std::mutex> lock(s_fftw3Mutex);
		in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * height * width);
		out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * height * width);


		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				in[y * width + x][0] = (double)inputData[x + y * width];  // real part
				in[y * width + x][1] = 0.0;                 // imaginary part
			}
		}

		// Plan the FFT
		plan = fftw_plan_dft_2d(
			height, width,
			in,
			out,
			FFTW_FORWARD,             // direction
			FFTW_ESTIMATE             // plan strategy (can also use FFTW_MEASURE, etc.)
		);
	}
    // Execute the FFT
    fftw_execute(plan);

    // freqData now contains the complex frequency-domain data
    // We will compute the magnitude and store in a 2D buffer of size width * height
    std::vector<float> magnitude(width * height, 0.0f);

    // The real-to-complex transform shape is (height) x (width/2 + 1)
    // We'll fill the full width * height by mirroring frequency content to get a typical "FFT image"
    // Note: the negative frequencies are implied or can be reconstructed from the positive frequencies
    // For a simple visualization, let's just fill what we have and ignore symmetrical reconstruction for now.

    // Calculate magnitude = std::sqrt( Re^2 + Im^2 )
    // freqData[y * (width/2 + 1) + x] is the complex value at coordinate (x, y)
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < (width / 2 + 1); ++x)
        {
			double real_part = out[y * width + x][0];
			double imag_part = out[y * width + x][1];
			double magnitude = std::sqrt(real_part * real_part + imag_part * imag_part);
        }
    }

	{
		std::lock_guard<std::mutex> lock(s_fftw3Mutex);
		// Clean up FFTW
		fftw_destroy_plan(plan);
		fftw_free(in);
		fftw_free(out);
	}

    // Optionally take the log of the magnitude to improve visibility
    if (useLog)
    {
        for (size_t i = 0; i < magnitude.size(); ++i)
        {
            // a common formula is log(1 + mag)
            magnitude[i] = std::log(1.0f + magnitude[i]);
        }
    }

    return magnitude;
}
#endif

//returns idx of max prime <= N
size_t factorizationEngine::establishPrimeRangeUpTo(uint32_t N)
{
    bool attemptedToExtendPrimes = false;
    size_t prime_idx = 0;
    while (primes[prime_idx] < N)
    {
        if (prime_idx + 1 < primes.size())
        {
            if (primes[prime_idx + 1] > N)
            {
                break;
            }
            else
            {
                prime_idx++;
            }
        } 
        // We cannot get prime + 1 beyond N without arbitrary memory usage in this scheme
        // Primes can have arbitrary gaps between them. 
        else if (attemptedToExtendPrimes)
        {
            break;
        }
        else if (!attemptedToExtendPrimes)
        {
            const uint32_t largestComputedPrime = primes.back();
            
            if(largestComputedPrime % 2 == 0)
            {
                ErrorHandler::FatalError("Primes are always odd!");
            }

            // Calculate the next odd number in the new sieve range
            uint32_t first_num = largestComputedPrime + 2;

            // Determine the sieve size for odd numbers only
            size_t sieve_size = 0;
            if (first_num <= N) {
                sieve_size = ((N - first_num) / 2) + 1;
            }

            // Create sieve with odd numbers starting from first_num
            std::vector<uint32_t> sieve(sieve_size);
            std::for_each(
                std::execution::par_unseq,
                sieve.begin(),
                sieve.end(),
                [first_num, base = sieve.data()](uint32_t& num) {
                    const size_t i = &num - base;  // Calculate index from memory offset
                    const size_t s_num = (size_t)first_num + 2 * i;
                    if (s_num > std::numeric_limits<uint32_t>::max())
                    {
                        num = std::numeric_limits<uint32_t>::max();
                    }
                    else
                    {
                        num = (uint32_t)s_num;
                    }
                }
            );

            std::vector<uint8_t> compositeMarkers(sieve_size);
            std::mutex frontier_mutex;
            size_t smallestNonCompositeIdx = 0;
            
            std::for_each(
                std::execution::par_unseq,
                std::next(primes.begin()),
                primes.end(),
                [&](uint32_t prime) 
            {
                // Unsynchronized read - may be stale but safe due to tolerance
                size_t local_frontier = smallestNonCompositeIdx;
            
                for (size_t idx = local_frontier; idx < sieve.size(); ++idx) {
                    // Lock-free fast path (tolerate stale 0)
                    if (compositeMarkers[idx] != 0) continue;
            
                    // Perform divisibility check without lock
                    if (sieve[idx] % prime != 0) continue;
            
                    // Only lock when we need to modify state
                    std::lock_guard<std::mutex> lock(frontier_mutex);
                    
                    // Safe modification section
                    compositeMarkers[idx] |= 1;
            
                    // Update frontier if needed
                    if (idx == smallestNonCompositeIdx) {
                        while (smallestNonCompositeIdx < sieve.size() && 
                               compositeMarkers[smallestNonCompositeIdx] == 1) {
                            smallestNonCompositeIdx++;
                        }
                    }
                }
            });

            // Need to continue Eratosthenes from largestComputedPrime
            // having marked composites of precomputed primes in the sieve
            for (uint32_t idx = smallestNonCompositeIdx; idx < sieve.size(); idx++)
            {
                while (idx < sieve.size() && compositeMarkers[idx] == 0x1) idx++; //advance idx to next prime

                if (idx < sieve.size())
                {
                    const uint32_t curPrime = sieve[idx];
                    uint32_t curMultiple = curPrime;
                    size_t curMultipleIdx = idx + curMultiple; 
                    while (curMultipleIdx < sieve.size())
                    {
                        compositeMarkers[curMultipleIdx] = 0x1; //for 'true' to avoid vector<bool>

                        curMultiple += curPrime;
                        curMultipleIdx = idx + curMultiple; 
                    }
                }
            }
            
            for (uint32_t idx = smallestNonCompositeIdx; idx < sieve.size(); idx++)
            {
                while (idx < sieve.size() && compositeMarkers[idx] == 0x1) idx++; //advance idx to next prime
                if (idx < sieve.size())
                {
                    primes.push_back(sieve[idx]); //compile new primes
                }
            }
            attemptedToExtendPrimes = true;
        }
        //else
        //{
        //    ErrorHandler::FatalError("One attempt to extend primes table should have been enough per intended design.");
        //}
    }
    return (prime_idx);
}

std::vector<uint32_t> factorizationEngine::primes = {
    2,    3,    5,    7,   11,   13,   17,   19,   23,   29,   31,   37,   41,   43,
   47,   53,   59,   61,   67,   71,   73,   79,   83,   89,   97,  101,  103,  107, 
  109,  113,  127,  131,  137,  139,  149,  151,  157,  163,  167,  173,  179,  181,
  191,  193,  197,  199,  211,  223,  227,  229,  233,  239,  241,  251,  257,  263,
  269,  271,  277,  281,  283,  293,  307,  311,  313,  317,  331,  337,  347,  349,
  353,  359,  367,  373,  379,  383,  389,  397,  401,  409,  419,  421,  431,  433,  
  439,  443,  449,  457,  461,  463,  467,  479,  487,  491,  499,  503,  509,  521,
  523,  541,  547,  557,  563,  569,  571,  577,  587,  593,  599,  601,  607,  613,  
  617,  619,  631,  641,  643,  647,  653,  659,  661,  673,  677,  683,  691,  701,
  709,  719,  727,  733,  739,  743,  751,  757,  761,  769,  773,  787,  797,  809,  
  811,  821,  823,  827,  829,  839,  853,  857,  859,  863,  877,  881,  883,  887,  
  907,  911,  919,  929,  937,  941,  947,  953,  967,  971,  977,  983,  991,  997, 
 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091,
 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193,
 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291,
 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423,
 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493,
 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601,
 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699,
 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811,
 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931,
 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029,
 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 2137, 
 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267,
 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357,
 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459,
 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593,
 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693,
 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753, 2767, 2777, 2789, 2791,
 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903,
 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023,
 3037, 3041, 3049, 3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167,
 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259, 3271,
 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347, 3359, 3361, 3371, 3373,
 3389, 3391, 3407, 3413, 3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511,
 3517, 3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607,
 3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709,
 3719, 3727, 3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833,
 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929, 3931,
 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057,
 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133, 4139, 4153, 4157, 4159, 4177,
 4201, 4211, 4217, 4219, 4229, 4231, 4241, 4243, 4253, 4259, 4261, 4271, 4273, 4283,
 4289, 4297, 4327, 4337, 4339, 4349, 4357, 4363, 4373, 4391, 4397, 4409, 4421, 4423,
 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493, 4507, 4513, 4517, 4519, 4523, 4547,
 4549, 4561, 4567, 4583, 4591, 4597, 4603, 4621, 4637, 4639, 4643, 4649, 4651, 4657,
 4663, 4673, 4679, 4691, 4703, 4721, 4723, 4729, 4733, 4751, 4759, 4783, 4787, 4789,
 4793, 4799, 4801, 4813, 4817, 4831, 4861, 4871, 4877, 4889, 4903, 4909, 4919, 4931,
 4933, 4937, 4943, 4951, 4957, 4967, 4969, 4973, 4987, 4993, 4999, 5003, 5009, 5011,
 5021, 5023, 5039, 5051, 5059, 5077, 5081, 5087, 5099, 5101, 5107, 5113, 5119, 5147,
 5153, 5167, 5171, 5179, 5189, 5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279,
 5281, 5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381, 5387, 5393, 5399, 5407, 5413,
 5417, 5419, 5431, 5437, 5441, 5443, 5449, 5471, 5477, 5479, 5483, 5501, 5503, 5507,
 5519, 5521, 5527, 5531, 5557, 5563, 5569, 5573, 5581, 5591, 5623, 5639, 5641, 5647,
 5651, 5653, 5657, 5659, 5669, 5683, 5689, 5693, 5701, 5711, 5717, 5737, 5741, 5743,
 5749, 5779, 5783, 5791, 5801, 5807, 5813, 5821, 5827, 5839, 5843, 5849, 5851, 5857,
 5861, 5867, 5869, 5879, 5881, 5897, 5903, 5923, 5927, 5939, 5953, 5981, 5987, 6007,
 6011, 6029, 6037, 6043, 6047, 6053, 6067, 6073, 6079, 6089, 6091, 6101, 6113, 6121,
 6131, 6133, 6143, 6151, 6163, 6173, 6197, 6199, 6203, 6211, 6217, 6221, 6229, 6247,
 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301, 6311, 6317, 6323, 6329, 6337, 6343,
 6353, 6359, 6361, 6367, 6373, 6379, 6389, 6397, 6421, 6427, 6449, 6451, 6469, 6473,
 6481, 6491, 6521, 6529, 6547, 6551, 6553, 6563, 6569, 6571, 6577, 6581, 6599, 6607,
 6619, 6637, 6653, 6659, 6661, 6673, 6679, 6689, 6691, 6701, 6703, 6709, 6719, 6733,
 6737, 6761, 6763, 6779, 6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833, 6841, 6857,
 6863, 6869, 6871, 6883, 6899, 6907, 6911, 6917, 6947, 6949, 6959, 6961, 6967, 6971,
 6977, 6983, 6991, 6997, 7001, 7013, 7019, 7027, 7039, 7043, 7057, 7069, 7079, 7103,
 7109, 7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207, 7211, 7213, 7219, 7229,
 7237, 7243, 7247, 7253, 7283, 7297, 7307, 7309, 7321, 7331, 7333, 7349, 7351, 7369,
 7393, 7411, 7417, 7433, 7451, 7457, 7459, 7477, 7481, 7487, 7489, 7499, 7507, 7517,
 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573, 7577, 7583, 7589, 7591, 7603,
 7607, 7621, 7639, 7643, 7649, 7669, 7673, 7681, 7687, 7691, 7699, 7703, 7717, 7723,
 7727, 7741, 7753, 7757, 7759, 7789, 7793, 7817, 7823, 7829, 7841, 7853, 7867, 7873,
 7877, 7879, 7883, 7901, 7907, 7919
}; 

std::vector<composite_factor> factorizeN(uint32_t N)
{
    auto factors = std::vector<composite_factor>{};

    if (N > 2)
    {
        const auto maxPrimeNeeded = static_cast<uint32_t>(std::sqrt(N));
        const auto maxPrimeIdx = factorizationEngine::establishPrimeRangeUpTo(maxPrimeNeeded);

        size_t factorIdx = 0;
        uint32_t factorUnderTest = 0;
        uint32_t compositeFactorAccumulator = BASE_FACTOR; // 1 is common factor to all numbers, 1^n = 1
        do {
            factorUnderTest = factorizationEngine::primes[factorIdx];

            auto localN = N;
            auto factorPower = 0;
            auto factorPowerAcc = BASE_FACTOR;
            while (localN % factorUnderTest == 0)
            {
                localN /= factorUnderTest;
                factorPower++;
                factorPowerAcc *= factorUnderTest;
            }
            if (factorPower > 0)
            {
                factors.push_back(composite_factor(factorUnderTest, factorPower));
                compositeFactorAccumulator *= factorPowerAcc;
            }
            factorIdx++;
        } while (
            compositeFactorAccumulator != N
            && factorIdx <= maxPrimeIdx
            && factorIdx < factorizationEngine::primes.size()
            && factorizationEngine::primes[factorIdx] <= maxPrimeNeeded
        );

        if (compositeFactorAccumulator != N) {
            // The remainder is prime
            auto remainder = N / compositeFactorAccumulator;
            factors.push_back({ remainder, 1 });
        }
    }
    return (factors);
}



void compositeNumberInformation::processFactor(uint32_t factorIdx, uint32_t power)
{
    bool factorAdded = false;
    if (factorIdx >= factorizationEngine::primes.size())
    {
        ErrorHandler::FatalError("Factor Idx exceeds computed primes!");
    }

    const auto factor =  factorizationEngine::primes[factorIdx];

    // Check if the factor already exists (sequential loop)
    for (auto& f : factors) {
        if (f.factor == factor) {
            f.power += power;
            factorAdded = true;
            break;  // Exit early since factors are unique
        }
    }

    // Insert new factor in sorted order
    if (!factorAdded) {
        // Find insertion point using binary search
        auto it = std::lower_bound(
            factors.begin(), factors.end(), factor,
            [](const composite_factor& cf, uint32_t f) {
                return cf.factor < f;
            }
        );
        factors.insert(it, composite_factor(factor, 1));
    }
}

#include "GlobalRand.h"

std::unique_ptr<compositeNumberInformation> randomCompositeNumber( uint32_t minOutput, uint32_t maxOutput, uint32_t minFactor, uint32_t maxFactor)
{
    auto compositeInfo = std::make_unique<compositeNumberInformation>();

    if (maxOutput <= SMALLEST_COMPOSITE)
    {
        compositeInfo->compositeNum = SMALLEST_COMPOSITE;
        auto primeIdx = factorizationEngine::establishPrimeRangeUpTo(SMALLEST_PRIME);
        compositeInfo->processFactor(primeIdx, 2); //smallest composite is 4 which is 2^2
    }
    else
    {
        if (minOutput > maxOutput)
        {
            maxOutput = std::numeric_limits<uint32_t>::max(); // ignore badly formed parameter
        }

        if (minOutput == maxOutput 
            || minOutput < SMALLEST_COMPOSITE)
        {
            minOutput = SMALLEST_COMPOSITE;                   // ignore badly formed parameter
        }

        if ((minFactor < SMALLEST_PRIME)                            
            || (minFactor >= (uint32_t)(std::sqrt(maxOutput))))
        {
            minFactor = SMALLEST_PRIME;                       // ignore badly formed parameter
        }

        if (maxOutput < std::numeric_limits<uint32_t>::max()
            || (maxFactor != (uint32_t)(std::sqrt(std::numeric_limits<uint32_t>::max()))))
        {
            maxFactor = std::min((uint32_t)(std::sqrt(maxOutput)), maxFactor);
        }

        if (minFactor > maxFactor || minFactor < SMALLEST_PRIME)
        {
            minFactor = SMALLEST_PRIME;
        }

        if (maxFactor <= SMALLEST_PRIME)
        {
            maxFactor = SMALLEST_PRIME+1;
        }

        const auto minPrimeIdx = factorizationEngine::establishPrimeRangeUpTo(minFactor);
        auto maxPrimeIdx = factorizationEngine::establishPrimeRangeUpTo(maxFactor);
        auto factorDistr = std::uniform_int_distribution<int>(minPrimeIdx, maxPrimeIdx);

        while(true)
        {
            const auto randomPrimeIdx = (uint32_t)GlobalRand::randIntDist(&factorDistr);
            const auto randomFactor = factorizationEngine::primes[randomPrimeIdx];
            const auto intermAcc = static_cast<long long>(compositeInfo->compositeNum) * randomFactor;
            
            if (   intermAcc > static_cast<long long>(maxOutput) 
                || intermAcc > std::numeric_limits<uint32_t>::max() 
                || intermAcc < std::numeric_limits<uint32_t>::min()) {
                // Overflow occurred or we exceeded our maxOutput
                // We now know to only try smaller factors
                maxPrimeIdx = factorizationEngine::establishPrimeRangeUpTo(randomFactor - 1);
                if (maxPrimeIdx > minPrimeIdx)
                {
                    factorDistr = std::uniform_int_distribution<int>(minPrimeIdx, maxPrimeIdx);
                }
                else // we tried a factor that caused us to exceed our max or overflow, 
                     // and we cannot try any smaller factor, so we are done
                {
                    break;
                }
            }
            else
            {
                compositeInfo->compositeNum = intermAcc;
                compositeInfo->processFactor(randomPrimeIdx);
                if (compositeInfo->compositeNum >= minOutput 
                    && compositeInfo->compositeNum <= maxOutput) //reached target!
                {
                    // Check if composite (not a single prime)
                    if (compositeInfo->factors.size() > 1 || 
                        (compositeInfo->factors.size() == 1 && 
                         compositeInfo->factors[0].power >= 2)) 
                    {
                        break;
                    }
                }
            }
        }
    }
    return (compositeInfo);
}

//std::vector<float> FFT_1d(const std::vector<float>& inputData)
//{
//
//}

//only 1 dft calc can be in progress for a given 'name'
void    dftMagImageGenerator(
              const std::string& name
            , const std::vector<float>& inputData    
            , int width
            , int height
            , size_t numTasks
            , DftExecPolicy execPolicy
            , bool useLog    
        #ifdef ASYNC_TASKING
            , bool useAync
        #endif
        )
{
    // Ensure mutex/state object exists
    {
        std::lock_guard<std::mutex> lock(s_fftw3Mutex);
        if (s_mutexes.find(name) == s_mutexes.end()) {
            s_mutexes[name] = std::make_unique<std::mutex>();
            s_states[name] = std::make_unique<DftState>();
        }
    }

    //
    // 2) Lock the name-specific mutex to check if a thread is running
    //
    bool startTask = false;
    std::shared_ptr<std::vector<float>> localInputCopy;
    {
        std::lock_guard<std::mutex> lock(*(s_mutexes[name]));
        auto& state = *s_states[name];

        if (!state.stateInit) 
        {
            // --- Check skip-timeout window first ---
            // If we are still within skipTimeoutSec from skipStartTime,
            // immediately return the last known result (no check of images).
            auto now = std::chrono::steady_clock::now();
            auto skipEndTime = state.skipStartTime + 
                            std::chrono::milliseconds(
                                static_cast<int>(state.skipTimeoutSec * 1000.0));

            // Compare current vs. previous input/params
            bool isSame =
                (width     == state.lastWidth     &&
                height    == state.lastHeight    &&
                useLog    == state.lastUseLog);
                
            if (isSame && state.skipInit && now < skipEndTime) {
                // Nothing obvious changed,
                // We are still in the "skip" window => skip full image comparsion
                //return state.lastComputedResult;
            }
            else
            {

                isSame = isSame && (inputData == state.lastInputData);

                // If everything is the same or if we don't have an image to display, skip re-computation:
                if (isSame || (width == 0 || height == 0)) {
                    // Record the time we decided to skip
                    state.skipStartTime = now;
                    // Increase skipTimeoutSec by 0.5 up to a max of 2.0
                    state.skipTimeoutSec = std::min(2.0, state.skipTimeoutSec + 0.5);
                    
                    state.skipInit = true;

                    // Return the last known result
                    //return state.lastComputedResult;
                }
                else
                {

                    state.skipInit = false;

                    // If it's different, reset the skipTimeoutSec to 0
                    state.skipTimeoutSec = 0.0;

                    // Otherwise, update the stored state to reflect this new input
                    state.lastInputData = inputData;
                    state.lastWidth     = width;
                    state.lastHeight    = height;
                    state.lastUseLog    = useLog;
                    
                    // Mark that we’re starting 
                    state.stateInit = true;

                    startTask = true;
                    
                    // Make a copy for the threads
                    localInputCopy = std::make_shared<std::vector<float>>(state.lastInputData);
                }
            }
        }
    }

    if (startTask)
    {
        // Allocate shared buffer for intermediate results
        auto freqData = std::make_shared<std::vector<std::complex<double>>>(width * height);

        // Prepare child tasks
        constexpr size_t minTasksNeeded = 1;
        numTasks = std::max(minTasksNeeded, numTasks);
        size_t chunkHeight = height / numTasks;

        std::vector<std::function<void()>> tasks;
        std::vector<uint32_t> subtaskCount;


        size_t taskIdx = 0;
        size_t startRow = (taskIdx * chunkHeight); //0
        size_t endRow   = (std::min(static_cast<size_t>(height), static_cast<size_t>(startRow + chunkHeight)));
        

#ifdef ASYNC_TASKING
        if (useAync)
        {
            //----------------------------------------------------------------------
            // ASYNC_TASKING branch: Precompute all task parameters first
            //----------------------------------------------------------------------
            struct TaskParams {
                size_t startRow;
                size_t endRow;
                uint32_t taskIdx;
                uint32_t subtaskPixels;
            };

            // Precompute all task parameters first
            std::vector<TaskParams> taskParams;
            taskParams.reserve(numTasks);
            subtaskCount.reserve(numTasks);

            size_t currentRow = 0;
            for (size_t idx = 0; idx < numTasks; ++idx) {
                const size_t endRow = (idx == numTasks - 1)
                                    ? static_cast<size_t>(height)
                                    : currentRow + chunkHeight;
                
                const uint32_t pixels = static_cast<uint32_t>((endRow - currentRow) * width);
                
                taskParams.push_back({
                    currentRow,
                    endRow,
                    static_cast<uint32_t>(idx),
                    pixels
                });
                
                subtaskCount.push_back(pixels);
                currentRow = endRow;
            }

            // Single initialization with real counts
            InitTaskMonitor(name, static_cast<uint32_t>(numTasks), subtaskCount);

            // Launch async tasks using precomputed parameters
            std::vector<std::future<void>> futures;
            futures.reserve(taskParams.size());

            for (const auto& params : taskParams) {
                // Capture freqData by value to keep buffer alive
                futures.push_back(
                    std::async(std::launch::async, [=, freqData=freqData]() {
                        IncTaskinflight(name);
                        DFTMag2D(
                            name,
                            *freqData,
                            localInputCopy,
                            width,
                            height,
                            params.startRow,
                            params.endRow,
                            params.taskIdx,
                            execPolicy
                        );
                        DecTaskinflight(name);
                    })
                );
            }

            // Detached thread for async post-processing
            std::thread([futures = std::move(futures),
                        name,
                        useLog,
                        freqData]() mutable 
            { 
                
                // Wait for all DFT tasks (non-blocking for main thread)
                for (auto& f : futures) f.get();

                // Parallel result processing
                std::vector<float> computedResult(freqData->size());
                std::transform(
                    std::execution::par,
                    freqData->begin(),
                    freqData->end(),
                    computedResult.begin(),
                    [](const auto& cval) { 
                        return static_cast<float>(std::abs(cval));
                    }
                );

                if (useLog) {
                    std::for_each(
                        std::execution::par,
                        computedResult.begin(),
                        computedResult.end(),
                        [](auto& v) { v = std::log(1.0f + v); }
                    );
                }

                // Final state update
                {
                    std::lock_guard<std::mutex> lock(*s_mutexes.at(name));
                    s_states.at(name)->lastComputedResult = std::move(computedResult);
                    s_states.at(name)->stateInit = false;
                }

                CleanupTaskMonitor(name);
                
            }).detach();
        }
        else
#endif
        {
            std::vector<std::function<void()>> tasks;
            tasks.reserve(numTasks);

            size_t startRow = 0;
            for (size_t taskIdx = 0; taskIdx < numTasks; ++taskIdx)
            {
                size_t endRow = (taskIdx == numTasks - 1)
                                ? (size_t)height
                                : (startRow + chunkHeight);

                tasks.push_back([=]() {
                    IncTaskinflight(name);
                    DFTMag2D(
                        name,
                        *freqData, localInputCopy,
                        width, height,
                        startRow, endRow,
                        (uint32_t)taskIdx,
                        execPolicy
                    );
                    DecTaskinflight(name);
                });

                uint32_t nPixels = static_cast<uint32_t>((endRow - startRow) * width);
                subtaskCount.push_back(nPixels);

                startRow = endRow;
            }

            InitTaskMonitor(name, (uint32_t)numTasks, subtaskCount);

            {
                SubmitBatchToTaskingSystem(
                    name,
                    std::move(tasks),
                    [=]() {
                        std::vector<float> computedResult(freqData->size());
                        for (size_t i = 0; i < freqData->size(); ++i) {
                            computedResult[i] = (float)std::abs((*freqData)[i]);
                        }
                        if (useLog) {
                            for (auto &v : computedResult) {
                                v = std::log(1.0f + v);
                            }
                        }
                        {
                            std::lock_guard<std::mutex> lock(*(s_mutexes[name]));
                            s_states[name]->lastComputedResult = std::move(computedResult);
                            s_states[name]->stateInit = false;
                            CleanupTaskMonitor(name);
                        }
                    }
                );
            }
        }

    } // end if (startTask)

    return;
}
