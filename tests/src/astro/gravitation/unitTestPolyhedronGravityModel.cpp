/*    Copyright (c) 2010-2022, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 *
 *    References:
 *      Analytical computation of gravity effects for polyhedral bodies, M.G. D'Urso (2014), Journal of Geodesy, 88,
 *          pp. 13-29
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <limits>

#include <boost/test/tools/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include "tudat/basics/testMacros.h"
#include "tudat/astro/gravitation/polyhedronGravityField.h"
#include "tudat/astro/gravitation/polyhedronGravityModel.h"

namespace tudat
{
namespace unit_tests
{

//! Test computation of potential, gradient of potential and laplacian of potential.
BOOST_AUTO_TEST_CASE( testGravityComputation )
{
    const double tolerance = 1.0E-14;

    // Define cuboid polyhedron dimensions
    const double w = 10.0; // width
    const double h = 10.0; // height
    const double l = 20.0; // length

    // Define parameters
    const double gravitationalConstant = 6.67259e-11;
    const double density = 2670;
    const double volume = w * h * l;
    const double gravitationalParameter = gravitationalConstant * density * volume;

    // Define cuboid
    Eigen::MatrixXd verticesCoordinates(8,3);
    verticesCoordinates <<
        0.0, 0.0, 0.0,
        l, 0.0, 0.0,
        0.0, w, 0.0,
        l, w, 0.0,
        0.0, 0.0, h,
        l, 0.0, h,
        0.0, w, h,
        l, w, h;
    Eigen::MatrixXi verticesDefiningEachFacet(12,3);
    verticesDefiningEachFacet <<
        2, 1, 0,
        1, 2, 3,
        4, 2, 0,
        2, 4, 6,
        1, 4, 0,
        4, 1, 5,
        6, 5, 7,
        5, 6, 4,
        3, 6, 7,
        6, 3, 2,
        5, 3, 7,
        3, 5, 1;

    // Compute potential, gradient, laplacian and hessian, and compare with results from D'Urso (2014)

    Eigen::Vector3d bodyFixedPosition;
    double expectedPotential, computedPotential;
    Eigen::Vector3d expectedGradient, computedGradient;
    double expectedLaplacian, computedLaplacian;
    Eigen::Matrix3d expectedHessian, computedHessian;

    for (unsigned int positionId : {0,1,2,3,4})
    {
        bool testPotential = true;
        bool testGradient = true;
        bool testLaplacian = true;
        bool testHessian = true;

        // Select expected values
        if ( positionId == 0 )
        {
            (bodyFixedPosition << 0.0, 0.0, 0.0).finished();
            expectedPotential = 3.19403761604211e-5;
            (expectedGradient << 2.31329148957265e-6, 1.91973919943187e-6, 1.91973919943187e-6).finished();
            expectedLaplacian = - 0.5 * mathematical_constants::PI * gravitationalConstant * density;
            testHessian = false;
        }
        else if ( positionId == 1 )
        {
            (bodyFixedPosition << 5.0, 0.0, 0.0).finished();
            expectedPotential = 3.99993558939122e-5;
            (expectedGradient << 9.90115534890074e-7, 3.24128042248715e-6, 3.24128042248715e-6).finished();
            expectedLaplacian = - 1.0 * mathematical_constants::PI * gravitationalConstant * density;
            testHessian = false;
        }
        else if ( positionId == 2 )
        {
            (bodyFixedPosition << 0.0, 3.0, 2.0).finished();
            expectedPotential = 4.03528375471853e-5;
            (expectedGradient << 4.73368592565013e-6, 9.68164362892554e-7, 1.59674500375495e-6).finished();
            expectedLaplacian = - 2.0 * mathematical_constants::PI * gravitationalConstant * density;
            (expectedHessian <<
                -4.02204713784183E-08, 1.87140408935899E-07, 3.51261972418670E-07,
                1.87140408935899E-07, -5.01781942367494E-07, 8.58712984897779E-08,
                3.51261972418670E-07, 8.58712984897779E-08, -5.77398275537941E-07).finished();
        }
        else if ( positionId == 3 )
        {
            (bodyFixedPosition << -5.0, 5.0, 5.0).finished();
            expectedLaplacian = 0.0;
            testPotential = false;
            testGradient = false;
            testHessian = false;
        }
        else
        {
            (bodyFixedPosition << 10.0, 5.0, 5.0).finished();
            expectedLaplacian = - 4.0 * mathematical_constants::PI * gravitationalConstant * density;
            testPotential = false;
            testGradient = false;
            testHessian = false;
        }

        // Create gravity model
        gravitation::PolyhedronGravityField gravityField = gravitation::PolyhedronGravityField(
            gravitationalParameter, volume, verticesCoordinates, verticesDefiningEachFacet);
        std::function< void( Eigen::Vector3d& ) > bodyFixedPositionFunction =
                [ = ]( Eigen::Vector3d& positionOfBodySubjectToAcceleration ){
            positionOfBodySubjectToAcceleration = bodyFixedPosition; };

        gravitation::PolyhedronGravitationalAccelerationModel gravityModel =
                gravitation::PolyhedronGravitationalAccelerationModel(
                        bodyFixedPositionFunction, gravitationalParameter, volume, verticesCoordinates,
                        verticesDefiningEachFacet, gravityField.getVerticesDefiningEachEdge(),
                        gravityField.getFacetDyads(), gravityField.getEdgeDyads() );

        gravityModel.resetUpdatePotential( true );
        gravityModel.resetUpdateLaplacianOfPotential( true );

        gravityModel.updateMembers( );

        // Compute values and compare with expected ones
        if ( testPotential )
        {
            computedPotential = gravityModel.getCurrentPotential( );
            BOOST_CHECK_CLOSE_FRACTION( expectedPotential, computedPotential, tolerance );
        }
        if ( testGradient )
        {
            computedGradient = gravityModel.getAcceleration( );
            TUDAT_CHECK_MATRIX_CLOSE_FRACTION( expectedGradient, computedGradient, tolerance );
        }
        if ( testLaplacian )
        {
            computedLaplacian = gravityModel.getCurrentLaplacianOfPotential( );
            // Expected value is 0 for point 3, so add a constant to the laplacian
            if ( positionId == 3)
            {
                computedLaplacian += 0.1;
                expectedLaplacian += 0.1;
            }
            BOOST_CHECK_CLOSE_FRACTION( expectedLaplacian, computedLaplacian, tolerance );
        }

    }
}

//! Test the functionality of the polyhedron gravity field class.
BOOST_AUTO_TEST_SUITE( test_polyhedron_gravity_model )

BOOST_AUTO_TEST_SUITE_END( )

} // namespace tudat
} // namespace unit_tests