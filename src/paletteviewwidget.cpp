#include "paletteviewwidget.h"

#include<QTimer>
#include<QDebug>
#include<cmath>
#include<QMouseEvent>
#include<QCryptographicHash>
#include <QColorDialog>
#include <QMenu>
#include "qrgb.h"
#include "math.h"
#include <algorithm>

// Widget shows a palette-time view

PaletteViewWidget::PaletteViewWidget(QWidget *parent) : OpenGLWidget(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=](){ blink = !blink; update(); });
    timer->start(800);
}

void PaletteViewWidget::paintGL()
{
    OpenGLWidget::paintGL();

    glScalef(scale, scale, scale);

    int w = width(), h = height();
    double aspect = w*1.0 / h;

    vector<Vec3d> const &ori_palette = data->GetOriginalPalette();
    vector<Vec3d> const &cur_palette = data->GetChangedPalette();

    float space = 0.02;
    float recw = 0.06;
    float x = -0.5, y = 0.35, y1 = 0.00;
    palette_pos.clear();

    for (int i = 0; i < ori_palette.size(); i++)
    {
        Vec3d overt = ori_palette[i];
        float ored = overt[0];
        float ogreen = overt[1];
        float oblue = overt[2];
        glColor3f(ored, ogreen, oblue);
        glRectf(x, y, x + recw, y - recw*aspect);

        Vec3d cvert = cur_palette[i];
        float cr = cvert[0], cg = cvert[1], cb = cvert[2];
        if (selected_vid == i) {
            double ex = 0.006;
            glColor3f(0.8, 0, 0);
            //glRectf(x- ex, y1+ ex*aspect, x + recw+ ex, y1 - recw*aspect- ex*aspect);
            glRectf(x, y1-recw*aspect - 0.01*aspect, x + recw, y1 - recw*aspect - 0.02*aspect);

        }

        glColor3f(cr , cg, cb);
        glRectf(x, y1, x + recw, y1 - recw*aspect);

        palette_pos.push_back(vec3(x + recw / 2, y1 - recw*aspect / 2, 0));

        x += recw + space;

    }
}
