#include "pch.h"
#include "ColorCalibrationGenerator.h"
#include "ColorCalibrationGenerator.g.cpp"

// Using Eigen for more advanced matrix math here, the actual math used here is defined in this paper:
// https://web.stanford.edu/class/cs273/refs/umeyama.pdf
#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <iostream>

namespace winrt
{
    using namespace winrt::Windows::Foundation::Numerics;
}

namespace winrt::CreatorLib::implementation
{
    winrt::float3 scRGBToXYZ(winrt::float3 scRGB)
    {
        return 80.f*winrt::float3(
            0.4124564f * scRGB.x + 0.3575761f * scRGB.y + 0.1804375f * scRGB.z,
            0.2126729f * scRGB.x + 0.7151522f * scRGB.y + 0.0721750f * scRGB.z,
            0.0193339f * scRGB.x + 0.1191920f * scRGB.y + 0.9503041f * scRGB.z);
    }

    winrt::float4x4 ColorCalibrationGenerator::ComputeMatrix(array_view<winrt::CreatorLib::ColorMeasure const> measurements)
    {
        auto applicationOutput = Eigen::MatrixXf(3, measurements.size());
        auto displayMeasures = Eigen::MatrixXf(3, measurements.size());
         
        auto index = 0;

        for (auto& measure : measurements)
		{
			auto applicationOutputXYZ = scRGBToXYZ(measure.scRGB);
            applicationOutput(0, index) = applicationOutputXYZ.x;
            applicationOutput(1, index) = applicationOutputXYZ.y;
            applicationOutput(2, index) = applicationOutputXYZ.z;
            displayMeasures(0, index) = measure.XYZ.x;
            displayMeasures(1, index) = measure.XYZ.y;
            displayMeasures(2, index) = measure.XYZ.z;

            index++;
		}

        auto umeyama = Eigen::umeyama(displayMeasures, applicationOutput);

        return float4x4(umeyama(0, 0), umeyama(0, 1), umeyama(0, 2), umeyama(0,3),
                        umeyama(1, 0), umeyama(1, 1), umeyama(1, 2), umeyama(1,3),
						umeyama(2, 0), umeyama(2, 1), umeyama(2, 2), umeyama(2,3),
						0.f,           0.f,           0.f,           1.f);
    }
}
