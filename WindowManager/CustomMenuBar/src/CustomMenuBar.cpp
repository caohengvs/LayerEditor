#include "CustomMenuBar.hpp"
#include <QDebug>
#include <QHBoxLayout>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>

CustomMenuBar::CustomMenuBar(QWidget* parent)
    : QWidget(parent)
{
    addAction("101", [this](const auto& args) { this->onSaveButtonClicked(); });
    addAction("102", [this](const auto& args) { this->onRemoveButtonClicked(); });
    addAction("103", [this](const auto& args) { this->onDoneButtonClicked(); });
    addAction("104",
              [this](const auto& args)
              {
                  if (args.size() != 1)
                  {
                      return;
                  }
                  try
                  {
                      int val = std::any_cast<int>(args[0]);
                      this->onRotateButtonClicked(val);
                  }
                  catch (const std::bad_any_cast& e)
                  {
                      qWarning() << "Error: Invalid argument types for 'print_info'. " << e.what();
                  }
              });

    addAction("105",
              [this](const auto& args)
              {
                  if (args.size() != 1)
                  {
                      return;
                  }
                  try
                  {
                      int val = std::any_cast<int>(args[0]);
                      this->onRotateButtonClicked(val);
                  }
                  catch (const std::bad_any_cast& e)
                  {
                      qWarning() << "Error: Invalid argument types for 'print_info'. " << e.what();
                  }
              });

    addAction("106",
              [this](const auto& args)
              {
                 this->onShowOriginalImg();
              });

    setupUi();

    QJsonArray btnConfig;
    readConfig(":/config/menuConfig.json", btnConfig);

    for (const auto& btn : btnConfig)
    {
        if (!btn.isObject())
        {
            qWarning() << "CustomMenuBar::CustomMenuBar: Invalid button configuration in JSON.";
            continue;
        }

        QJsonObject btnObj = btn.toObject();
        QString iconPath = btnObj.value("icon").toString();
        QString toolTip = btnObj.value("tip").toString();

        QPushButton* button = new QPushButton;
        QIcon icon(iconPath);
        button->setIcon(icon);
        button->setIconSize(QSize(28, 28));
        button->setFlat(true);
        button->setToolTip(toolTip);
        m_mainLayout->addWidget(button);
        connect(button, &QPushButton::clicked, this,
                [this, btnObj]()
                {
                    const auto& actionId = btnObj.value("actionId").toString();
                    if (!m_actionMap.contains(actionId))
                    {
                        qWarning() << "CustomMenuBar: Action ID not found in action map:" << actionId;
                        return;
                    }

                    if (btnObj.contains("value"))
                    {
                        const auto& val = btnObj.value("value").toInt();
                        m_actionMap[actionId]({val});
                    }
                    else
                    {
                        m_actionMap[actionId]({});
                    }
                });
    }

    m_mainLayout->addStretch();
}

CustomMenuBar::~CustomMenuBar()
{
}

void CustomMenuBar::onSaveButtonClicked()
{
    emit saveClicked();
}

void CustomMenuBar::onRemoveButtonClicked()
{
    emit removeClicked();
}

void CustomMenuBar::onDoneButtonClicked()
{
    emit doneClicked();
}

void CustomMenuBar::onRotateButtonClicked(int val)
{
    emit rotateClicked(val);
}

void CustomMenuBar::onShowOriginalImg()
{
    emit showOriginalImg();
}

void CustomMenuBar::setupUi()
{
    setFixedHeight(50);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(
        "CustomMenuBar {"
        "   background-color: #27272a; /* zinc-800 */"
        // "   border-bottom: 1px solid #3f3f46; /* zinc-700 */"
        // "   border-radius: 6px; /* rounded-md */"
        "   padding: 0 16px; "
        "}"
        "QPushButton {"
        "   background-color: transparent; "
        "   color: #d4d4d8; "
        "   border: none; "
        "   padding: 4px 12px; "
        "   border-radius: 4px; "
        "   font-size: 13px; "
        "   font-weight: 500; "
        "}"
        "QPushButton:hover {"
        "   background-color: #3f3f46; /* zinc-700 */"
        "   color: #ffffff; /* hover:text-white */"
        "}"
        "QPushButton:pressed {"
        "   background-color: #52525b;"
        "}");
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(1);
}

bool CustomMenuBar::readConfig(const QString& config, QJsonArray& outJson)
{
    QFile file(config);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "CustomMenuBar::readConfig: 错误: 无法打开配置文件" << config << file.errorString();
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "CustomMenuBar::readConfig: 错误: JSON解析失败 -" << parseError.errorString();
        qWarning() << "Offset:" << parseError.offset;
        return false;
    }

    if (!doc.isObject())
    {
        qWarning() << "CustomMenuBar::readConfig: 错误: JSON文档不是一个对象.";
        return false;
    }

    QJsonObject jsonObj = doc.object();
    if (!jsonObj.contains("btn") || !jsonObj["btn"].isArray())
    {
        qWarning() << "CustomMenuBar::readConfig: 'btn' key not found in JSON configuration.";
        return false;
    }

    outJson.swap(jsonObj["btn"].toArray());
    return true;
}

void CustomMenuBar::addAction(const QString& actionId,
                              const std::function<void(const std::vector<std::any>& args)>& action)
{
    m_actionMap[actionId] = action;
}
