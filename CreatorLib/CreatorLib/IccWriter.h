#pragma once
#include "IccWriter.g.h"

namespace winrt::CreatorLib::implementation
{
    using unique_hprofile = wil::unique_any<HPROFILE, decltype(&::CloseColorProfile), ::CloseColorProfile>;

    struct IccWriter : IccWriterT<IccWriter>
    {
        IccWriter();

        winrt::Windows::Foundation::IAsyncAction CommitToFileAsync(winrt::Windows::Storage::StorageFile file);
        hstring ProfileDescription();
        void ProfileDescription(hstring const& value);
        hstring ProfileCopyright();
        void ProfileCopyright(hstring const& value);
        winrt::Windows::Foundation::Numerics::float3 WhitePoint();
        void WhitePoint(winrt::Windows::Foundation::Numerics::float3 const& value);
        winrt::Windows::Foundation::Numerics::float3 RedPrimary();
        void RedPrimary(winrt::Windows::Foundation::Numerics::float3 const& value);
        winrt::Windows::Foundation::Numerics::float3 GreenPrimary();
        void GreenPrimary(winrt::Windows::Foundation::Numerics::float3 const& value);
        winrt::Windows::Foundation::Numerics::float3 BluePrimary();
        void BluePrimary(winrt::Windows::Foundation::Numerics::float3 const& value);
        float FullFrameLuminance();
        void FullFrameLuminance(float value);
        float MinLuminance();
        void MinLuminance(float value);
        float PeakLuminance();
        void PeakLuminance(float value);
        winrt::Windows::Foundation::Numerics::float4x4 CscMatrix();
        void CscMatrix(winrt::Windows::Foundation::Numerics::float4x4 value);
        com_array<winrt::Windows::Foundation::Numerics::float3> ReGammaLuts();
        void ReGammaLuts(array_view<winrt::Windows::Foundation::Numerics::float3 const> value);

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
