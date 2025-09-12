#pragma once
DEFINE_GUID(GUID_DEVINTERFACE_HuafengPciEDriver, 0x536db260, 0x70ee, 0x451e, 0xb9, 0x61, 0x56, 0x70, 0xb7, 0xbd, 0xb3,
            0xb0);
// {536db260-70ee-451e-b961-5670b7bdb3b0}

#define HF_PCIE_INTERFACE_DETAIL_SIZE 1024  //所需要的输出长度，定义足够大

//////////////////////////////////////////////////////////////////////////
// 宏定义
// 1. 通用定义
// 无效的地址
#define HF_PCIE_INVALID_ADDRESS 0xffffffff
// 无效的值
#define HF_PCIE_INVALID_VALUE 0xffffffff

// I/O操作时，没有限制请求长度
#define HF_PCIE_IO_NO_REQ_LENGTH 0

// 2. Memery Bar定义
// 读/写MemeryBar的地址和字节数的粒度
#define HF_PCIE_MEMERY_BAR_UNIT 4

// 3. DDR定义
// 标记读/写DDR地址和长度单位的粒度
#define HF_PCIE_DDR_UNIT 64
// 定义DDR的长度(4GB)
#define HF_PCIE_DDR_LENGTH (4LL * 1024 * 1024 * 1024)

// 4.DMA相关
// dma common buffer地址对齐（DWORD)
#define HF_DMA_ALIGNMENT_REQUIREMENT FILE_LONG_ALIGNMENT
// DMA单次传输的最大长度64KB
#define HF_DMA_MAX_TRANSFER_LENGTH (64 * 1024)
// Common Buffer长度
#define HF_DMA_COMMON_BUFFER_LENGTH (1024 * 1024 * 8)
// DMA descroptor buf长度
#define HF_DMA_DECROPTOR_BUFFER_LENGTH (608 + 32 * 125)
// DMA status position
#define HF_DMA_STATUS_POSITION 127
#define HF_DMA_DATA_POOL_COEF 2
#define HF_PCIE_DMA_ISR_RGEISTER_ADDRESS 0x00000020  // DMA中断寄存器地址

#define HF_PCIE_DMA_ISR_ENABLE_BUFFERA 0x00000050  // DMA中断使能寄存器地址A
#define HF_PCIE_DMA_ISR_ENABLE_BUFFERB 0x00000054  // DMA中断使能寄存器地址B

#define HF_DMA_UPLOAD_BUFFER_COUNT 16  // 数据上传缓冲区数量

#define MAX_MSI_INTERRUPTS 16  // 最大MSI中断数量

#define HF_BUFFER_STATUS_EMPTY 0x00000000  // DMA缓冲区状态空
#define HF_BUFFER_STATUS_INUSE 0x00000001  // DMA缓冲区状态空
#define HF_BUFFER_STATUS_FULL 0x00000002   // DMA缓冲区状态空
//////////////////////////////////////////////////////////////////////////
// 枚举定义
// 定义DMA的操作对象
typedef enum _ENUM_HF_PCIE_DMA_ACCESS_
{
    eHfPciE_DMAAccess_DDR_RW,
    eHfPciE_DMAAccess_DataUpload
} EHfPciE_DmaAccess;

// 定义读/写操作的目标
typedef enum _ENUM_HF_PCIE_RW_DEST
{
    eHfPciE_RWDest_Memory,
    eHfPciE_RWDest_DDR
} EHfPciE_RWDest;

//////////////////////////////////////////////////////////////////////////
// 定义结构体
// 定义读/写MEMERY BAR/DDR的参数的结构体
typedef struct _STRUCT_HF_PCIE_READ_WRITE_ADDRESS_PARAM_
{
    ULONG32 nMemoryBarID;    // MemoryBar ID
    ULONG32 nAddressOffset;  // 地址偏移量
    ULONG32 nLength;         // 写入或读取数据的大小
} HF_PCIE_RW_PARAM, *PHF_PCIE_RW_PARAM;

// 定义保存丢弃数据包的结构
typedef struct _STRUCT_HF_PCIE_DISCARD_PACKAGE_
{
    ULONG32 nDiscardNumberByHW;      // 被硬件丢弃的数据包
    ULONG32 nDiscardNumberByDriver;  // 被驱动丢弃的数据包
} HF_PCIE_DISCARD_PACKAGE, *PHF_PCIE_DISCARD_PACKAGE;

//////////////////////////////////////////////////////////////////////////
// 定义DeviceIOCtrl使用的编码

// 1.初始化操作
// 提供初始化时时操作,该操作将分配缓冲区，并复位所有寄存器
// InputBuffer:  ULONG32指针, 用于保存DataUp链表节点的数量
// OutputBuffer: 无
#define HF_PCIE_IOCTL_INITIALIZATE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x001, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 2.复位操作
// 复位操作，复位所有寄存器，清空所有未获取的数据，将DMA设置到DDR方向
// InputBuffer: 无
// OutputBuffer:无
#define HF_PCIE_IOCTL_SOFTWARE_RESET CTL_CODE(FILE_DEVICE_UNKNOWN, 0x002, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 3. 执行读取MEMERYBAR操作
// 执行从MEMERYBAR中读取内存地址的操作
// InputBuffer:BS_X1_RW_PARAM结构体指针，包括读取的地址和长度
// OutputBuffer:保存数据的地址指针
#define HF_PCIE_IOCTL_READ_MEMERY_BAR CTL_CODE(FILE_DEVICE_UNKNOWN, 0x003, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

