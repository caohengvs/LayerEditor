// CustomFileListWidget.h
#pragma once
#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView> // For QTreeView header customization
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QDesktopServices> // For opening files
#include <QUrl>

class CustomFileListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomFileListWidget(QWidget *parent = nullptr);
    ~CustomFileListWidget();

    void setCurrentPath(const QString &path);

    QString currentPath() const;

signals:
    void fileDoubleClicked(const QString &filePath);
    void currentPathChanged(const QString &newPath);

private slots:
    void on_pathLineEdit_returnPressed();
    void on_treeView_doubleClicked(const QModelIndex &index);

private:
    QTreeView *treeView;
    QFileSystemModel *fileSystemModel;
    QLineEdit *pathLineEdit;
};
