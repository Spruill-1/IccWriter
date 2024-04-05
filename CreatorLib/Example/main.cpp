#include "pch.h"

#include "winrt\CreatorLib.h"

using namespace winrt;
using namespace Windows::Foundation;

int main()
{
    init_apartment();

    auto iccWriter = CreatorLib::IccWriter();
}
