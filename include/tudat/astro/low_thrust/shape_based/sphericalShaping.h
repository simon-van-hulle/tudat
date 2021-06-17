/*    Copyright (c) 2010-2019, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 *
 */

#ifndef TUDAT_SPHERICAL_SHAPING_H
#define TUDAT_SPHERICAL_SHAPING_H

#include "tudat/astro/basic_astro/physicalConstants.h"
#include "tudat/astro/low_thrust/shape_based/shapeBasedMethod.h"
#include "tudat/astro/low_thrust/shape_based/baseFunctionsSphericalShaping.h"
#include "tudat/astro/low_thrust/shape_based/compositeFunctionSphericalShaping.h"
#include "tudat/math/basic/basicFunction.h"
#include "tudat/math/root_finders/createRootFinder.h"
#include "tudat/math/quadrature/createNumericalQuadrature.h"
#include "tudat/math/interpolators/createInterpolator.h"
#include <cmath>
#include <vector>
#include <Eigen/Dense>
#include <map>

namespace tudat
{
namespace shape_based_methods
{


class SphericalShaping : public ShapeBasedMethod
{
public:

    //! Constructor for spherical shaping.
    SphericalShaping( const Eigen::Vector6d& initialState,
                      const Eigen::Vector6d& finalState,
                      const double requiredTimeOfFlight,
                      const double centralBodyGravitationalParameter,
                      const int numberOfRevolutions,
                      const double initialValueFreeCoefficient,
                      const std::shared_ptr< root_finders::RootFinderSettings > rootFinderSettings,
                      const double lowerBoundFreeCoefficient = TUDAT_NAN,
                      const double upperBoundFreeCoefficient = TUDAT_NAN,
                      const std::shared_ptr< interpolators::InterpolatorSettings > interpolatorSettings =
            std::make_shared< interpolators::InterpolatorSettings >( interpolators::cubic_spline_interpolator ) );

    //! Default destructor.
    ~SphericalShaping( ) { }

    //! Convert time to independent variable.
    double convertTimeToIndependentVariable( const double time );

    //! Convert independent variable to time.
    double convertIndependentVariableToTime( const double independentVariable );

    //! Returns initial value of the independent variable.
    double getInitialValueInpendentVariable( )
    {
        return initialAzimuthAngle_;
    }

    //! Returns final value of the independent variable.
    double getFinalValueInpendentVariable( )
    {
        return finalAzimuthAngle_;
    }

    //! Compute time of flight.
    double getTimeOfFlight()
    {
        return currentNormalizedTimeOfFlight_ * physical_constants::JULIAN_YEAR;
    }


    //! Compute current cartesian state.
    Eigen::Vector6d computeCurrentStateVector( const double currentAzimuthAngle );


    //! Compute current thrust acceleration in cartesian coordinates.
    Eigen::Vector3d computeCurrentThrustAcceleration( const double currentAzimuthAngle );

    Eigen::Vector3d computeCurrentThrustAcceleration( const double currentTime,
                                                      const double timeOffset )
    {
        return computeCurrentThrustAcceleration(
                    convertTimeToIndependentVariable( currentTime - timeOffset ) );
    
    }
    
    //! Compute deltaV.
    double computeDeltaV( );

protected:

    //! Update trajectory with new boundary states
    void updateBoundaryStates( const Eigen::Vector6d& initialState, const Eigen::Vector6d& finalState );

    //! Resets data structures to handle boundary conditions with new states
    void reinitializeBoundaryConstraints( );

    //! Compute the inverse of the boundary conditions matrix.
    void setInverseMatrixBoundaryConditions( );


    //! Iterate to match the required time of flight, by updating the value of the free coefficient.
    void iterateToMatchRequiredTimeOfFlight( std::shared_ptr< root_finders::RootFinderSettings > rootFinderSettings,
                                             const double lowerBound = TUDAT_NAN,
                                             const double upperBound = TUDAT_NAN,
                                             const double initialGuess = TUDAT_NAN );

    //! Computes the offset between required and actual time-of-flight for current free coefficient
    double computeRequiredTimeOfFlightError( const double freeCoefficient );

    //! Reset the value of the free coefficient, in order to match the required time of flight.
    void resetValueFreeCoefficient( const double freeCoefficient );

    //! Ensure that the boundary conditions are respected.
    void satisfyBoundaryConditions( );

    //! Compute normalized time of flight.
    void computeNormalizedTimeOfFlight( );

    void createTimeInterpolator( );




    //! Compute the initial value of the variable alpha, as defined in ... (ADD REFERENCE) to express the boundary conditions.
    double computeInitialAlphaValue( );

    //! Compute the final value of the variable alpha, as defined in ... (ADD REFERENCE) to express the boundary conditions.
    double computeFinalAlphaValue( );

    //! Compute the initial value of the constant C, as defined in ... (ADD REFERENCE) to express the boundary conditions.
    double computeInitialValueBoundariesConstant( );

