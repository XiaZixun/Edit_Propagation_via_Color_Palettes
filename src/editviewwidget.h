#pragma once

#include "openglwidget.h"
#include <QMenu>

class EditViewWidget : public OpenGLWidget
{
Q_OBJECT
public:
    explicit EditViewWidget(QWidget *parent = 0);

protected:
    void paintGL() Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) override;

public Q_SLOTS:
	void getColor(QColor c);
	void setTime(int t_);

	void resetEditColor();
	void resetAllEditColors();
	void deleteEditColor();
	
Q_SIGNALS:
	void setColor(QColor c);

private:
    bool blink = false;
	double scale = 1.95;
	int selected_vid = -1;
	double aspect = 1.0;
	int time = 0;

	vector<vec3> palette_pos;
	QMenu *pMenu;
};

