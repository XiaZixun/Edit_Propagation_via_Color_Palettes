#pragma once

#include <vector>
#include <array>
#include <numeric>
#include <fstream>
#include <algorithm>
#include <opencv2/opencv.hpp>

#include "utility.hpp"

using std::array;

inline double det(Vec3d const &a,Vec3d const &b,Vec3d const &c) {
    return a.dot(b.cross(c));
}

// inline Vec3d closest_point_on_line(array<Vec3d,2> const &l,Vec3d const &p) {
//     Vec3d const &s=l[0],&t=l[1];
//     Vec3d const e=t-s,delta=p-s;
//     // double const k=dot(delta,e)/dot(e,e);
//     double const k=delta.dot(e)/e.dot(e);
//     return s+e*clamp(k,0.0,1.0);
// }

// inline Vec3d closest_point_on_triangle(array<Vec3d,3> const &tri,Vec3d const &p) {
//     Vec3d const e0=tri[1]-tri[0],e1=tri[2]-tri[0],dif=tri[0]-p;
//     // double const a=e0.normsqr(),b=dot(e0,e1),c=e1.normsqr();
//     double const a=e0.dot(e0),b=e0.dot(e1),c=e1.dot(e1);
//     // double const d=dot(e0,dif),e=dot(e1,dif),f=normsqr(dif);
//     double const d=e0.dot(dif),e=e1.dot(dif),f=dif.dot(dif);
//     double const det=a*c-b*b;
//     double const s=(b*e-c*d)/det,t=(b*d-a*e)/det;
//     if(in_section(s,0.0,1.0)&&in_section(t,0.0,1.0))
//         return tri[0]+e0*s+e1*t;
//     Vec3d p0=closest_point_on_line({tri[0],tri[1]},p);
//     Vec3d p1=closest_point_on_line({tri[1],tri[2]},p);
//     Vec3d p2=closest_point_on_line({tri[2],tri[0]},p);
//     double const d0=cv::norm(p-p0),d1=cv::norm(p-p1),d2=cv::norm(p-p2);
//     if(d0<d1)
//         return d0<d2?p0:p2;
//     else
//         return d1<d2?p1:p2;
// }

inline Vec3d closest_point_on_triangle(array<Vec3d,3> const &triangle,Vec3d const &p)
{
    Vec3d edge0 = triangle[1] - triangle[0];
    Vec3d edge1 = triangle[2] - triangle[0];
    Vec3d v0 = triangle[0] - p;

    // double a = dot(edge0, edge0);
    // double b = dot(edge0, edge1);
    // double c = dot(edge1, edge1);
    // double d = dot(edge0, v0);
    // double e = dot(edge1, v0);
    double a = edge0.dot(edge0);
    double b = edge0.dot(edge1);
    double c = edge1.dot(edge1);
    double d = edge0.dot(v0);
    double e = edge1.dot(v0);
    double det = a*c - b*b;
    double s = b*e - c*d;
    double t = b*d - a*e;

    if (s + t < det)
    {
        if (s < 0.0)
        {
            if (t < 0.0)
            {
                if (d < 0.0)
                {
                    s = std::clamp(-d / a, 0.0, 1.0);
                    t = 0.0;
                }
                else
                {
                    s = 0.0;
                    t = std::clamp(-e / c, 0.0, 1.0);
                }
            }
            else
            {
                s = 0.0;
                t = std::clamp(-e / c, 0.0, 1.0);
            }
        }
        else if (t < 0.0)
        {
            s = std::clamp(-d / a, 0.0, 1.0);
            t = 0.0;
        }
        else
        {
            double invDet = 1.0 / det;
            s *= invDet;
            t *= invDet;
        }
    }
    else
    {
        if (s < 0.0)
        {
            double tmp0 = b + d;
            double tmp1 = c + e;
            if (tmp1 > tmp0)
            {
                double numer = tmp1 - tmp0;
                double denom = a - 2 * b + c;
                s = std::clamp(numer / denom, 0.0, 1.0);
                t = 1.0 - s;
            }
            else
            {
                t = std::clamp(-e / c, 0.0, 1.0);
                s = 0.0;
            }
        }
        else if (t < 0.0)
        {
            if (a + d > b + e)
            {
                double numer = c + e - b - d;
                double denom = a - 2 * b + c;
                s = std::clamp(numer / denom, 0.0, 1.0);
                t = 1.0 - s;
            }
            else
            {
                s = std::clamp(-e / c, 0.0, 1.0);
                t = 0.0;
            }
        }
        else
        {
            double numer = c + e - b - d;
            double denom = a - 2 * b + c;
            s = std::clamp(numer / denom, 0.0, 1.0);
            t = 1.0 - s;
        }
    }
    // if (s<0.0 || 1.0 < s || t<0.0 || 1.0<t)
    //     std::cout<<"Insert Error: "<<s<<","<<t<<std::endl;

    return triangle[0] + s * edge0 + t * edge1;
}

