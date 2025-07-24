#pragma once
#include <QWidget>
#include <QJsonArray>
#include <QSignalMapper>
#include <QMap>
#include <functional>

class QHBoxLayout;

class CustomMenuBar : public QWidget
{
    Q_OBJECT

public:
    explicit CustomMenuBar(QWidget* parent = nullptr);

    ~CustomMenuBar();

signals:
    void saveClicked();
    void removeClicked();
    void doneClicked();

private slots:
    void onSaveButtonClicked();
    void onRemoveButtonClicked();
    void onDoneButtonClicked();
    
private:
    void setupUi();
    bool readConfig(const QString&,  QJsonArray&);
    void addAction(const QString& actionId, const std::function<void()>& action);

private:
    QHBoxLayout* m_mainLayout;
    QMap<QString,std::function<void()>> m_actionMap;
};
