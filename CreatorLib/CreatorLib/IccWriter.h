#pragma once
#include "IccWriter.g.h"

namespace winrt::CreatorLib::implementation
{
    inline uint32_t FixEndian(uint32_t value)
    {
        return _byteswap_ulong(value);
    }

    inline uint16_t FixEndian16(uint16_t value)
    {
        return _byteswap_ushort(value);
    }

    using unique_hprofile = wil::unique_any<HPROFILE, decltype(&::CloseColorProfile), ::CloseColorProfile>;

    struct Tag
    {
        Tag(const std::vector<uint8_t>& data)
            : signature(FixEndian(*reinterpret_cast<const uint32_t*>(data.data())))
            , size(data.size())
            , rawData(data)
        {
		}
		const uint32_t signature;
        const uint32_t size;
        std::vector<uint8_t> rawData;
	};

    struct IccWriter : IccWriterT<IccWriter>
    {
        IccWriter();

        winrt::Windows::Foundation::IAsyncAction CommitToFileAsync(winrt::Windows::Storage::StorageFile file);
        hstring profileDescription();
        void profileDescription(hstring const& value);
        hstring profileCopyright();
        void profileCopyright(hstring const& value);
        winrt::CreatorLib::XYZ whitePoint();
        void whitePoint(winrt::CreatorLib::XYZ const& value);
        winrt::CreatorLib::XYZ redPrimary();
        void redPrimary(winrt::CreatorLib::XYZ const& value);
        winrt::CreatorLib::XYZ greenPrimary();
        void greenPrimary(winrt::CreatorLib::XYZ const& value);
        winrt::CreatorLib::XYZ bluePrimary();
        void bluePrimary(winrt::CreatorLib::XYZ const& value);
        float fullFrameLuminance();
        void fullFrameLuminance(float value);
        float minLuminance();
        void minLuminance(float value);
        float peakLuminance();
        void peakLuminance(float value);

    private:
        // Profile
        unique_hprofile m_hProfile = nullptr;

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
