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
        // Modifiable properties
        hstring m_profileDescription = L"";
		hstring m_profileCopyright = L"";
        CreatorLib::XYZ m_whitePoint = {};
		CreatorLib::XYZ m_redPrimary = {};
		CreatorLib::XYZ m_greenPrimary = {};
		CreatorLib::XYZ m_bluePrimary = {};
		float m_fullFrameLuminance = 0;
		float m_minLuminance = 0;
		float m_peakLuminance = 0;

        // Profile
        unique_hprofile m_hProfile = nullptr;
    };
}
namespace winrt::CreatorLib::factory_implementation
{
    struct IccWriter : IccWriterT<IccWriter, implementation::IccWriter>
    {
    };
}
