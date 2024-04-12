#include "pch.h"

#include "winrt\CreatorLib.h"

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

    // Create a StorageFile to save the output with
    winrt::StorageFolder folder = winrt::StorageFolder::GetFolderFromPathAsync(std::filesystem::current_path().c_str()).get();

    winrt::hstring fileName = L"test.icc";
    if (argc > 1)
	{
        fileName = winrt::to_hstring(argv[1]);
	}

    winrt::StorageFile file = folder.CreateFileAsync(fileName, winrt::CreationCollisionOption::ReplaceExisting).get();

    // Write the ICC profile to the file
    iccWriter.CommitToFileAsync(file).get();

    printf("Profile written to %s\n", winrt::to_string(file.Path()).c_str());
}
