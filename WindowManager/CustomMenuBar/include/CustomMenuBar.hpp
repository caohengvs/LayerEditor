#pragma once
#include <QWidget>
#include <QHBoxLayout> 
#include <QPushButton> 
#include <QLabel>     
class CustomMenuBar : public QWidget
{
    Q_OBJECT 

public:
    explicit CustomMenuBar(QWidget* parent = nullptr);

    ~CustomMenuBar();

signals:
    void importClicked();
    void playAndPauseClicked();
    void removeClicked();

private slots:
    void onImportButtonClicked();
    void onPlayAndPauseButtonClicked();
    void onRemoveButtonClicked();

private:
    QHBoxLayout* m_mainLayout; 
};
