#include "rawwidget.h"
#include "ui_rawwidget.h"

RawWidget::RawWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RawWidget)
{
    ui->setupUi(this);
}

RawWidget::~RawWidget()
{
    delete ui;
    qDebug() << "dtoc RawWidget";
}
