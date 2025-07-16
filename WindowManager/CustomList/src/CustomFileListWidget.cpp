// CustomFileListWidget.cpp
#include "CustomFileListWidget.hpp"

CustomFileListWidget::CustomFileListWidget(QWidget* parent)
    : QWidget(parent)
{
    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.bmp"
            << "*.mp4" << "*.avi" << "*.mkv" << "*.mov" << "*.wmv" << "*.png";
    fileSystemModel->setNameFilters(filters);
    fileSystemModel->setNameFilterDisables(false);
    fileSystemModel->setRootPath("");

    treeView = new QTreeView(this);
    treeView->setModel(fileSystemModel);
    treeView->hideColumn(1);
    treeView->hideColumn(2);
    treeView->hideColumn(3);
    treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    pathLineEdit = new QLineEdit(this);

    connect(pathLineEdit, &QLineEdit::returnPressed, this, &CustomFileListWidget::on_pathLineEdit_returnPressed);
    connect(treeView, &QTreeView::doubleClicked, this, &CustomFileListWidget::on_treeView_doubleClicked);

    setCurrentPath(QDir::currentPath());

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(pathLineEdit);
    mainLayout->addWidget(treeView);

    setLayout(mainLayout);
}

CustomFileListWidget::~CustomFileListWidget()
{
}

void CustomFileListWidget::setCurrentPath(const QString& path)
{
    QDir dir(path);
    if (!dir.exists())
    {
        return;
    }

    treeView->setRootIndex(fileSystemModel->index(path));
    pathLineEdit->setText(QDir::toNativeSeparators(path));
    emit currentPathChanged(path);
}

QString CustomFileListWidget::currentPath() const
{
    QModelIndex rootIndex = treeView->rootIndex();
    if (rootIndex.isValid())
    {
        return fileSystemModel->fileInfo(rootIndex).absoluteFilePath();
    }
    return QString();
}

void CustomFileListWidget::on_pathLineEdit_returnPressed()
{
    setCurrentPath(pathLineEdit->text());
}

void CustomFileListWidget::on_treeView_doubleClicked(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }

    QFileInfo fileInfo = fileSystemModel->fileInfo(index);

    if (fileInfo.isDir())
    {
        setCurrentPath(fileInfo.absoluteFilePath());
    }
    else if (fileInfo.isFile())
    {
        emit fileDoubleClicked(fileInfo.absoluteFilePath());
    }
}