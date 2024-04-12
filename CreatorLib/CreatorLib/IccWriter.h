#pragma once
#include "IccWriter.g.h"

namespace winrt::CreatorLib::implementation
{
    using unique_hprofile = wil::unique_any<HPROFILE, decltype(&::CloseColorProfile), ::CloseColorProfile>;

    struct IccWriter : IccWriterT<IccWriter>
    {
        IccWriter();

        winrt::Windows::Foundation::IAsyncAction CommitToFileAsync(winrt::Windows::Storage::StorageFile file);
        hstring profileDescription();
        void profileDescription(hstring const& value);
        hstring profileCopyright();
        void profileCopyright(hstring const& value);
        winrt::Windows::Foundation::Numerics::float3 whitePoint();
        void whitePoint(winrt::Windows::Foundation::Numerics::float3 const& value);
        winrt::Windows::Foundation::Numerics::float3 redPrimary();
        void redPrimary(winrt::Windows::Foundation::Numerics::float3 const& value);
        winrt::Windows::Foundation::Numerics::float3 greenPrimary();
        void greenPrimary(winrt::Windows::Foundation::Numerics::float3 const& value);
        winrt::Windows::Foundation::Numerics::float3 bluePrimary();
        void bluePrimary(winrt::Windows::Foundation::Numerics::float3 const& value);
        float fullFrameLuminance();
        void fullFrameLuminance(float value);
        float minLuminance();
        void minLuminance(float value);
        float peakLuminance();
        void peakLuminance(float value);
        winrt::Windows::Foundation::Numerics::float4x4 cscMatrix();
        void cscMatrix(winrt::Windows::Foundation::Numerics::float4x4 value);

    private:
        // Profile
        unique_hprofile m_hProfile = nullptr;

        // Lock
        std::mutex m_lock;

        // Helpers
        std::vector<uint8_t> GetTagData(uint32_t tagSignature);
        void SetTagData(uint32_t tagSignature, std::vector<uint8_t>& data);
    };
}
namespace winrt::CreatorLib::factory_implementation
{
    struct IccWriter : IccWriterT<IccWriter, implementation::IccWriter>
    {
    };
}
