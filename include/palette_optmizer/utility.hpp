#pragma once

#include <vector>
#include <array>
#include <opencv2/opencv.hpp>

using std::vector;
using cv::Vec3d;

template<typename T>
inline int sign(T const &x) { return (T(0)<x)-(x<T(0)); }

// template<typename T>
// inline T clamp(T const &x,T const &lo,T const &hi) {
//     return x<lo?lo:(hi<x?hi:x);
// }

template<class dataType>
inline bool in_section(dataType const &val,dataType const &min,dataType const &max) {
    return min<=val && val<=max;
}

template<class dataType>
inline bool in_section(vector<dataType> const &vals,dataType const &min,dataType const &max) {
    for(auto const &val:vals)
        if(!in_section(val,min,max)) {
            return false;
        }
    return true;
}

template<typename T,int cn>
inline vector<T> flatten(vector<cv::Vec<T,cn>> const &a) {
    vector<double> v(a.size()*cn);
    for(int i=0;i<a.size();++i)
        for(int j=0;j<cn;++j)
            v[i*cn+j]=a[i][j];
    return v;
}

template<typename T,int cn>
inline vector<cv::Vec<T,cn>> fold(vector<T> const &v) {
    vector<cv::Vec<T,cn>> a(v.size()/cn);
    for(int i=0;i<a.size();++i)
        for(int j=0;j<cn;++j)
            a[i][j]=v[i*cn+j];
    return a;
}

inline vector<Vec3d> recover(vector<Vec3d> const &vertices,vector<double> const &weights) {
    uint n_vertices=vertices.size();
    uint n_pixels=weights.size()/n_vertices;
    vector<Vec3d> pixels(n_pixels,Vec3d(0,0,0));
    for(int i=0;i<n_pixels;++i)
        for(int j=0;j<n_vertices;++j)
            pixels[i]+=weights[i*n_vertices+j]*vertices[j];
    return pixels;
}

