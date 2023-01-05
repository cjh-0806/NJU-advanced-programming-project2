#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "map.h"
#include "mapwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startButton_clicked()
{
    MapWidget* m_w = new MapWidget(nullptr, map);
    m_w->show();
}

void MainWindow::on_exportButton_clicked() //导出默认地图到指定路径
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "选择文件夹", "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!dirPath.isEmpty())
    {
        dirPath += "/default_map.txt";
        map.map2file(dirPath.toStdString().c_str());
        QMessageBox::information(this, "导出默认地图", "默认地图已成功保存至" + dirPath);
    }
}

void MainWindow::on_importButton_clicked() //导入用户自己设计的地图，默认地图无错误
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), "/home", "Text files (*.txt)");
    if(!filePath.isEmpty())
    {
        map.file2map(filePath.toStdString().c_str());
        QMessageBox::information(this, "导入地图", "已成功导入地图" + filePath);
    }
}

void MainWindow::on_exitButton_clicked()
{
    this->close();
}
