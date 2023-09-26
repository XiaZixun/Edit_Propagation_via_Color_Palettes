#pragma once

#include <vector>
#include <string>
#include <utility>
#include <random>
#include <ctime>
#include <opencv2/opencv.hpp>

#include "optim_by_gurobi.hpp"
#include "geometry.hpp"

using std::string;

class imgPlt {
public:
    int height=0,width=0;
    static int const RGB_MAX=255;
    vector<Vec3d> pixels, sample;
    cvxHull cvx;
    vector<double> weights, sample_weights;
    bool weight_calculated=false;

public:
    bool set_src_image(string const & im_path) { return set_src_image(cv::imread(im_path.c_str())); }
    bool set_src_image(cv::Mat _im, int sample_num=256) {
        if(_im.empty()||_im.type()!=CV_8UC3)
            return false;
        cv::cvtColor(_im, _im, cv::COLOR_BGR2RGB);
        height=_im.rows,width=_im.cols;
        pixels.resize(height*width);
        for(int x=0; x<height; x++) {
            cv::Vec3b const *p=_im.ptr<cv::Vec3b>(x);
            for(int y=0;y<width;y++)
                pixels[x*width+y]=Vec3d(p[y])/RGB_MAX;
        }
        sample.resize(sample_num);
        std::mt19937 gen(std::time(nullptr));
        std::sample(pixels.begin(), pixels.end(), sample.begin(), sample_num, gen);
        weight_calculated=false;
        return true;
    }
    bool set_src_palette(string const &cvx_path) { return cvx.from_obj_file(cvx_path)==0; }
    bool set_src_palette(cvxHull const &_cvx) { cvx=_cvx; return true; }
    void reset() {
        height=0, width=0;
        pixels.clear(), sample.clear();
        cvx=cvxHull();
        weights.clear();
        sample.clear(), sample_weights.clear();
        weight_calculated=false;
    }

    bool calc_MVCweights() {
        if(pixels.empty()||cvx.vertices.size()<4)
            return false;
        if(weight_calculated)
            return true;
        weights=cvx.calc_MVCweight_inside_cvx_hull(pixels);
        sample_weights=cvx.calc_MVCweight_inside_cvx_hull(sample);
        weight_calculated=true;
        return true;
    }
    std::pair<vector<Vec3d>, vector<Vec3d>> recolor_by(vector<cv::Vec3d> src_pixels, vector<cv::Vec3d> const& dst_pixels) {
        if(!calc_MVCweights())
            return {};
        vector<double> src_weights=cvx.calc_MVCweight_inside_cvx_hull(src_pixels);
        std::fill(src_pixels.begin(), src_pixels.end(), Vec3d(0,0,0));
        for(int i=0; i<src_pixels.size(); i++)
            for(int j=0; j<cvx.vertices.size(); j++)
                src_pixels[i]+=cvx.vertices[j]*src_weights[i*cvx.vertices.size()+j];
        vector<double> phi(sample.size());
        for(int i=0; i<sample.size(); i++) {
            double min_dis=sqrt(3);
            for(int j=0; j<src_pixels.size(); j++)
                min_dis=std::min(min_dis,cv::norm(sample[i]-src_pixels[j]));
            phi[i]=std::exp(min_dis*min_dis);
        }
        vector<Vec3d> recolor_vertices=optimize(cvx.vertices, dst_pixels, src_weights, sample, sample_weights, phi);
        vector<Vec3d> recolor_img(height*width, Vec3d(0,0,0));
#pragma omp parallel for
        for(int i=0; i<height*width; i++)
            for(int j=0; j<cvx.vertices.size(); j++)
                recolor_img[i]+=recolor_vertices[j]*weights[i*cvx.vertices.size()+j];
            // if(!in_section({recolor_img[i][0],recolor_img[i][1],recolor_img[i][2]},0.0,1.0)) {
            //     std::cout<<"Rebuild Error : "<<i<<' '<<std::endl;
            //     std::cout<<"source data "<<cv::Vec3b(pixels[i]*RGB_MAX)<<std::endl;
            //     std::cout<<"recolored data "<<recolor_img[i]<<std::endl;
            // }
        return {recolor_img,recolor_vertices};
    }
    // std::pair<vector<Vec3d>,vector<Vec3d>> recolor_by(vector<cv::Vec3b> const &_src_pixels,vector<cv::Vec3b> const &_dst_pixels) {
    //     assert(_src_pixels.size()==_dst_pixels.size());
    //     vector<Vec3d> src_pixels(_src_pixels.size()),dst_pixels(_src_pixels.size());
    //     for(int i=0;i<src_pixels.size();++i) {
    //         src_pixels[i]=Vec3d(_src_pixels[i])/RGB_MAX;
    //         dst_pixels[i]=Vec3d(_dst_pixels[i])/RGB_MAX;
    //     }
    //     return recolor_by(src_pixels,dst_pixels);
    // }
    // cv::Mat recolor_Mat(vector<cv::Vec3b> const &src_pixels,vector<cv::Vec3b> const &dst_pixels) {
    //     vector<Vec3d> recolor_data=recolor_by(src_pixels,dst_pixels).first;
    //     cv::Mat recolor_im(height,width,CV_8UC3);
    //     for(int x=0;x<height;++x) {
    //         cv::Vec3b *p=recolor_im.ptr<cv::Vec3b>(x);
    //         for(int y=0;y<width;y++) {
    //             p[y]=cv::Vec3b(recolor_data[x*width+y]*RGB_MAX);
    //         }
    //     }
    //     return recolor_im;
    // }
};