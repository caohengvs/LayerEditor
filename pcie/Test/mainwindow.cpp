#include "mainwindow.h"
#include <QApplication>
#include <QCloseEvent>
#include <QRandomGenerator>
#include <algorithm>
#include <filesystem>
#include "IHfFeCommonLayer.h"
#include "PCIEWindow.hpp"
#include "HfFeDataParser/CHfFeDataParserPCIE_V15.h"




// #pragma pack(push, 1)
// struct  _ST_LINE_HEADER
// {
//     uint64_t rsvd7_0 : 64;  // [63:0]
//     uint64_t rsvd7_1 : 64;  // [127:64]
//     uint64_t rsvd7_2 : 64;  // [191:128]
//     uint64_t rsvd7_3 : 64;  // [255:192]
//     uint64_t rsvd7_4 : 64;  // [319:256]
//     uint64_t rsvd7_5 : 63;  // [382:320]
//     uint64_t sof : 1;       // [383:383]

//     uint16_t linlen : 16;            // [399:384]
//     uint8_t pw_gap : 1;              // [400:400]
//     uint8_t dummy_line : 1;          // [401:401]
//     uint8_t freq_comp_param : 2;     // [403:402]
//     uint8_t img_mode : 4;            // [407:404]
//     uint8_t rsvd6 : 3;               // [410:408]
//     uint8_t depthid : 5;             // [415:411]
//     uint8_t rsvd5 : 2;               // [417:416]
//     uint8_t eof : 1;                 // [418:418]
//     uint8_t cfm_ensembler : 5;       // [423:419]
//     uint8_t rsvd4 : 8;               // [431:424]
//     uint8_t beamno : 8;              // [439:432]
//     uint8_t rsvd3 : 1;               // [440:440]
//     uint8_t tfocus : 5;              // [445:441]
//     uint8_t bi_plane_mode : 2;       // [447:446]
//     uint8_t rsvd2 : 1;               // [448:448]
//     uint8_t ext_img_mode : 1;        // [449:449]
//     uint8_t spacial_comp : 1;        // [450:450]
//     uint8_t color_line_density : 2;  // [452:451]
//     uint8_t freq_comp : 1;           // [453:453]
//     uint8_t b_line_density : 2;      // [455:454]
//     uint8_t scanid : 8;              // [463:456]
//     uint8_t rsvd1 : 7;               // [470:464]
//     uint16_t angle_index : 9;        // [479:471]
//     uint32_t line_tag : 32;          // [511:480]
// };
// #pragma pack(pop)
// typedef _ST_LINE_HEADER STLineHeader;


// static_assert(sizeof(STLineHeader) == 64, "size mismatch, check padding!");

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_fe(IHfFeCommonLayer::s_GetInstance())
    , m_winPcie(PCIEWindow::s_GetInstance())
    , m_bRun(false)
{
    resize(500, 500);
    QPushButton* btnRead = new QPushButton(this);
    btnRead->setGeometry(50, 50, 50, 50);
    btnRead->setText("read");
    // btnRead->show();

    QPushButton* btnWrite = new QPushButton(this);
    btnWrite->setGeometry(50, 120, 50, 50);
    btnWrite->setText("write");
   
    connect(btnRead, &QPushButton::clicked, this,
            [this]()
            {
                 CHfFeDataParserPCIE_V15 parser;
                auto buf = read_bytes("D:/repos/TestQt/DMABuffer/DMABuffer1MB.bin", 0x00, 64 + 1024 * 1024);
                parser.SetRawData(buf.data(), 64+1024*1024);
                const auto lines = parser.GetLineData();
                // STLineHeader* lineHeader = (STLineHeader*)buf.data();
                // if (lineHeader->line_tag == 0x80008000)
                // {
                //     std::cout << "hello" << "\n";
                // }
                // std::cout << "scanid = 0x" << std::hex << std::setw(2) << std::setfill('0')
                //           << static_cast<unsigned>(lineHeader->scanid) << '\n';

                // std::cout << "b_line_density = 0x" << std::hex << std::setw(2) << std::setfill('0')
                //           << static_cast<unsigned>(lineHeader->b_line_density) << '\n';

                // std::cout << "color_line_density = 0x" << std::hex << std::setw(2) << std::setfill('0')
                //           << static_cast<unsigned>(lineHeader->color_line_density) << '\n';

                // std::cout << "angle= 0x" << std::hex << std::setw(4) << std::setfill('0') << lineHeader->getAngle()
                //           << '\n';

                // if(lineHeader->linlen == 1024)
                // {
                //     std::cout << "good";
                // }
                // std::cout << "linelen=" << std::dec << lineHeader->linlen
                //           << '\n';
            //  std::cout << "angle = " << std::dec << lineHeaderangle << '\n'; 
            });

    connect(btnWrite, &QPushButton::clicked, this,
            [this]()
            {
                auto buf = read_bytes("D:/repos/TestQt/DMABuffer/DMABuffer.bin", 0x00095980, 64 + 1024 * 1024);
                write_bytes("D:/repos/TestQt/DMABuffer/DMABuffer1MB.bin", buf);
            });

    m_winPcie->show();
    m_bRun = true;
    // m_prod = std::thread(
    //     [this]()
    //     {
    //         while (m_bRun.load())
    //         {
    //             m_winPcie->AddImage(randomPixmap(512, 512), 0);
    //             std::this_thread::sleep_for(std::chrono::milliseconds(1));
    //         }
    //     });

    m_winPcie->StartCollect();
}

MainWindow::~MainWindow()
{
    m_bRun = false;
    if (m_prod.joinable())
        m_prod.join();

    PCIEWindow::s_DeleteInstance();
}
void MainWindow::closeEvent(QCloseEvent* event)
{
    m_bRun = false;
    if (m_prod.joinable())
        m_prod.join();
    PCIEWindow::s_DeleteInstance();

    // 接受关闭事件
    event->accept();
    QApplication::quit();
}

QPixmap MainWindow::randomPixmap(int w, int h)
{
    QImage img(w, h, QImage::Format_RGB32);
    for (int y = 0; y < h; ++y)
    {
        QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < w; ++x)
        {
            line[x] = qRgb(QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256),
                           QRandomGenerator::global()->bounded(256));
        }
    }
    return QPixmap::fromImage(img);
}
