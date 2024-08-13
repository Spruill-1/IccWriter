#include "pch.h"

#include "winrt\CreatorLib.h"

#include "output.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::Storage;
    using namespace winrt::CreatorLib;
};

int main(int argc, char* argv[])
{
    winrt::init_apartment();

    auto iccWriter = winrt::IccWriter();

    // Get the current whitepoint
    auto whitePoint = iccWriter.WhitePoint();
    printf("White point: (%f, %f, %f)\n", whitePoint.x, whitePoint.y, whitePoint.z);

    auto luts = iccWriter.ReGammaLuts();
    printf("ReGamma LUTs: %d\n", luts.size());
    for (size_t i = 0; i < luts.size(); i++)
    {
        printf("\t%f, %f, %f\n", luts[i].x, luts[i].y, luts[i].z);
    }

    // This is the set of output scRGB values and the corresponding measured XYZ values for a display. There is no real right answer to how
    // many points/measurements should be included here, but more is generally better and you need a minimum of 3. Likewise, there is no real right answer as to which
    // points should be included - any affine transform (i.e. the matrix hardware) is going to impact _all_ points.
    std::array<winrt::ColorMeasure, 3> colorValues = {
        //                               scRGB output values                measured XYZ values
		winrt::ColorMeasure{ winrt::float3(1.f, 0.f, 0.f), winrt::float3( 38.258f,   16.184f,   0.849f) },  // Red
		winrt::ColorMeasure{ winrt::float3(0.f, 1.f, 0.f), winrt::float3( 23.313f,   29.723f,   2.952f) },  // Green
		winrt::ColorMeasure{ winrt::float3(0.f, 0.f, 1.f), winrt::float3(  3.795f,    1.807f,  10.332f) }  // Blue
	};

    auto matrix = winrt::ColorCalibrationGenerator::ComputeMatrix(colorValues);
    iccWriter.CscMatrix(matrix);

    // Create a StorageFile to save the output with
    winrt::StorageFolder folder = winrt::StorageFolder::GetFolderFromPathAsync(std::filesystem::current_path().c_str()).get();

    winrt::hstring fileName = L"test.icc";
    if (argc > 1)
    {
        fileName = winrt::to_hstring(argv[1]);
    }

    winrt::StorageFile file = folder.CreateFileAsync(fileName, winrt::CreationCollisionOption::GenerateUniqueName).get();

    // Write the ICC profile to the file
    iccWriter.CommitToFileAsync(file).get();

    printf("Profile written to %s\n", winrt::to_string(file.Path()).c_str());

    // Apply the transform to the display

    winrt::float4x4 M =
    {
         0.4124564, 0.3575761, 0.1804375, 0,
         0.2126729, 0.7151522, 0.0721750, 0,
         0.0193339, 0.1191920, 0.9503041, 0,
         0.f,       0.f,       0.f,       1.f
    };

    winrt::float4x4 Mi =
    {
         3.2404542, -1.5371385, -0.4985314,  0,
        -0.9692660,  1.8760108,  0.0415560,  0,
         0.0556434, -0.2040259,  1.0572252,  0,
         0.f,        0.f,        0.f,        1.f
    };


    auto matrix2 = Mi * matrix * M;

    auto colors = winrt::float4x4(
        1, 0, 0, 1,
        0, 1, 0, 1,
        0, 0, 1, 1,
        0, 0, 0, 1);

    auto primaries = matrix2 * colors;

    printf("\n\nscRGB primaries: \n");
    printf("/*R*/ {%f,\t%f,\t%f},\n", primaries.m11, primaries.m21, primaries.m31);
    printf("/*G*/ {%f,\t%f,\t%f},\n", primaries.m12, primaries.m22, primaries.m32);
    printf("/*B*/ {%f,\t%f,\t%f},\n", primaries.m13, primaries.m23, primaries.m33);
    printf("/*W*/ {%f,\t%f,\t%f} \n", primaries.m14, primaries.m24, primaries.m34);
}
