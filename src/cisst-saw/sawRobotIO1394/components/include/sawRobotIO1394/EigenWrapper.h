#ifndef __SAW_EIGEN_WRAPPER_H
#define __SAW_EIGEN_WRAPPER_H

#include <stdint.h>
#include <vector>

// NOTE: We are augmenting the Eigen::Matrix class with the contents of
// sawRobotIO1394/cisstVectorEigenAddons.h
#include <Eigen/Dense>

//! Vector Typedefs
typedef Eigen::VectorXd vctDoubleVec;
typedef Eigen::VectorXi vctIntVec;
typedef Eigen::Matrix<uint32_t,Eigen::Dynamic,1> vctLongVec;
typedef Eigen::Matrix<bool,Eigen::Dynamic,1> vctBoolVec;

//! Matrix Typedefs
typedef Eigen::MatrixXd vctDoubleMat;

#endif // ifndef __SAW_EIGEN_WRAPPER_H
