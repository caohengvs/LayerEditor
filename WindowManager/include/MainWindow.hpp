#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <array>
#include "CommonDef/ExportDef.h"

class CustomTitleBar;
class CustomView;
class CustomScene;

class LIB_API MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void init();

private:
    QPushButton* m_btnUpdate;
    CustomTitleBar* m_customTitleBar;
    std::pair<CustomScene*, CustomView*> m_mainSceneAndView;
    std::pair<CustomScene*, std::array<CustomView*, 8>> m_previewSceneAndView;
};