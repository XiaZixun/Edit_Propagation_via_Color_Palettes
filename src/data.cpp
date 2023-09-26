#include "data.h"
#include "utility.h"
#include <QFile>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <QProgressDialog>
#include <QThread>
#include <QMessageBox>
#include <QWidget>
#include <QTime>
#include <QTime>
#include <omp.h>
#include <map>
#include "my_util.h"
#include <fstream>

Data::Data()
{

}

void Data::OpenImage(QString fileName)
{
	img_plt.set_src_image(fileName.toStdString());
	recolored_image = img_plt.pixels;
	emit updated();
}

void Data::OpenPalette(string fileName)
{
	img_plt.set_src_palette(fileName);
	recolored_palette = img_plt.cvx.vertices;
	emit updated();
}

void Data::reset()
{
	img_plt.reset();
	recolored_image.clear();
	recolored_palette.clear();
	ori_color.clear();
	cur_color.clear();
}

void Data::addEditColor(QPoint pos) {
	// std::cout<<"addEditColor, pos: "<<pos.x()<<' '<<pos.y()<<std::endl;
	int index=pos.y()*img_plt.width + pos.x();
	Vec3d color_sum(0,0,0);
	double weight_sum=0;
	for(int y=std::max(pos.y()-2,0); y<=std::min(pos.y()+2,img_plt.height-1); y++)
		for(int x=std::max(pos.x()-2,0); x<=std::min(pos.x()+2,img_plt.width-1); x++) {
			int idx=y*img_plt.width + x;
			double dis=sqrt(pow(pos.x()-x,2)+pow(pos.y()-y,2))+cv::norm((img_plt.pixels[idx]-img_plt.pixels[index])/std::max(img_plt.height,img_plt.width));
			double weight=std::exp(-dis);
			color_sum+=img_plt.pixels[idx]*weight;
			weight_sum+=weight;
		}
	ori_color.push_back(color_sum/weight_sum);
	cur_color.push_back(color_sum/weight_sum);
}

void Data::setEditColor(int id, QColor c) {
	double r = qRed(c.rgb()) / 255.0;
	double g = qGreen(c.rgb()) / 255.0;
	double b = qBlue(c.rgb()) / 255.0;
	cur_color.at(id)=cv::Vec3d(r,g,b);
}

void Data::ComputeMVCWeights() {
	if (img_plt.weight_calculated)
		return;
	QTime time1;
	time1.start();
	img_plt.calc_MVCweights();
	qDebug() << "MVC time: " << time1.elapsed() / 1000.0 << "s";
	QMessageBox::information(NULL, "Info", "MVC finished", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
}

void Data::Recolor() {
	if (!img_plt.weight_calculated)
		return;

	QTime time1;
	time1.start();

	auto const &result=img_plt.recolor_by(ori_color, cur_color);
    recolored_image=result.first;
    recolored_palette=result.second;

	qDebug() << "Recolor time: " << time1.elapsed() / 1000.0 << "s";
	emit updated();
}

void Data::ExportRecoloredImage(string path) {

	QImage image(img_plt.width, img_plt.height, QImage::Format_RGB888);
    for(int i=0; i<img_plt.height; i++) {
        uchar *const row = image.scanLine(i);
        for(int j=0; j<img_plt.width; j++) {
            cv::Vec3b pixel(recolored_image[i*img_plt.width+j]*255);
            row[j*3] = pixel[0];
            row[j*3+1] = pixel[1];
            row[j*3+2] = pixel[2];
        }
    }
	if(path.substr(path.size()-4) != ".png")
		path += ".png";
    image.save(path.c_str(), "PNG", 100);
}

void Data::resetEditColor(int id) {
	cur_color.at(id) = ori_color.at(id);
	Recolor();
	emit updated();
}

void Data::resetAllEditColors() {
	for (int i = 0; i < ori_color.size(); i++)
		cur_color[i] = ori_color[i];
}

void Data::deleteEditColor(int id) {
	if(id < 0 || id >= ori_color.size())
		return;
	ori_color.erase(ori_color.begin() + id);
	cur_color.erase(cur_color.begin() + id);
}

// void Data::ImportChangedPalette(string path) {
// 	ifstream ifs(path);
// 	int n; ifs >> n;
// 	for (int i = 0; i < n; i++) {
// 		ifs >> changed_palette.vertices[i][0] >> changed_palette.vertices[i][1]
// 			>> changed_palette.vertices[i][2];
// 	}
// 	emit updated();
// }


void Data::ExportChangedPalette(string path) {
	std::ofstream ofs(path);
	ofs << recolored_palette.size() << std::endl;
	for (int i = 0; i < recolored_palette.size(); i++) {
		ofs << recolored_palette[i][0] << " " <<
			recolored_palette[i][1] << " " <<
			recolored_palette[i][2] << " " << std::endl;
	}
}

void Data::ImportEdit(string path) {
	std::ifstream ifs(path);
	int n; ifs >> n;
	ori_color.resize(n);
	cur_color.resize(n);
	for(int i = 0; i < n; i++) {
		ifs >> ori_color[i][0] >> ori_color[i][1] >> ori_color[i][2];
		ifs >> cur_color[i][0] >> cur_color[i][1] >> cur_color[i][2];
	}
}

void Data::ExportEdit(string path) {
	std::ofstream ofs(path);
	ofs << ori_color.size() << std::endl;
	for (int i = 0; i < ori_color.size(); i++) {
		ofs << ori_color[i][0] << " " <<
			ori_color[i][1] << " " <<
			ori_color[i][2] << " " << 
			cur_color[i][0] << " " <<
			cur_color[i][1] << " " <<
			cur_color[i][2] << std::endl;
	}
}