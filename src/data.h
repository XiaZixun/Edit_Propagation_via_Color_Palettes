#pragma once

#include<QString>
#include<QObject>
#include<vector>
#include <QThread>
#include"utility.h"
#include "vec3.h"
#include "image.hpp"
#include <string>
#include <opencv2/opencv.hpp>

class Data : public QObject
{
    Q_OBJECT

public:
    Data();
    void OpenImage(QString fileName);
    void OpenPalette(string fileName);
    void reset();

	vector<Vec3d> GetImage(bool isAfter = true) const { return isAfter ? recolored_image : img_plt.pixels; }
	vector<Vec3d> GetChangedPalette() const { return recolored_palette; }
	vector<Vec3d> GetOriginalPalette() const { return img_plt.cvx.vertices; }
	vector<Vec3d> GetPalette() const { return img_plt.weight_calculated ? recolored_palette : img_plt.cvx.vertices; }
	vector<std::array<uint,3>> GetFaces() const { return img_plt.cvx.triangles; }
	vector<cv::Vec3d> GetOriginalEdit()	const { return ori_color; }
	vector<cv::Vec3d> GetChangedEdit()	const { return cur_color; }

	void ComputeMVCWeights();

	void Recolor();
	
    int GetFrameWidth() const { return img_plt.width; }
    int GetFrameHeight() const{ return img_plt.height; }
    int GetFrameDepth() const{ return cv::Vec3d::channels; }

	void addEditColor(QPoint pos);
	void setEditColor(int id, QColor c);
	void resetEditColor(int id);
	void resetAllEditColors();
	void deleteEditColor(int id);

	void ExportRecoloredImage(string path);
	void ExportChangedPalette(string path);
	void ImportEdit(string path);
	void ExportEdit(string path);
public slots:
signals:
    void updated();

private:
	imgPlt img_plt;
	vector<Vec3d> recolored_image;
	vector<Vec3d> recolored_palette;

	vector<cv::Vec3d> ori_color;
	vector<cv::Vec3d> cur_color;
};
