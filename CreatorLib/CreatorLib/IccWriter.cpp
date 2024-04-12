#include "pch.h"
#include "IccWriter.h"
#include "IccWriter.g.cpp"
#include "template.icm.h"
#include "WindowsNumerics.h"

namespace winrt
{
    using namespace winrt::Windows::Foundation::Numerics;
}

namespace winrt::CreatorLib::implementation
{
    // Static helpers
    static inline uint32_t FixEndian(uint32_t value)
    {
        return _byteswap_ulong(value);
    }

    static inline uint16_t FixEndian16(uint16_t value)
    {
        return _byteswap_ushort(value);
    }

    static inline uint32_t FloatToS15Fixed16Number(float value)
    {
        int s15 = static_cast<int>(floorf(value));
        int f16 = static_cast<int>((value - s15) * 65536.f);

        return FixEndian(static_cast<uint32_t>((s15 << 16) | f16));
    }

    static inline float S15Fixed16NumberToFloat(uint32_t value)
    {
        auto valueFixed = FixEndian(value);
        int s15 = static_cast<int>(valueFixed >> 16);
        int f16 = static_cast<int>(valueFixed & 0xFFFF);

        return static_cast<float>(s15) + static_cast<float>(f16) / 65536.f;
    }

    constexpr winrt::Windows::Foundation::Numerics::float4x4 chromaticAdaptation = winrt::Windows::Foundation::Numerics::float4x4(
        1.0479073817101700, 0.0229333845542104, -0.0502016347980104, 0,
        0.0296059594177168, 0.9904560399107850, -0.0170755291958700, 0,
        -0.0092467943267824, 0.0150626801401488, 0.7517912326090780, 0,
        0, 0, 0, 1);

    static inline winrt::Windows::Foundation::Numerics::float3 AddChad(winrt::Windows::Foundation::Numerics::float3 xyz)
    {

        return winrt::Windows::Foundation::Numerics::float3(
            chromaticAdaptation.m11 * xyz.x + chromaticAdaptation.m12 * xyz.y + chromaticAdaptation.m13 * xyz.z,
            chromaticAdaptation.m21 * xyz.x + chromaticAdaptation.m22 * xyz.y + chromaticAdaptation.m23 * xyz.z,
            chromaticAdaptation.m31 * xyz.x + chromaticAdaptation.m32 * xyz.y + chromaticAdaptation.m33 * xyz.z);
    }

    static inline winrt::Windows::Foundation::Numerics::float3 RemoveChad(winrt::Windows::Foundation::Numerics::float3 xyz)
    {
        winrt::Windows::Foundation::Numerics::float4x4 inv;
        invert(chromaticAdaptation, &inv);
        return winrt::Windows::Foundation::Numerics::float3(
            inv.m11 * xyz.x + inv.m12 * xyz.y + inv.m13 * xyz.z,
            inv.m21 * xyz.x + inv.m22 * xyz.y + inv.m23 * xyz.z,
            inv.m31 * xyz.x + inv.m32 * xyz.y + inv.m33 * xyz.z);
    }

