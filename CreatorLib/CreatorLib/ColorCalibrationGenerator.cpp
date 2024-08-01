#include "pch.h"
#include "ColorCalibrationGenerator.h"
#include "ColorCalibrationGenerator.g.cpp"

// Using Eigen for more advanced matrix math here, the actual math used here is defined in this paper:
// https://web.stanford.edu/class/cs273/refs/umeyama.pdf
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
        auto M = winrt::float4x4(
            0.4124564f, 0.3575761f, 0.1804375f, 0.f,
            0.2126729f, 0.7151522f, 0.0721750f, 0.f,
            0.0193339f, 0.1191920f, 0.9503041f, 0.f,
            0.f,        0.f,        0.f,        1.f);

        auto xyz_ = winrt::transform(winrt::float4(scRGB, 1.f), M);

        return winrt::float3(xyz_.x, xyz_.y, xyz_.z);
    }

    winrt::float4x4 ColorCalibrationGenerator::ComputeMatrix(array_view<winrt::CreatorLib::ColorMeasure const> measurements)
    {
        auto applicationOutput = Eigen::Matrix3Xf();
        auto displayMeasures = Eigen::Matrix3Xf();;

        for (auto& measure : measurements)
		{
			auto applicationOutputXYZ = scRGBToXYZ(measure.scRGB);

            applicationOutput << Eigen::Vector3f (applicationOutputXYZ.x, applicationOutputXYZ.y, applicationOutputXYZ.z);
            displayMeasures << Eigen::Vector3f(measure.XYZ.x, measure.XYZ.y, measure.XYZ.z);
		}

        auto umeyama = Eigen::umeyama(applicationOutput, displayMeasures);

        return float4x4(umeyama(0, 0), umeyama(0, 1), umeyama(0, 2), 0.f,
                        umeyama(1, 0), umeyama(1, 1), umeyama(1, 2), 0.f,
						umeyama(2, 0), umeyama(2, 1), umeyama(2, 2), 0.f,
						0.f,           0.f,           0.f,           1.f);
    }
}
