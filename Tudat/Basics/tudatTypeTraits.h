/*    Copyright (c) 2010-2018, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 *
 */

#ifndef TUDAT_TYPE_TRAITS_H
#define TUDAT_TYPE_TRAITS_H

#include "Tudat/Basics/timeType.h"

namespace tudat
{

template< typename T >
struct is_state_scalar {
  static const bool value = false;
};

template< >
struct is_state_scalar< double > {
  static const bool value = true;
};

template< >
struct is_state_scalar< long double > {
  static const bool value = true;
};

template< typename T >
struct is_time_type {
  static const bool value = false;
};

template< >
struct is_time_type< double > {
  static const bool value = true;
};

template< >
struct is_time_type< Time > {
  static const bool value = true;
};

template< typename StateScalarType, typename TimeType >
struct is_state_scalar_and_time_type {
  static const bool value = ( is_time_type< TimeType >::value && is_state_scalar< StateScalarType >::value );
};

template< typename T >
struct is_direct_gravity_partial {
    static const bool value = false;
};

namespace acceleration_partials
{
class CentralGravitationPartial;
}

template< >
struct is_direct_gravity_partial< acceleration_partials::CentralGravitationPartial > {
    static const bool value = true;
};

namespace acceleration_partials
{
class SphericalHarmonicsGravityPartial;
}

template< >
struct is_direct_gravity_partial< acceleration_partials::SphericalHarmonicsGravityPartial > {
    static const bool value = true;
};

namespace acceleration_partials
{
class MutualSphericalHarmonicsGravityPartial;
}

template< >
struct is_direct_gravity_partial< acceleration_partials::MutualSphericalHarmonicsGravityPartial > {
    static const bool value = true;
};

template< typename T >
struct is_direct_gravity_acceleration {
    static const bool value = false;
};

namespace gravitation
{
template< typename ReturnType >
class CentralGravitationalAccelerationModel;
}

template< >
struct is_direct_gravity_acceleration< gravitation::CentralGravitationalAccelerationModel< Eigen::Vector3d > > {
    static const bool value = true;
};

namespace gravitation
{
class SphericalHarmonicsGravitationalAccelerationModel;
}

template< >
struct is_direct_gravity_acceleration< gravitation::SphericalHarmonicsGravitationalAccelerationModel > {
    static const bool value = true;
};

namespace gravitation
{
class MutualSphericalHarmonicsGravitationalAccelerationModel;
}

template< >
struct is_direct_gravity_acceleration< gravitation::MutualSphericalHarmonicsGravitationalAccelerationModel > {
    static const bool value = true;
};





} // namespace tudat

#endif // TUDAT_TYPE_TRAITS_H
