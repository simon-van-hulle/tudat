/*    Copyright (c) 2010-2016, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#include <iostream>

#include "Tudat/Astrodynamics/OrbitDetermination/EstimatableParameters/radiationPressureCoefficient.h"
#include "Tudat/Astrodynamics/OrbitDetermination/AccelerationPartials/radiationPressureAccelerationPartial.h"

namespace tudat
{

namespace orbit_determination
{

namespace partial_derivatives
{

Eigen::Vector3d computePartialOfCannonBallRadiationPressureAccelerationWrtRadiationPressureCoefficient(
        const double radiationPressure,
        const double area,
        const double bodyMass,
        const Eigen::Vector3d& vectorToSource )
{
    return -radiationPressure * area / bodyMass * vectorToSource;
}

std::pair< boost::function< void( Eigen::MatrixXd& ) >, int > CannonBallRadiationPressurePartial::getParameterPartialFunction(
        boost::shared_ptr< estimatable_parameters::EstimatableParameter< double > > parameter )
{
    boost::function< void( Eigen::MatrixXd& ) > partialFunction;
    int numberOfRows = 0;
    if( parameter->getParameterName( ).second.first == acceleratedBody_ )
    {
        switch( parameter->getParameterName( ).first )
        {
        case estimatable_parameters::radiation_pressure_coefficient:

            partialFunction = boost::bind( &CannonBallRadiationPressurePartial::wrtRadiationPressureCoefficient,
                                           this, _1 );
            numberOfRows = 1;

            break;
        default:
            break;
        }
    }
    return std::make_pair( partialFunction, numberOfRows );
}


std::pair< boost::function< void( Eigen::MatrixXd& ) >, int > CannonBallRadiationPressurePartial::getParameterPartialFunction(
        boost::shared_ptr< estimatable_parameters::EstimatableParameter< Eigen::VectorXd > > parameter )
{
    boost::function< void( Eigen::MatrixXd& ) > partialFunction;
    int numberOfRows = 0;
    if( parameter->getParameterName( ).second.first == acceleratedBody_ )
    {
        switch( parameter->getParameterName( ).first )
        {

        default:
            break;
        }
    }
    return std::make_pair( partialFunction, numberOfRows );

}


}

}

}


