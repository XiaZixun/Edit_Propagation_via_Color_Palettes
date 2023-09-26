#pragma once

#include <QMainWindow>
#include <data.h>
#include <QSlider>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
	QDockWidget *imageBeforeDockWidget = nullptr;
	QDockWidget *imageAfterDockWidget = nullptr;
    QDockWidget *paletteViewDockWidget = nullptr;
    
	void exportImage();
	// void importPalette();
	void exportPalette();
    void importEdit();
    void exportEdit();
    void importPalette();
    void openFile(bool merge);
    Data *data = nullptr;
    QSlider *slider = nullptr;
    QSlider *mergeStepSlider = nullptr;

   // QString title = "Video Recoloring Tool (Build " __DATE__ " " __TIME__ ")";
	QString title = "Edit Propagation via Geometry Palette";

	bool isPlaying = false;
};
