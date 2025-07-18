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
    void doneClicked();

private slots:
    void onImportButtonClicked();
    void onPlayAndPauseButtonClicked();
    void onRemoveButtonClicked();
    void onDoneButtonClicked();

private:
    QHBoxLayout* m_mainLayout; 
};
