#ifndef RAWWIDGET_H
#define RAWWIDGET_H

#include <QWidget>

namespace Ui {
class RawWidget;
}

class RawWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RawWidget(QWidget *parent = nullptr);
    ~RawWidget();

private:
    Ui::RawWidget *ui;
};

#endif // RAWWIDGET_H
