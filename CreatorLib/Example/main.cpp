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
    std::array<winrt::ColorMeasure, 7> colorValues = {
        //                               scRGB output values                measured XYZ values
		winrt::ColorMeasure{ winrt::float3(1.f, 0.f, 0.f), winrt::float3( 68.761f,   33.375f,   2.499f) },  // Red
		winrt::ColorMeasure{ winrt::float3(0.f, 1.f, 0.f), winrt::float3( 54.647f,  106.784f,  13.553f) },  // Green
		winrt::ColorMeasure{ winrt::float3(0.f, 0.f, 1.f), winrt::float3( 38.798f,   15.266f, 205.280f) },  // Blue
        winrt::ColorMeasure{ winrt::float3(1.f, 1.f, 1.f), winrt::float3(155.563f,  150.539f, 210.642f) },  // White
        winrt::ColorMeasure{ winrt::float3(1.f, 1.f, 0.f), winrt::float3(127.191f,  144.404f,  16.150f) },  // Red-Green
        winrt::ColorMeasure{ winrt::float3(0.f, 1.f, 1.f), winrt::float3( 94.112f,  123.849f, 218.484f) },  // Green-Blue
        winrt::ColorMeasure{ winrt::float3(1.f, 0.f, 1.f), winrt::float3(109.677f,   49.537f, 209.878f) }   // Red-Blue
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
}
