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
			throw hresult_error(E_FAIL, L"Failed to open the template profile");
		}

        // Read the profile description
		DWORD cbSize = 0;
        BOOL reference = FALSE;

        GetColorProfileElement(m_hProfile.get(), 'desc', 0, &cbSize, nullptr, &reference);

        if (cbSize == 0)
        {
			throw hresult_error(E_FAIL, L"Failed to get the size of the profile description");
		}

		std::vector<char> description(cbSize / sizeof(char));
        if (FALSE == GetColorProfileElement(m_hProfile.get(), 'desc', 0, &cbSize, reinterpret_cast<PDWORD>(description.data()), &reference))
        {
			throw hresult_error(E_FAIL, L"Failed to get the profile description");
		}

        std::wstring descriptionString(description.begin()+12, description.end());

        m_profileDescription = descriptionString;
	}
    winrt::Windows::Foundation::IAsyncAction IccWriter::CommitToFileAsync(winrt::Windows::Storage::StorageFile file)
    {
        throw hresult_not_implemented();
    }
    hstring IccWriter::profileDescription()
    {
        throw hresult_not_implemented();
    }
    void IccWriter::profileDescription(hstring const& value)
    {
        throw hresult_not_implemented();
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
}
