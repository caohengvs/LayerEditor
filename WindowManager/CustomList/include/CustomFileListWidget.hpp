#pragma once
#include <QDesktopServices>  // For opening files
#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QHeaderView>  // For QTreeView header customization
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeView>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>


class CustomFileListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomFileListWidget(QWidget* parent = nullptr);
    ~CustomFileListWidget();

    void setCurrentPath(const QString& path);

    QString currentPath() const;

signals:
    void fileDoubleClicked(const QString& filePath);
    void currentPathChanged(const QString& newPath);

private slots:
    void on_pathLineEdit_returnPressed();
    void on_treeView_doubleClicked(const QModelIndex& index);

private:
    QTreeView* treeView;
    QFileSystemModel* fileSystemModel;
    QLineEdit* pathLineEdit;
};