class cvxHull {
public:
    vector<Vec3d> vertices;
    vector<array<uint,3>> triangles;
    
    Vec3d closest_point_on_surface(Vec3d const &p) {
        Vec3d nearest=closest_point_on_triangle({vertices[triangles[0][0]],vertices[triangles[0][1]],vertices[triangles[0][2]]},p);
        for(int i=1;i<triangles.size();++i) {
            Vec3d neaer=closest_point_on_triangle({vertices[triangles[i][0]],vertices[triangles[i][1]],vertices[triangles[i][2]]},p);
            if(cv::norm(p-nearest)>cv::norm(p-neaer))
                nearest=neaer;
        }
        return nearest;
    }

    int from_obj_file(std::string const &path) {
        std::fstream infile(path.c_str(),std::ios::in);
        if(!infile)
            return -1;
        vertices.clear();
        triangles.clear();
        for(std::string s;getline(infile,s);) {
            std::stringstream line(s);
            std::string flag;
            line>>flag;
            if(flag[0]=='v') {
                double x,y,z;
                line>>x>>y>>z;
                vertices.push_back(Vec3d(x,y,z));
            }
            else if(flag[0]=='f') {
                uint a,b,c;
                line>>a>>b>>c;
                triangles.push_back({a-1,b-1,c-1});
            }
            else
                return 1;
        }
        infile.close();
        return 0;
    }

    // vector<double> calc_MVCweight(Vec3d const &p,double const eps=1e-6) const {
    //     size_t n=vertices.size();
        
    //     vector<double> w(n,0.0),d(n,0.0);
    //     vector<Vec3d> u(n);
    //     for(size_t i=0;i<n;++i) {
    //         d[i]=cv::norm(vertices[i]-p);
    //         if(d[i]<eps) {
    //             w[i]=1;
    //             return w;
    //         }
    //         u[i]=(vertices[i]-p)/d[i];
    //     }
    //     for(auto const &f:triangles) {
    //         double l[3], theta[3], sin_theta[3];
    //         for(size_t i=0;i<3;++i) {
    //             l[i]=cv::norm(u[f[(i+1)%3]]-u[f[(i+2)%3]]);
    //             theta[i]=2*std::asin(l[i]/2);
    //             sin_theta[i]=sin(theta[i]);
    //         }
    //         double const h=(theta[0]+theta[1]+theta[2])/2;
    //         double const sin_h=sin(h);
    //         if(M_PI-h<eps) {
    //             double tot_w=0.0;
    //             std::fill(w.begin(),w.end(),0.0);
    //             for(size_t i=0;i<3;++i) {
    //                 w[f[i]]=sin_theta[i]*l[(i+2)%3]*l[(i+1)%3];
    //                 tot_w+=w[f[i]];
    //             }
    //             for(size_t i=0;i<3;++i)
    //                 w[f[i]]/=tot_w;
    //             return w;
    //         }
    //          double c[3],s[3];
    //         int const sign_det=sign(det(u[f[0]],u[f[1]],u[f[2]]));
    //         for(size_t i=0;i<3;++i) {
    //             c[i]=2*sin_h*sin(h-theta[i])/(sin_theta[(i+1)%3]*sin_theta[(i+2)%3])-1;
    //             s[i]=sign_det*std::sqrt(1-c[i]*c[i]);
    //         }
    //         if(std::min({std::abs(s[0]),std::abs(s[1]),std::abs(s[2])})>eps)
    //             for(size_t i=0;i<3;++i) {
    //                 double const numerator=theta[i]-c[(i+1)%3]*theta[(i+2)%3]-c[(i+2)%3]*theta[(i+1)%3];
    //                 double const denominator=d[f[i]]*sin_theta[(i+1)%3]*s[(i+2)%3];
    //                 w[f[i]]+=numerator/denominator;
    //             }
    //     }
    //     double tot_w=std::accumulate(w.begin(),w.end(),0.0);
    //     for(size_t i=0;i<n;++i)
    //         w[i]/=tot_w;
    //     return w;
    // }

