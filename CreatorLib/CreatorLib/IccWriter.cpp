#include "pch.h"
#include "IccWriter.h"
#include "IccWriter.g.cpp"
#include "template.icm.h"

namespace winrt::CreatorLib::implementation
{
    IccWriter::IccWriter()
    {
        std::vector<uint8_t> templateData(template_icm, template_icm + sizeof(template_icm) / sizeof(template_icm[0]));

        PROFILE profile = {};
        profile.dwType = PROFILE_MEMBUFFER;
        profile.pProfileData = templateData.data();
        profile.cbDataSize = templateData.size();

        m_hProfile.reset(OpenColorProfileW(&profile, PROFILE_READWRITE, FILE_SHARE_WRITE, OPEN_EXISTING));

        if (!m_hProfile)
        {
			throw hresult_error(HRESULT_FROM_WIN32(GetLastError()), L"Failed to open the template profile");
        }
    }
    winrt::Windows::Foundation::IAsyncAction IccWriter::CommitToFileAsync(winrt::Windows::Storage::StorageFile file)
    {
        auto stream = co_await file.OpenAsync(Windows::Storage::FileAccessMode::ReadWrite);
        auto outputStream = stream.GetOutputStreamAt(0);
        auto dataWriter = Windows::Storage::Streams::DataWriter(outputStream);

        DWORD cbSize = 0;
        if (FALSE == GetColorProfileFromHandle(m_hProfile.get(), nullptr, &cbSize) && cbSize == 0)
        {
            throw hresult_error(HRESULT_FROM_WIN32(GetLastError()), L"Failed to get the size of the profile");
        }

        std::vector<uint8_t> profileData(cbSize);

        if (FALSE == GetColorProfileFromHandle(m_hProfile.get(), profileData.data(), &cbSize))
        {
            throw hresult_error(HRESULT_FROM_WIN32(GetLastError()), L"Failed to get the profile buffer");
        }

        dataWriter.WriteBytes(array_view<uint8_t const>(profileData));

        co_await dataWriter.StoreAsync();

        co_return;
    }
    hstring IccWriter::profileDescription()
    {
        // Read the profile description
        auto description = GetTagData('desc'); // desc
        std::wstring descriptionString(description.begin() + 12, description.end());

        return hstring(descriptionString);
    }
    void IccWriter::profileDescription(hstring const& value)
    {
        // Write the profile description
        std::vector<uint8_t> descriptionString(value.size());
        descriptionString.insert(descriptionString.begin(), 12, 0);
        std::copy(value.begin(), value.end(), descriptionString.begin() + 12);

        uint32_t* tagSignature = reinterpret_cast<uint32_t*>(&descriptionString[0]);
        *tagSignature = FixEndian('desc');

        uint32_t* tagLength = reinterpret_cast<uint32_t*>(&descriptionString[8]);
        *tagLength = FixEndian(descriptionString.size());

        SetTagData('desc', std::vector<uint8_t>(descriptionString.begin(), descriptionString.end()));
    }

    hstring IccWriter::profileCopyright()
    {
        throw hresult_not_implemented();
    }
    void IccWriter::profileCopyright(hstring const& value)
    {
        throw hresult_not_implemented();
    }
    winrt::CreatorLib::XYZ IccWriter::whitePoint()
    {
        throw hresult_not_implemented();
    }
    void IccWriter::whitePoint(winrt::CreatorLib::XYZ const& value)
    {
        throw hresult_not_implemented();
    }
    winrt::CreatorLib::XYZ IccWriter::redPrimary()
    {
        throw hresult_not_implemented();
    }
    void IccWriter::redPrimary(winrt::CreatorLib::XYZ const& value)
    {
        throw hresult_not_implemented();
    }
    winrt::CreatorLib::XYZ IccWriter::greenPrimary()
    {
        throw hresult_not_implemented();
    }
    void IccWriter::greenPrimary(winrt::CreatorLib::XYZ const& value)
    {
        throw hresult_not_implemented();
    }
    winrt::CreatorLib::XYZ IccWriter::bluePrimary()
    {
        throw hresult_not_implemented();
    }
    void IccWriter::bluePrimary(winrt::CreatorLib::XYZ const& value)
    {
        throw hresult_not_implemented();
    }
    float IccWriter::fullFrameLuminance()
    {
        throw hresult_not_implemented();
    }
    void IccWriter::fullFrameLuminance(float value)
    {
        throw hresult_not_implemented();
    }
    float IccWriter::minLuminance()
    {
        throw hresult_not_implemented();
    }
    void IccWriter::minLuminance(float value)
    {
        throw hresult_not_implemented();
    }
    float IccWriter::peakLuminance()
    {
        throw hresult_not_implemented();
    }
    void IccWriter::peakLuminance(float value)
    {
        throw hresult_not_implemented();
    }
    std::vector<uint8_t> IccWriter::GetTagData(uint32_t tagSignature)
    {
        DWORD cbSize = 0;
        BOOL reference = FALSE;

        GetColorProfileElement(m_hProfile.get(), tagSignature, 0, &cbSize, nullptr, &reference);

        if (cbSize == 0)
        {
            throw hresult_error(HRESULT_FROM_WIN32(GetLastError()), L"Failed to get the size of the tag");
        }

        std::vector<uint8_t> rawData(cbSize / sizeof(uint8_t));
        if (FALSE == GetColorProfileElement(m_hProfile.get(), tagSignature, 0, &cbSize, reinterpret_cast<PDWORD>(rawData.data()), &reference))
        {
            throw hresult_error(HRESULT_FROM_WIN32(GetLastError()), L"Failed to get the tag");
        }

        return rawData;
    }
    void IccWriter::SetTagData(uint32_t tagSignature, std::vector<uint8_t>& data)
    {
        DWORD cbSize = data.size();
        BOOL reference = FALSE;

        if (FALSE == SetColorProfileElementSize(m_hProfile.get(), tagSignature, cbSize))
        {
            throw hresult_error(HRESULT_FROM_WIN32(GetLastError()), L"Failed to set the tag size");
        }

        if (FALSE == SetColorProfileElement(m_hProfile.get(), tagSignature, 0, &cbSize, data.data()))
        {
            throw hresult_error(HRESULT_FROM_WIN32(GetLastError()), L"Failed to set the tag");
		}
    }
}
