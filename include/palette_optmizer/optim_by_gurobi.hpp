#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>

#include <opencv2/opencv.hpp>
#include <gurobi_c++.h>

#include "utility.hpp"


struct Optimizer {
    static int const n_channels=Vec3d::channels;
    // static GRBEnv env;
    int const n;
    vector<Vec3d> const vertices;
    double const lb,ub;

    GRBEnv env;
    GRBModel model;
    vector<GRBVar> u,v;
    GRBQuadExpr obj;

    Optimizer(vector<Vec3d> const &vs,double const lower_bound=0.0,double const upper_bound=1.0): 
        n(vs.size()*n_channels), vertices(vs), lb(lower_bound), ub(upper_bound),
        model(env), u(n), v(n), 
        obj(0.0) {
        env.set(GRB_IntParam_OutputFlag, 0);
        model.set(GRB_IntParam_Presolve,2);
        for(int i=0; i<vertices.size(); ++i) 
            for(int j=0; j<n_channels; ++j) {
                int idx=i*n_channels+j;
                u[idx]=model.addVar(0.0, std::max(ub-vertices[i][j], 0.0), 0.0, GRB_CONTINUOUS, "u_"+std::to_string(i)+','+std::to_string(j));
                v[idx]=model.addVar(0.0, std::max(vertices[i][j]-lb, 0.0), 0.0, GRB_CONTINUOUS, "v_"+std::to_string(i)+','+std::to_string(j));
            }
    }

    void add_loss_abs() {
        GRBLinExpr expr=0.0;
        for(int i=0; i<n; ++i)
            expr+=u[i]+v[i];
        obj+=expr/n;
    }

    void add_loss_reconstruction(vector<Vec3d> const &pixels,vector<double> const &weights, vector<double> const &phi, double const &lambda) {
        assert(pixels.size()*vertices.size()==weights.size());
        assert(phi.empty()||pixels.size()==phi.size());

        vector<GRBLinExpr> x(n);
        for(int j=0; j<vertices.size(); ++j)
            for(int k=0; k<n_channels; ++k)
                x[j*n_channels+k]=u[j*n_channels+k]-v[j*n_channels+k]+vertices[j][k]; 
        GRBQuadExpr sum=0.0;
        for(int i=0; i<pixels.size(); ++i)
            for(int k=0; k<n_channels; ++k) {
                GRBLinExpr expr=-pixels[i][k];
                for(int j=0; j<vertices.size(); ++j)
                    expr+=x[j*n_channels+k]*weights[i*vertices.size()+j];
                if(!phi.empty())
                    sum+=phi[i]*expr*expr;
                else
                    sum+=expr*expr;
            }
        if(!phi.empty())
            obj+=(lambda/std::accumulate(phi.begin(),phi.end(),0.0))*sum;
        else
            obj+=(lambda/pixels.size())*sum;
    }

    vector<Vec3d> optimize() {
        model.setObjective(obj, GRB_MINIMIZE);
        model.optimize();
        vector<Vec3d> x(vertices.size());
        double l1_loss=0.0;
        for(int i=0; i<vertices.size(); ++i) 
            for(int j=0; j<n_channels; ++j)
                // l1_loss+=u[i*n_channels+j].get(GRB_DoubleAttr_X)+v[i*n_channels+j].get(GRB_DoubleAttr_X);
                x[i][j]=u[i*n_channels+j].get(GRB_DoubleAttr_X)-v[i*n_channels+j].get(GRB_DoubleAttr_X)+vertices[i][j];
        // qDebug()<<"L1 loss: "<<l1_loss/n<<std::endl;
        return x;
    }
};

inline vector<Vec3d> optimize(vector<Vec3d> const &vertices,
                       vector<Vec3d> const &user_pixels,
                       vector<double> const &user_weights,
                       vector<Vec3d> const &img_pixels={},
                       vector<double> const &img_weights={},
                       vector<double> const &img_phi={},
                       double const user_lambda=8.0,
                       double const img_lambda=4.0,
                       double const lower_bound=0.0,
                       double const upper_bound=1.0) {

    Optimizer opter(vertices, lower_bound, upper_bound);
    opter.add_loss_abs();
    opter.add_loss_reconstruction(user_pixels, user_weights, {}, user_lambda);
    if(!img_pixels.empty())
        opter.add_loss_reconstruction(img_pixels, img_weights, img_phi, img_lambda);
    return opter.optimize();
}