    vector<double> calc_MVCweight(const Vec3d &o, double const eps = 1e-6) const {
        int vcnt = vertices.size();

        vector<double> d(vcnt, 0.0);
        vector<Vec3d> u(vcnt, Vec3d(0,0,0));
        vector<double> weights(vcnt, 0.0);
        vector<double> w_weights(vcnt, 0.0);

        for (int v = 0; v < vcnt; v++) {
            d[v] = cv::norm(vertices[v] - o);
            if (d[v] < eps) {
                weights[v] = 1.0;
                return weights;
            }
            u[v] = (vertices[v] - o) / d[v];
        }

        double sumWeights = 0.0;
        vector<double> w(vcnt);

        auto &faces=triangles;
        int fcnt = faces.size();
        for (int i = 0; i < fcnt; i++) {
            uint vids[3] = { faces[i][0], faces[i][1], faces[i][2] };
            double l[3], theta[3], sintheta[3];
            for(int i = 0; i < 3; ++i) {
                l[i] = std::clamp(cv::norm(u[vids[(i + 1) % 3]] - u[vids[(i + 2) % 3]]), 0.0, 2.0);
                theta[i] = 2 * std::asin(l[i] / 2);
                sintheta[i] = sin(theta[i]);
            }
            double h = (theta[0] + theta[1] + theta[2]) / 2.0;
            double sinh = sin(h);

            if (std::abs(M_PI - h)  < eps ) {
                // x lies inside the triangle t , use 2d barycentric coordinates :
                double w[3];
                for (int i = 0; i < 3; ++i) {
                    w[i] = sintheta[i] * d[vids[(i + 2) % 3]] * d[vids[(i + 1) % 3]];
                }
                sumWeights = w[0] + w[1] + w[2];

                weights[vids[0]] = w[0] / sumWeights;
                weights[vids[1]] = w[1] / sumWeights;
                weights[vids[2]] = w[2] / sumWeights;

                return weights;
            }

            double det = u[vids[0]].dot(u[vids[1]].cross(u[vids[2]]));
            double sign_det = signbit(det) ? -1.0 : 1.0;

            double s[3], c[3];
            for (int i = 0; i < 3; ++i) {
                c[i] = 2 * sinh * sin(h - theta[i]) / (sintheta[(i + 1) % 3] * sintheta[(i + 2) % 3]) - 1;
                s[i] = sign_det * sqrt(1 - c[i] * c[i]);
            }

            if (fabs(s[0]) > eps && fabs(s[1]) > eps && fabs(s[2]) > eps)
            {
                for (int i = 0; i < 3; ++i) {
                    double w_numerator = theta[i] - c[(i + 1) % 3] * theta[(i + 2) % 3] - c[(i + 2) % 3] * theta[(i + 1) % 3];
                    double w_denominator = d[vids[i]] * sintheta[(i + 1) % 3] * s[(i + 2) % 3];
                    double w_i = w_numerator / w_denominator;
                    sumWeights += w_i;
                    w_weights[vids[i]] += w_i;
                }
            }
        }

        for (int v = 0; v < vcnt; ++v)
            weights[v] = w_weights[v] / sumWeights;

        return weights;
    }

    vector<double> calc_MVCweight_inside_cvx_hull(vector<Vec3d> &points) {
        vector<double> weights(points.size()*vertices.size());
        // int cnt0=0, cnt1=0;
#pragma omp parallel for
        for(int i=0;i<points.size();i++) {
            vector<double> weight=calc_MVCweight(points[i]);
            if(!in_section(weight,0.0,1.0)) {
                Vec3d closest = closest_point_on_surface(points[i]);
                points[i]=closest;
                weight=calc_MVCweight(closest);
                // ++cnt0;
            }
            // if(!in_section(weight,0.0,1.0))
            //     ++cnt1;
            std::copy(weight.begin(),weight.end(),weights.begin()+i*vertices.size());
        }
        // std::cout<<" out cvx cnt "<< cnt0<< ' '<<cnt1 <<std::endl;
        return weights;
    }
};

