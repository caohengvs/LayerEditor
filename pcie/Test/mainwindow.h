#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <vector>

class PCIEWindow;
class IHfFeCommonLayer;
QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    QPixmap randomPixmap(int w = 512, int h = 512);
    void closeEvent(QCloseEvent* event) override;

private:
    inline std::vector<std::byte> read_bytes(const std::string& path, std::size_t offset, std::size_t want)
    {
        // 二进制打开，指针移到末尾
        std::ifstream ifs(path, std::ios::binary | std::ios::ate);
        if (!ifs)
            throw std::runtime_error("cannot open file: " + path);

        // 文件实际大小
        auto file_size = static_cast<std::size_t>(ifs.tellg());
        auto will_read = std::min(want, file_size);

        // 回到开头
        ifs.seekg(static_cast<std::streamoff>(offset), std::ios::beg);

        // 一次性读
        std::vector<std::byte> buffer(will_read);
        ifs.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(will_read));

        // 把实际读到的字节数收缩到 vector 里（可选）
        buffer.resize(static_cast<std::size_t>(ifs.gcount()));
        return buffer;
    }

    inline void write_bytes(const std::string& path, const std::vector<std::byte>& data)
    {
        std::ofstream ofs(path, std::ios::binary);
        if (!ofs)
            throw std::runtime_error("cannot open output file: " + path);
        ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        ofs.flush();  // 立即落盘
    }

private:
    Ui::MainWindow* ui;
    IHfFeCommonLayer& m_fe;
    PCIEWindow* m_winPcie;
    std::thread m_prod;
    std::atomic<bool> m_bRun;
};
#endif  // MAINWINDOW_H