    //! Compute the final value of the constant C, as defined in ... (ADD REFERENCE) to express the boundary conditions.
    double computeFinalValueBoundariesConstant( );

    //! Compute derivative of the scalar function D in the time equation (ADD REFERENCE AND EQUATION NUMBER) w.r.t. azimuth angle.
    double computeScalarFunctionTimeEquation( double currentAzimuthAngle );

    //! Compute second derivative of the scalar function D in the time equation (ADD REFERENCE AND EQUATION NUMBER) w.r.t. azimuth angle.
    double computeDerivativeScalarFunctionTimeEquation( double currentAzimuthAngle );

    //! Compute current time from azimuth angle.
    double computeCurrentTimeFromAzimuthAngle( const double currentAzimuthAngle );

    //! Compute first derivative of the azimuth angle w.r.t. time.
    double computeFirstDerivativeAzimuthAngleWrtTime( const double currentAzimuthAngle );

    //! Compute second derivative of the azimuth angle w.r.t. time.
    double computeSecondDerivativeAzimuthAngleWrtTime( const double currentAzimuthAngle );

    //! Compute current velocity in spherical coordinates parametrized by azimuth angle theta.
    Eigen::Vector3d computeVelocityVectorParametrizedByAzimuthAngle( const double currentAzimuthAngle );

    //! Compute state vector in spherical coordinates.
    Eigen::Vector6d computeStateVectorInSphericalCoordinates( const double currentAzimuthAngle );

    //! Compute current acceleration in spherical coordinates parametrized by azimuth angle theta.
    Eigen::Vector3d computeThrustAccelerationVectorParametrizedByAzimuthAngle( const double currentAzimuthAngle );


    //! Compute thrust acceleration vector in spherical coordinates.
    Eigen::Vector3d computeThrustAccelerationVectorInSphericalCoordinates( const double currentAzimuthAngle );

    //! Compute magnitude thrust acceleration.
    double computeCurrentThrustAccelerationMagnitude(
            double currentAzimuthAngle );

    //! Compute direction thrust acceleration in cartesian coordinates.
    Eigen::Vector3d computeCurrentThrustAccelerationDirection(
            double currentAzimuthAngle );

private:

    //! Central body gravitational parameter.
    double normalizedCentralBodyGravitationalParameter_;

    //! Number of revolutions.
    int numberOfRevolutions_;

    //! Initial state in spherical coordinates.
    Eigen::Vector6d initialStateSphericalCoordinates_;

    //! Final state in spherical coordinates.
    Eigen::Vector6d finalStateSphericalCoordinates_;

    //! Initial value azimuth angle.
    double initialAzimuthAngle_;

    //! Final value azimuth angle.
    double finalAzimuthAngle_;

    //! Initial state parametrised by the azimuth angle theta.
    Eigen::Vector6d initialStateParametrizedByAzimuthAngle_;

    //! Final state parametrised by the azimuth angle theta.
    Eigen::Vector6d finalStateParametrizedByAzimuthAngle_;

    //! Pointer to the spherical shaping radial distance composite function.
    std::shared_ptr< CompositeRadialFunctionSphericalShaping > radialDistanceCompositeFunction_;

    //! Pointer to the spherical shaping elevation angle composite function.
    std::shared_ptr< CompositeElevationFunctionSphericalShaping > elevationAngleCompositeFunction_;

    //! Coefficients for the radial distance composite function.
    Eigen::VectorXd coefficientsRadialDistanceFunction_;

    //! Coefficients for the elevation angle composite function.
    Eigen::VectorXd coefficientsElevationAngleFunction_;

    //! Initial guess for the free coefficient (i.e. coefficient of the second order component of the radial inverse polynomial).
    double initialValueFreeCoefficient_;

    //! Root finder settings, to be used to find the free coefficient value that ensures the time of flight is correct.
    std::shared_ptr< root_finders::RootFinderSettings > rootFinderSettings_;

    //! Lower bound for the free coefficient, to be used when trying to match the required time of flight.
    const double lowerBoundFreeCoefficient_;

    //! Upper bound for the free coefficient, to be used when trying to match the required time of flight.
    const double upperBoundFreeCoefficient_;

    //! Inverse of matrix containing the boundary conditions
    Eigen::MatrixXd inverseMatrixBoundaryConditions_;

    double currentFreeCoefficient_;

    double currentNormalizedTimeOfFlight_;


    Eigen::VectorXd boundaryValues_;

    Eigen::VectorXd secondComponentContributionNormalized_;


    std::shared_ptr< interpolators::InterpolatorSettings > interpolatorSettings_;

    std::shared_ptr< interpolators::OneDimensionalInterpolator< double, double > > interpolator_;

    std::map< double, Eigen::Vector3d > thrustAccelerationVectorCache_;
};


} // namespace shape_based_methods
} // namespace tudat

#endif // TUDAT_SPHERICAL_SHAPING_H
