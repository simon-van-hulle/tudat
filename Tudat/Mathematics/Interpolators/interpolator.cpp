#include "Tudat/Mathematics/Interpolators/interpolator.h"

namespace tudat
{
namespace interpolators
{

template class Interpolator< double, Eigen::VectorXd >;
template class Interpolator< double, Eigen::Vector6d >;
template class Interpolator< double, Eigen::MatrixXd >;

template class Interpolator< Time, Eigen::VectorXd >;
template class Interpolator< Time, Eigen::Vector6d >;
template class Interpolator< Time, Eigen::MatrixXd >;

template class Interpolator< double, Eigen::Matrix< long double, Eigen::Dynamic, 1 > >;
template class Interpolator< double, Eigen::Matrix< long double, Eigen::Dynamic, 6 > >;
template class Interpolator< double, Eigen::Matrix< long double, Eigen::Dynamic,  Eigen::Dynamic > >;

template class Interpolator< Time, Eigen::Matrix< long double, Eigen::Dynamic, 1 > >;
template class Interpolator< Time, Eigen::Matrix< long double, Eigen::Dynamic, 6 > >;
template class Interpolator< Time, Eigen::Matrix< long double, Eigen::Dynamic, Eigen::Dynamic > >;

} // namespace interpolators

} // namespace tudat

