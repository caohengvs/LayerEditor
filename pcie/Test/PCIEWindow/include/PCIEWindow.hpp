#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <array>
#include <future>
#include <mutex>
#include <queue>
#include "ExportDef.h"

class CustomTitleBar;
class CustomView;
class CustomScene;

class LIB_API PCIEWindow : public QMainWindow
{
    Q_OBJECT

public:
    static PCIEWindow* s_GetInstance();
    static void s_DeleteInstance();

private:
    explicit PCIEWindow(QWidget* parent = nullptr);
    ~PCIEWindow();

public:
    bool AddImage(const QPixmap& image, int nIndex);
    bool StartCollect();
    void StopCollect();
    void PauseCollect();
    void ResumeCollect();
    void Show();
    void Hide();
    bool IsShow();

signals:
    void update(const QPixmap&);
    void dontDisplay();

private:
    void init();

private:
    QPushButton* m_btnUpdate;
    CustomTitleBar* m_customTitleBar;
    std::pair<CustomScene*, CustomView*> m_mainSceneAndView;
    static PCIEWindow* m_pInstance;


    std::mutex m_mtxImg;  // 图像队列
    std::queue<QPixmap> m_qImage;
    std::future<void> m_futUpdate;
    std::atomic<bool> m_bRun;
    std::condition_variable m_cvRun;  // 运行条件
    std::atomic<bool> m_bPaused;
};