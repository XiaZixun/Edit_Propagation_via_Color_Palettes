#include "editviewwidget.h"

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
#include "QtColorWidgets/color_dialog.hpp"
using namespace color_widgets;

// Widget shows a palette-time view

EditViewWidget::EditViewWidget(QWidget *parent) : OpenGLWidget(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=](){ blink = !blink; update(); });
    timer->start(800);

	pMenu = new QMenu(this);
	QAction *pResetTask = new QAction(tr("reset this color"), this);
	QAction *pResetAllTask = new QAction(tr("reset all color"), this);
	QAction *pDeleteTask = new QAction(tr("delete this color"), this);
	pMenu->addAction(pResetTask);
	pMenu->addAction(pResetAllTask);
	pMenu->addAction(pDeleteTask);
	connect(pResetTask, SIGNAL(triggered()), this, SLOT(resetEditColor()));
	connect(pResetAllTask, SIGNAL(triggered()), this, SLOT(resetAllEditColors()));
	connect(pDeleteTask, SIGNAL(triggered()), this, SLOT(deleteEditColor()));
}

void EditViewWidget::setTime(int t) {
	time = t;
	if (selected_vid != -1 ) {
		data->Recolor();
	}
}

void EditViewWidget::getColor(QColor c) {
	int r = qRed(c.rgb());
	int g = qGreen(c.rgb());
	int b = qBlue(c.rgb());

	if (selected_vid != -1) {
		data->setEditColor(selected_vid, c);
		update();
		data->Recolor();
	}
}

void EditViewWidget::paintGL()
{
    OpenGLWidget::paintGL();

    glScalef(scale, scale, scale);

	int w = width(), h = height();
	double aspect = w*1.0 / h;

	//���ư�ɫ���ο�
	/*
	glBegin(GL_LINES);

	for (int p = 0; p <= 1; p++)
	{
		double q = p - .5;
		glColor3f(1., 1., 1.);
		glVertex3f(q, -.5, 0.);
		glColor3f(1., 1., 1.);
		glVertex3f(q, .5, 0.);
		glColor3f(1., 1., 1.);
		glVertex3f(-.5, q, 0.);
		glColor3f(1., 1., 1.);
		glVertex3f(.5, q, 0.);
	}
	glEnd();
	*/

	auto const &ori_edit = data->GetOriginalEdit();
	auto const &cur_edit = data->GetChangedEdit();
	

	float space = 0.02;
	float recw = 0.06;
	float x = -0.5, y = 0.45, y1 = 0.1;
	palette_pos.clear();
	
	for (int i = 0; i < ori_edit.size(); i++)
	{
		cv::Vec3d const &overt = ori_edit[i];
		double or_ = overt[0], og = overt[1], ob = overt[2];
		glColor3f(or_ , og, ob);
		glRectf(x, y, x + recw, y - recw*aspect);
		
		cv::Vec3d const &cvert = cur_edit[i];
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

void EditViewWidget::mousePressEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton) {
		//1. ����ϵ�任������Ļ����ϵ -> opengl����ϵ
		int w = width(), h = height();
		double half_w = w / 2.0, half_h = h / 2.0;
		double new_x = (ev->x() - half_w) / half_w / scale;
		double new_y = -(ev->y() - half_h) / half_h / scale;

		vector<cv::Vec3d> cur_color = data->GetChangedEdit();

		//2. �ҵ������������ĵ�
		double aspect = w*1.0 / h;
		float recw = 0.07;
		float rech = recw * aspect;

		for (int i = 0; i < cur_color.size(); i++)
		{
			double x = palette_pos[i][0];
			double y = palette_pos[i][1];

			double dis1 = fabs(x - new_x);
			double dis2 = fabs(y - new_y);

			if (dis1 < recw / 2 && dis2 < rech / 2) {
				selected_vid = i;

				double r = cur_color[i][0];
				double g = cur_color[i][1];
				double b = cur_color[i][2];

				int R = r * 255, G = g * 255, B = b * 255;
				QColor c;
				c.setRed(R);
				c.setGreen(G);
				c.setBlue(B);
				Q_EMIT setColor(c);
				break;
			}
		}
	}
	else if (ev->button() == Qt::RightButton) {
		pMenu->exec(cursor().pos());
	}
}

void EditViewWidget::resetEditColor() {
	if (selected_vid != -1) {
		data->resetEditColor(selected_vid);
		emit update();
	}
}
void EditViewWidget::resetAllEditColors() {
	if (selected_vid != -1) {
		data->resetAllEditColors();
		emit update();
	}
}

void EditViewWidget::deleteEditColor() {
	if (selected_vid != -1) {
		data->deleteEditColor(selected_vid);
		emit update();
	}
}