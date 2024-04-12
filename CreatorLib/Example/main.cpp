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
    auto whitePoint = iccWriter.whitePoint();
    printf("White point: (%f, %f, %f)\n", whitePoint.x, whitePoint.y, whitePoint.z);

    // Modify the whitepoint (these are D65-relative coordinates, the chromatic adaptation to D50 is done automatically)
    iccWriter.whitePoint({ 0.33f, 0.33f, 0.33f });

    // Modify the CSC matrix to turn the display horrendously pink
    // This is actually a 3x3 matrix, we're just using a 4x4 one for convenience
    // remember that this is a XYZ-XYZ matrix, not RGB-RGB.
    iccWriter.cscMatrix({
		    1.0f, 0.5f, 0.0f, 0.0f,
		    0.0f, 0.1f, 0.0f, 0.0f,
		    0.0f, 0.0f, 0.1f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
	    });

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