// 4. 设置写入MEMERYBAR数据的参数
// 设置下一个DeviceIOCtrl操作将要写入的的内存地址和长度
// 由于考虑32/64位操作系统的问题，在参数结构体中尽量不使用指针型变量，因此将写入操作分解为设置参数和执行写操作两步
// 读操作完成后，将解除对其它读参数设置操作的挂起
// 寄存器地址必须4字节对齐，因此地址偏移量以及读取的长度必须是4字节的整数倍
// InputBuffer:一个HF_PCIE_RW_PARAM指针
// OutputBuffer:无
#define HF_PCIE_IOCTL_SET_WRITE_MEMERY_BAR_PARAM CTL_CODE(FILE_DEVICE_UNKNOWN, 0x004, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 5. 执行写入MEMERYBAR操作
// 执行向MEMERYBAR中写数据的操作
// 该操作必须在IOCTL_SET_WRITE_MEMERY_BAR_PARAM执行成功后执行，否则将返回失败
// InputBuffer:要写入的数据的地址指针
// OutputBuffer:无
#define HF_PCIE_IOCTL_WRITE_MEMERY_BAR CTL_CODE(FILE_DEVICE_UNKNOWN, 0x005, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 6. 执行通过硬件DMA读取DDR DATA操作
// 执行从MEMERYBAR中读取内存地址的操作
// InputBuffer:HF_PCIE_RW_PARAM结构体指针，包括读取的地址和长度
// OutputBuffer:保存数据的地址指针
#define HF_PCIE_IOCTL_READ_DDR_BAR CTL_CODE(FILE_DEVICE_UNKNOWN, 0x006, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

// 7. 设置通过硬件DMA写入DDR DATA的参数
// 注意事项同写入MEMERY BAR
// InputBuffer:一个HF_PCIE_RW_PARAM指针
// OutputBuffer:无
#define HF_PCIE_IOCTL_SET_WRITE_DDR_BAR_PARAM CTL_CODE(FILE_DEVICE_UNKNOWN, 0x007, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 8. 执行通过硬件DMA写入FE DATA的操作
// 通过硬件的DMA执行向FE DATA中写数据的操作
// 该操作必须在IOCTL_SET_WRITE_FE_DATA_PARAM执行成功后执行，否则将返回失败
// InputBuffer:要写入的数据的地址指针
// OutputBuffer:无
#define HF_PCIE_IOCTL_WRITE_DDR_BAR CTL_CODE(FILE_DEVICE_UNKNOWN, 0x008, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 9. 获取被丢弃的线数据包的数量
// InputBuffer:无
// OutputBuffer:HF_PCIE_DISCARD_PACKAGE型的指针
#define HF_PCIE_IOCTL_GET_DISCARD_PACKAGES_INFO \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0009, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

// 10. 清除被丢弃的线数据包的计数
// InputBuffer:无
// OutputBuffer:无
#define HF_PCIE_IOCTL_RESET_DISCARD_PACKAGES_INFO \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x00A, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

// 11. 设置DMA的方向
// InputBuffer:EDmaAccess
// OutputBuffer:无
#define HF_PCIE_IOCTL_SET_DMA_ACCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x00B, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 12. 设置开始/停止扫描
// InputBuffer:ULONG32
// OutputBuffer:无
#define HF_PCIE_IOCTL_SET_BEGIN_TX_STATE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x00C, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 13. 获取线数据包数据包
// InputBuffer:无
// OutputBuffer:缓冲区地址
#define HF_PCIE_IOCTL_GET_DATA_UPLOAD_PACKAGE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x00D, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 14. 获取当前片内存储器的数量
// InputBuffer:无
// OutputBuffer:缓冲区地址
#define HF_PCIE_IOCTL_GET_MEMORY_BARS_NUMBER CTL_CODE(FILE_DEVICE_UNKNOWN, 0x00E, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 15. 获取连续物理缓冲区A数据
// InputBuffer:无
// OutputBuffer:缓冲区地址
#define HF_PCIE_IOCTL_GET_PHY_MEMORY_BUFA_DATA CTL_CODE(FILE_DEVICE_UNKNOWN, 0x00F, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 16. 获取连续物理缓冲区B数据
// InputBuffer:ULONG32
// OutputBuffer:缓冲区地址
#define HF_PCIE_IOCTL_GET_PHY_MEMORY_BUFB_DATA CTL_CODE(FILE_DEVICE_UNKNOWN, 0x010, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 17. 获取连续物理缓冲区A地址
// InputBuffer:无
// OutputBuffer:缓冲区地址
#define HF_PCIE_IOCTL_GET_PHY_MEMORY_BUFA_ADDR CTL_CODE(FILE_DEVICE_UNKNOWN, 0x011, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 18. 获取连续物理缓冲区b地址
// InputBuffer:无
// OutputBuffer:缓冲区地址
#define HF_PCIE_IOCTL_GET_PHY_MEMORY_BUFB_ADDR CTL_CODE(FILE_DEVICE_UNKNOWN, 0x012, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 19. 获取连续物理缓冲区数据
// InputBuffer:无
// OutputBuffer:缓冲区地址
#define HF_PCIE_IOCTL_GET_PHY_MEMORY_BUF_DATA CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0013, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
