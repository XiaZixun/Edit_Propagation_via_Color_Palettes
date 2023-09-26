#pragma once

#include "openglwidget.h"

class PaletteViewWidget : public OpenGLWidget
{
Q_OBJECT
public:
    explicit PaletteViewWidget(QWidget *parent = 0);

protected:
    void paintGL() Q_DECL_OVERRIDE;

private:
    bool blink = false;
	double scale = 1.95;
	int selected_vid = -1;
	double aspect = 1.0;
	int time = 0;

	vector<vec3> palette_pos;
};