    std::vector<uint8_t> Float3ToXYZ(const float3& value)
    {
        auto xyz = std::vector<uint8_t>(20, 0);

        uint32_t* tagSignature = reinterpret_cast<uint32_t*>(&xyz[0]);
        *tagSignature = FixEndian('XYZ ');

        uint32_t* X = reinterpret_cast<uint32_t*>(&xyz[8]);
        *X = FloatToS15Fixed16Number(value.x);
        uint32_t* Y = reinterpret_cast<uint32_t*>(&xyz[12]);
        *Y = FloatToS15Fixed16Number(value.y);
        uint32_t* Z = reinterpret_cast<uint32_t*>(&xyz[16]);
        *Z = FloatToS15Fixed16Number(value.z);

        return xyz;
    }
    float3 XYZToFloat3(const std::vector<uint8_t>& data)
    {
        if (data.size() != 20)
        {
            throw hresult_error(E_INVALIDARG, L"Invalid XYZ data size");
        }

        uint32_t* X = reinterpret_cast<uint32_t*>(const_cast<uint8_t*>(data.data()) + 8);
        uint32_t* Y = reinterpret_cast<uint32_t*>(const_cast<uint8_t*>(data.data()) + 12);
        uint32_t* Z = reinterpret_cast<uint32_t*>(const_cast<uint8_t*>(data.data()) + 16);

        return float3(S15Fixed16NumberToFloat(*X), S15Fixed16NumberToFloat(*Y), S15Fixed16NumberToFloat(*Z));
    }

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
    hstring IccWriter::ProfileDescription()
    {
        std::scoped_lock lock(m_lock);

        // Read the profile description
        auto description = GetTagData('desc'); // desc
        std::wstring descriptionString(description.begin() + 12, description.end());

        return hstring(descriptionString);
    }
    void IccWriter::ProfileDescription(hstring const& value)
    {
        std::scoped_lock lock(m_lock);

        // Write the profile description (only the ASCII part - leaving Unicode and ScriptCode blank)
        std::vector<uint8_t> descriptionString(value.size());
        descriptionString.insert(descriptionString.begin(), 12, 0);
        std::copy(value.begin(), value.end(), descriptionString.begin() + 12);

        // Insert the null terminator
        descriptionString.insert(descriptionString.end(), 1, 0);

        // Insert the null Unicode section of 4 bytes at the end
        descriptionString.insert(descriptionString.end(), 4, 0); // The region code
        descriptionString.insert(descriptionString.end(), 4, 0); // The length

        // Insert the null ScriptCode section of 4 bytes at the end
        descriptionString.insert(descriptionString.end(), 2, 0);  // The region code
        descriptionString.insert(descriptionString.end(), 1, 0);  // The length
        descriptionString.insert(descriptionString.end(), 67, 0); // The blank 67 bytes of the empty ScriptCode section

        uint32_t* tagSignature = reinterpret_cast<uint32_t*>(&descriptionString[0]);
        *tagSignature = FixEndian('desc');

        uint32_t* asciiLength = reinterpret_cast<uint32_t*>(&descriptionString[8]);
        *asciiLength = FixEndian(value.size() + 1);

        SetTagData('desc', std::vector<uint8_t>(descriptionString.begin(), descriptionString.end()));
    }

