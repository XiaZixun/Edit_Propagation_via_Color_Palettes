#pragma once

#include <QWidget>
#include <QString>
#include "data.h"

class ImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageWidget(bool _isAfter, QWidget *parent = nullptr);

    void setData(Data *value);

protected:
    virtual void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event) override;
signals:

public slots:
    void setTime(int t);
    void update();

protected:
    Data *data = nullptr;
    int time = 0;

private:
    bool isAfter;
};
