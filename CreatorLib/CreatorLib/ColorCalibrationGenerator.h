#pragma once
#include "ColorCalibrationGenerator.g.h"

namespace winrt::CreatorLib::implementation
{
    struct ColorCalibrationGenerator : ColorCalibrationGeneratorT<ColorCalibrationGenerator>
    {
        ColorCalibrationGenerator() = default;

        static winrt::Windows::Foundation::Numerics::float4x4 ComputeMatrix(array_view<winrt::CreatorLib::ColorMeasure const> measurements);
    };
}
namespace winrt::CreatorLib::factory_implementation
{
    struct ColorCalibrationGenerator : ColorCalibrationGeneratorT<ColorCalibrationGenerator, implementation::ColorCalibrationGenerator>
    {
    };
}