    hstring IccWriter::ProfileCopyright()
    {
        std::scoped_lock lock(m_lock);

        // Read the profile description
        auto cprt = GetTagData('cprt'); // desc
        std::wstring cprtString(cprt.begin() + 8, cprt.end());

        return hstring(cprtString);
    }
    void IccWriter::ProfileCopyright(hstring const& value)
    {
        std::scoped_lock lock(m_lock);

		std::vector<uint8_t> cprtString(value.size());
		cprtString.insert(cprtString.begin(), 8, 0);
		std::copy(value.begin(), value.end(), cprtString.begin() + 8);

		// Insert the null terminator
		cprtString.insert(cprtString.end(), 1, 0);

		uint32_t* tagSignature = reinterpret_cast<uint32_t*>(&cprtString[0]);
		*tagSignature = FixEndian('text');

		SetTagData('cprt', std::vector<uint8_t>(cprtString.begin(), cprtString.end()));
    }
    winrt::float3 IccWriter::WhitePoint()
    {
        std::scoped_lock lock(m_lock);

        auto xyz = GetTagData('wtpt');

        // Chromatic Adaptation
        return RemoveChad(XYZToFloat3(xyz));
    }
    void IccWriter::WhitePoint(winrt::float3 const& value)
    {
        std::scoped_lock lock(m_lock);

        SetTagData('wtpt', Float3ToXYZ(AddChad(value)));
    }
    winrt::float3 IccWriter::RedPrimary()
    {
        std::scoped_lock lock(m_lock);

        auto xyz = GetTagData('rXYZ');
        return RemoveChad(XYZToFloat3(xyz));
    }
    void IccWriter::RedPrimary(winrt::float3 const& value)
    {
        std::scoped_lock lock(m_lock);

        SetTagData('rXYZ', Float3ToXYZ(AddChad(value)));
    }
    winrt::float3 IccWriter::GreenPrimary()
    {
        std::scoped_lock lock(m_lock);

        auto xyz = GetTagData('gXYZ');
        return RemoveChad(XYZToFloat3(xyz));
    }
    void IccWriter::GreenPrimary(winrt::float3 const& value)
    {
        std::scoped_lock lock(m_lock);

        SetTagData('gXYZ', Float3ToXYZ(AddChad(value)));
    }
    winrt::float3 IccWriter::BluePrimary()
    {
        std::scoped_lock lock(m_lock);

        auto xyz = GetTagData('bXYZ');
        return RemoveChad(XYZToFloat3(xyz));
    }
    void IccWriter::BluePrimary(winrt::float3 const& value)
    {
        std::scoped_lock lock(m_lock);

        SetTagData('bXYZ', Float3ToXYZ(AddChad(value)));
    }
    float IccWriter::FullFrameLuminance()
    {
        std::scoped_lock lock(m_lock);

        auto xyz = GetTagData('lumi');
        return XYZToFloat3(xyz).y;
    }
    void IccWriter::FullFrameLuminance(float value)
    {
        std::scoped_lock lock(m_lock);

        SetTagData('lumi', Float3ToXYZ(float3(0,value,0)));
    }
    float IccWriter::MinLuminance()
    {
        std::scoped_lock lock(m_lock);

        auto mhc2 = GetTagData('MHC2');
        uint32_t minLuminance = *reinterpret_cast<uint32_t*>(mhc2.data() + 12);
        return S15Fixed16NumberToFloat(minLuminance);
    }
    void IccWriter::MinLuminance(float value)
    {
        std::scoped_lock lock(m_lock);

        auto mhc2 = GetTagData('MHC2');
        uint32_t* minLuminance = reinterpret_cast<uint32_t*>(mhc2.data() + 12);
        *minLuminance = FloatToS15Fixed16Number(value);

        SetTagData('MHC2', mhc2);
    }
    float IccWriter::PeakLuminance()
    {
        std::scoped_lock lock(m_lock);

        auto mhc2 = GetTagData('MHC2');
        uint32_t maxLuminance = *reinterpret_cast<uint32_t*>(mhc2.data() + 16);
        return S15Fixed16NumberToFloat(maxLuminance);
    }
    void IccWriter::PeakLuminance(float value)
    {
        std::scoped_lock lock(m_lock);

        auto mhc2 = GetTagData('MHC2');
        uint32_t* maxLuminance = reinterpret_cast<uint32_t*>(mhc2.data() + 16);
        *maxLuminance = FloatToS15Fixed16Number(value);

        SetTagData('MHC2', mhc2);
    }
    winrt::float4x4 IccWriter::CscMatrix()
    {
        std::scoped_lock lock(m_lock);

        auto mhc2 = GetTagData('MHC2');
        uint32_t* matrix = reinterpret_cast<uint32_t*>(mhc2.data() + 36);

        return winrt::float4x4(
			S15Fixed16NumberToFloat(matrix[0]), S15Fixed16NumberToFloat(matrix[1]), S15Fixed16NumberToFloat(matrix[2]), 0,
			S15Fixed16NumberToFloat(matrix[4]), S15Fixed16NumberToFloat(matrix[5]), S15Fixed16NumberToFloat(matrix[6]), 0,
			S15Fixed16NumberToFloat(matrix[8]), S15Fixed16NumberToFloat(matrix[9]), S15Fixed16NumberToFloat(matrix[10]), 0,
			0, 0, 0, 1);
    }
    void IccWriter::CscMatrix(winrt::float4x4 value)
    {
        std::scoped_lock lock(m_lock);

        auto mhc2 = GetTagData('MHC2');
		uint32_t* matrix = reinterpret_cast<uint32_t*>(mhc2.data() + 36);
        matrix[0]  = FloatToS15Fixed16Number(value.m11);
        matrix[1]  = FloatToS15Fixed16Number(value.m12);
        matrix[2]  = FloatToS15Fixed16Number(value.m13);

		matrix[4]  = FloatToS15Fixed16Number(value.m21);
        matrix[5]  = FloatToS15Fixed16Number(value.m22);
        matrix[6]  = FloatToS15Fixed16Number(value.m23);

        matrix[8]  = FloatToS15Fixed16Number(value.m31);
		matrix[9]  = FloatToS15Fixed16Number(value.m32);
        matrix[10] = FloatToS15Fixed16Number(value.m33);

        SetTagData('MHC2', mhc2);
    }
    com_array<winrt::float3> IccWriter::ReGammaLuts()
    {
        std::scoped_lock lock(m_lock);
        std::vector<winrt::float3> luts;

        auto mhc2 = GetTagData('MHC2');
        uint32_t lutSize = FixEndian(*reinterpret_cast<uint32_t*>(mhc2.data() + 8));

        luts.insert(luts.end(), lutSize, winrt::float3(0, 0, 0));

        const uint32_t redOffset   = 0x54 + 8;
        const uint32_t greenOffset = redOffset   + 8 + (lutSize * 4);
        const uint32_t blueOffset  = greenOffset + 8 + (lutSize * 4);

        // Read the first Lut (red)
        for (auto i = 0; i < lutSize; i++)
		{
			uint32_t value = *reinterpret_cast<uint32_t*>(mhc2.data() + i * 4 + redOffset);
			luts[i].x = S15Fixed16NumberToFloat(value);
		}

        // Read the second Lut (green)
        for (auto i = 0; i < lutSize; i++)
        {
            uint32_t value = *reinterpret_cast<uint32_t*>(mhc2.data() + i * 4 + greenOffset);
            luts[i].y = S15Fixed16NumberToFloat(value);
        }

        // Read the third Lut (blue)
        for (auto i = 0; i < lutSize; i++)
        {
            uint32_t value = *reinterpret_cast<uint32_t*>(mhc2.data() + i * 4 + blueOffset);
            luts[i].z = S15Fixed16NumberToFloat(value);
        }

        return com_array<winrt::float3>(luts.begin(), luts.end());
    }
    void IccWriter::ReGammaLuts(array_view<winrt::float3 const> value)
    {
        std::scoped_lock lock(m_lock);

        auto mhc2 = GetTagData('MHC2');

        // Ensure that the buffer is big enough
        uint32_t redLutOffset = 0x54;
        uint32_t greenLutOffset = redLutOffset + 8 + (value.size() * 4);
        uint32_t blueLutOffset = greenLutOffset + 8 + (value.size() * 4);
        uint32_t expand = max(mhc2.size(), blueLutOffset + 8 + (value.size() * 4)) - mhc2.size();
        mhc2.insert(mhc2.end(), expand, 0);

        uint32_t* lutSize = reinterpret_cast<uint32_t*>(mhc2.data() + 8);
        *lutSize = FixEndian(static_cast<uint32_t>(value.size()));

        uint32_t* redOffset = reinterpret_cast<uint32_t*>(mhc2.data() + 24);
        uint32_t* greenOffset = reinterpret_cast<uint32_t*>(mhc2.data() + 28);
        uint32_t* blueOffset = reinterpret_cast<uint32_t*>(mhc2.data() + 32);

        *redOffset   = FixEndian(redLutOffset);
        *greenOffset = FixEndian(greenLutOffset);
        *blueOffset  = FixEndian(blueLutOffset);

        uint32_t* redLut = reinterpret_cast<uint32_t*>(mhc2.data() + redLutOffset);
        uint32_t* greenLut = reinterpret_cast<uint32_t*>(mhc2.data() + greenLutOffset);
        uint32_t* blueLut = reinterpret_cast<uint32_t*>(mhc2.data() + blueLutOffset);

        redLut[0] = FixEndian('sf32');
        redLut[1] = 0;
        greenLut[0] = FixEndian('sf32');
        greenLut[1] = 0;
        blueLut[0] = FixEndian('sf32');
        blueLut[1] = 0;

		for (auto i = 2; i < value.size() + 2; i++)
		{
            redLut[i] = FloatToS15Fixed16Number(value[i - 2].x);
            greenLut[i] = FloatToS15Fixed16Number(value[i - 2].y);
            blueLut[i] = FloatToS15Fixed16Number(value[i - 2].z);
		}

		SetTagData('MHC2', mhc2);
    }

    // Private profile interaction functions
    std::vector<uint8_t> IccWriter::GetTagData(uint32_t tagSignature)
    {
        DWORD cbSize = 0;
        BOOL reference = FALSE;

        try
        {

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
		catch (...)
		{
			return std::vector<uint8_t>();
		}
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
