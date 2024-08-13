#include "pch.h"
#include "ColorCalibrationGenerator.h"
#include "ColorCalibrationGenerator.g.cpp"

// Using Eigen for more advanced matrix math here, the actual math used here is defined in this paper:
// https://web.stanford.edu/class/cs273/refs/umeyama.pdf
// Effectively this is computing the affine transform which minimizes error between two sets of points, 
// in this case the application's output and the corresponding display measurements.
#include <Eigen/Dense>
#include <Eigen/Geometry>

namespace winrt
{
    using namespace winrt::Windows::Foundation::Numerics;
}

namespace winrt::CreatorLib::implementation
{
    winrt::float3 scRGBToXYZ(winrt::float3 scRGB)
    {
        // scRGB is defined such that 1,1,1 = 80 nits, XYZ is defined such that Y = nits. The standard transform for rec709 primaries->XYZ leaves
        // luminance unscaled, so 1,1,1 = 1 nit. So we include the scaling factor here.
        return 80 * winrt::float3(
            0.4124564f * scRGB.x + 0.3575761f * scRGB.y + 0.1804375f * scRGB.z,
            0.2126729f * scRGB.x + 0.7151522f * scRGB.y + 0.0721750f * scRGB.z,
            0.0193339f * scRGB.x + 0.1191920f * scRGB.y + 0.9503041f * scRGB.z);
    }

    // The matrix we're computing here is specifically an XYZ->XYZ matrix, so we need to convert the scRGB values to XYZ first.
    winrt::float4x4 ColorCalibrationGenerator::ComputeMatrix(array_view<winrt::CreatorLib::ColorMeasure const> measurements)
    {
        auto applicationOutput = Eigen::MatrixXf(3, measurements.size());
        auto displayMeasures = Eigen::MatrixXf(3, measurements.size());
         
        auto index = 0;

        for (auto& measure : measurements)
		{
			auto applicationOutputXYZ = scRGBToXYZ(measure.scRGB);
            applicationOutput(0, index) = applicationOutputXYZ.x / applicationOutputXYZ.y;
            applicationOutput(1, index) = applicationOutputXYZ.y / applicationOutputXYZ.y;
            applicationOutput(2, index) = applicationOutputXYZ.z / applicationOutputXYZ.y;

            displayMeasures(0, index) = measure.XYZ.x / measure.XYZ.y;
            displayMeasures(1, index) = measure.XYZ.y / measure.XYZ.y;
            displayMeasures(2, index) = measure.XYZ.z / measure.XYZ.y;

            index++;
		}

        auto umeyama = Eigen::umeyama(displayMeasures, applicationOutput, true);

        umeyama.transposeInPlace();

        return float4x4(umeyama(0, 0), umeyama(0, 1), umeyama(0, 2), umeyama(0,3),
                        umeyama(1, 0), umeyama(1, 1), umeyama(1, 2), umeyama(1,3),
						umeyama(2, 0), umeyama(2, 1), umeyama(2, 2), umeyama(2,3),
						0.f,           0.f,           0.f,           1.f);
    }
}
