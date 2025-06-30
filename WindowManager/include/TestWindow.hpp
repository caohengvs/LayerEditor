#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget> 
#include "CommonDef/ExportDef.h"

class LIB_API MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCreateNewWindowClicked(); 

private:
    QPushButton *createNewWindowButton;
};