#pragma once
#include <QJsonArray>
#include <QMap>
#include <QSignalMapper>
#include <QWidget>
#include <any>
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
    void rotateClicked(const int dir);
    void showOriginalImg(const bool visible);
    void cutClicked();

private:
    void onSaveButtonClicked();
    void onRemoveButtonClicked();
    void onDoneButtonClicked();
    void onRotateButtonClicked(const int dir);
    void onShowOriginalImg(const bool visible);
    void onCutButtonClicked();
    void initActions();

private:
    void setupUi();
    bool readConfig(const QString& config, QJsonArray& outJson);
    void addAction(const QString& actionId, const std::function<void(const std::vector<std::any>& args)>& action);

private:
    QHBoxLayout* m_mainLayout;
    QMap<QString, std::function<void(const std::vector<std::any>&)>> m_actionMap;
};
