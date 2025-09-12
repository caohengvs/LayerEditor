#include "pch.h"
#include "CHfFeCommonPcie.h"
#include "DriverDefine.h"

static const GUID HF_PCIE_GUID = {0x536db260, 0x70ee, 0x451e, 0xb9, 0x61, 0x56, 0x70, 0xb7, 0xbd, 0xb3, 0xb0};

CHfFeCommonPcie::CHfFeCommonPcie()
    : m_hDevice(NULL)
{
}

bool CHfFeCommonPcie::Open()
{
    if (enumDevice() <= 0)
    {
        return false;
    }

    return openDevice();
}

void CHfFeCommonPcie::Close()
{
    if (m_hDevice && m_hDevice != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
    }
}

bool CHfFeCommonPcie::ReadRegister(uint16_t nMemoryBarID, uint32_t nAddress, uint64_t& nValue)
{
    if (m_hDevice == NULL || m_hDevice == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    HF_PCIE_RW_PARAM oReadParam = {};
    oReadParam.nMemoryBarID = nMemoryBarID;
    oReadParam.nAddressOffset = nAddress;
    oReadParam.nLength = sizeof(DWORD);

    DWORD dwResult;

    if (DeviceIoControl(m_hDevice, HF_PCIE_IOCTL_READ_MEMERY_BAR, &oReadParam, sizeof(HF_PCIE_RW_PARAM), &nValue,
                        sizeof(DWORD), &dwResult, NULL) == FALSE)
    {
        return false;
    }

    return true;
}

bool CHfFeCommonPcie::WriteRegister(uint16_t nMemoryBarID, uint32_t nAddress, uint64_t nValue)
{
    if (m_hDevice == NULL || m_hDevice == INVALID_HANDLE_VALUE)
        return false;

    HF_PCIE_RW_PARAM oWriteParam = {};
    oWriteParam.nMemoryBarID = nMemoryBarID;
    oWriteParam.nAddressOffset = nAddress;
    oWriteParam.nLength = sizeof(DWORD);

    DWORD dwResult;

    if (DeviceIoControl(m_hDevice, HF_PCIE_IOCTL_SET_WRITE_MEMERY_BAR_PARAM, &oWriteParam, sizeof(HF_PCIE_RW_PARAM),
                        nullptr, 0, &dwResult, NULL) == FALSE)
    {
        return false;
    }

    if (DeviceIoControl(m_hDevice, HF_PCIE_IOCTL_WRITE_MEMERY_BAR, &nValue, sizeof(DWORD), nullptr, 0, &dwResult,
                        NULL) == FALSE)
    {
        return false;
    }

    return true;
}

uint32_t CHfFeCommonPcie::GetBuffer(void* pBuf, uint32_t nSize)
{
    if (m_hDevice == NULL || m_hDevice == INVALID_HANDLE_VALUE)
        return false;

    DWORD dwResult;

    if (DeviceIoControl(m_hDevice, HF_PCIE_IOCTL_GET_PHY_MEMORY_BUF_DATA, &nSize, sizeof(UINT), pBuf, nSize, &dwResult,
                        NULL) == FALSE)
    {
        return false;
    }

    return dwResult;
}

bool CHfFeCommonPcie::Init(uint16_t nMaxCacheNodesNumber)
{
    if (m_hDevice == NULL || m_hDevice == INVALID_HANDLE_VALUE)
        return false;

    DWORD dwResult = 0;
    if (DeviceIoControl(m_hDevice, HF_PCIE_IOCTL_INITIALIZATE, &nMaxCacheNodesNumber, sizeof(UINT), NULL, 0, &dwResult,
                        NULL) == FALSE)
    {
        return false;
    }

    return true;
}

int CHfFeCommonPcie::Reset()
{
    if (m_hDevice == NULL || m_hDevice == INVALID_HANDLE_VALUE)
        return -1;

    DWORD dwResult = 0;
    UINT nMemoryBarsNumber = 0;
    if (DeviceIoControl(m_hDevice, HF_PCIE_IOCTL_GET_MEMORY_BARS_NUMBER, nullptr, 0, &nMemoryBarsNumber, sizeof(UINT),
                        &dwResult, NULL) == FALSE)
    {
        return -1;
    }

    return (int)nMemoryBarsNumber;
}

int CHfFeCommonPcie::enumDevice()
{
    CString strDeviceDetail;

    HDEVINFO hDevInfoSet = NULL;
    SP_DEVICE_INTERFACE_DATA ifdata = {};
    PSP_DEVICE_INTERFACE_DETAIL_DATA pDetail = nullptr;
    int nCount = 0;
    BOOL bResult = 0;
    const GUID* lpGuid = &HF_PCIE_GUID;

    m_arrDevicePath.clear();

    // 取得一个该GUID相关的设备信息集句柄
    hDevInfoSet = ::SetupDiGetClassDevs(lpGuid,                                  // class GUID
                                        NULL,                                    // 无关键字
                                        NULL,                                    // 不指定父窗口句柄
                                        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);  // 目前存在的设备

    // 失败...
    if (hDevInfoSet == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT, HF_PCIE_INTERFACE_DETAIL_SIZE);
    if (pDetail == NULL)
    {
        return 0;
    }

    pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    nCount = 0;
    bResult = TRUE;

    while (bResult)
    {
        ifdata.cbSize = sizeof(ifdata);

        // 枚举符合该GUID的设备接口
        bResult = ::SetupDiEnumDeviceInterfaces(hDevInfoSet,    // 设备信息集句柄
                                                NULL,           // 不需额外的设备描述
                                                lpGuid,         // GUID
                                                (ULONG)nCount,  // 设备信息集里的设备序号
                                                &ifdata);       // 设备接口信息

        if (bResult)
        {
            // 取得该设备接口的细节(设备路径)
            bResult = SetupDiGetInterfaceDeviceDetail(hDevInfoSet,                    // 设备信息集句柄
                                                      &ifdata,                        // 设备接口信息
                                                      pDetail,                        // 设备接口细节(设备路径)
                                                      HF_PCIE_INTERFACE_DETAIL_SIZE,  // 输出缓冲区大小
                                                      NULL,   // 不需计算输出缓冲区大小(直接用设定值)
                                                      NULL);  // 不需额外的设备描述

            if (bResult)
            {
                // 复制设备路径到输出缓冲区
                strDeviceDetail = pDetail->DevicePath;
                m_arrDevicePath.push_back(strDeviceDetail);
                // 调整计数值
                nCount++;
            }
        }
    }

    // 释放设备接口数据空间
    ::GlobalFree(pDetail);

    // 关闭设备信息集句柄
    ::SetupDiDestroyDeviceInfoList(hDevInfoSet);

    return nCount;
}

bool CHfFeCommonPcie::openDevice()
{
    Close();

    for (const auto& path : m_arrDevicePath)
    {
        m_hDevice = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

        if (m_hDevice != INVALID_HANDLE_VALUE)
            break;
    }

    if (m_hDevice == INVALID_HANDLE_VALUE)
        return false;

    return true;
}
