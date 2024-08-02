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

    std::array<winrt::ColorMeasure, 7> colorValues = {
        //                               scRGB output values                measured XYZ values
		winrt::ColorMeasure{ winrt::float3(1.f, 0.f, 0.f), winrt::float3(34.335f, 15.406f,  1.260f) },  // Red
		winrt::ColorMeasure{ winrt::float3(0.f, 1.f, 0.f), winrt::float3(23.503f, 41.313f,  5.623f) },  // Green
		winrt::ColorMeasure{ winrt::float3(0.f, 0.f, 1.f), winrt::float3( 9.840f,  4.160f, 49.154f) },  // Blue
        winrt::ColorMeasure{ winrt::float3(1.f, 1.f, 1.f), winrt::float3(62.432f, 58.375f, 51.043f) },  // White
        winrt::ColorMeasure{ winrt::float3(1.f, 1.f, 0.f), winrt::float3(59.669f, 58.607f,  5.246f) },  // Red-Green
        winrt::ColorMeasure{ winrt::float3(0.f, 1.f, 1.f), winrt::float3(32.321f, 44.542f, 48.673f) },  // Green-Blue
        winrt::ColorMeasure{ winrt::float3(1.f, 0.f, 1.f), winrt::float3(38.900f, 17.666f, 47.036f) }   // Red-Blue
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
