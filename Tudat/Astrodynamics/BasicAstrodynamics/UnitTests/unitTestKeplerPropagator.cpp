/*    Copyright (c) 2010-2017, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 *
 *    References
 *      Melman, J. Propagate software, J.C.P.Melman@tudelft.nl, 2010.
 *      NASA, Goddard Spaceflight Center. Orbit Determination Toolbox (ODTBX), NASA - GSFC Open
 *          Source Software, http://opensource.gsfc.nasa.gov/projects/ODTBX/, last accessed:
 *          31st January, 2012.
 *      ESA, GTOP Toolbox, http://www.esa.int/gsp/ACT/doc/INF/Code/globopt/GTOPtoolbox.rar.
 *
 */

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include "Tudat/Astrodynamics/BasicAstrodynamics/keplerPropagator.h"

#include <Eigen/Core>

#include <map>
#include <limits>


#include "Tudat/Astrodynamics/BasicAstrodynamics/stateVectorIndices.h"
#include "Tudat/Astrodynamics/BasicAstrodynamics/orbitalElementConversions.h"
#include "Tudat/Basics/testMacros.h"

#include "Tudat/Mathematics/BasicMathematics/basicMathematicsFunctions.h"
#include "Tudat/Mathematics/BasicMathematics/mathematicalConstants.h"
#include "Tudat/Basics/basicTypedefs.h"

#include "Tudat/Astrodynamics/BasicAstrodynamics/UnitTests/keplerPropagatorTestData.h"
#include "Tudat/Astrodynamics/BasicAstrodynamics/keplerPropagator.h"
#include "Tudat/InputOutput/basicInputOutput.h"


namespace tudat
{
namespace unit_tests
{

using namespace orbital_element_conversions;

//! Test 1: Comparison of propagateKeplerOrbit() output with benchmark data from (Melman, 2010).
BOOST_AUTO_TEST_CASE( testPropagateKeplerOrbit_Eccentric_Melman )
{
    // Load benchmark data.
    // This data originates from J. Melman and is generated by the software package Propagate.

    // Create propagation history map for benchmark data to be stored in.
    PropagationHistory benchmarkKeplerPropagationHistory = getMelmanBenchmarkData( );

    // Propagate to final state in Keplerian elements.
    Eigen::Vector6d computedFinalStateInKeplerianElements
            = propagateKeplerOrbit(
                benchmarkKeplerPropagationHistory.begin( )->second,
                benchmarkKeplerPropagationHistory.rbegin( )->first -
                benchmarkKeplerPropagationHistory.begin( )->first,
                getMelmanEarthGravitationalParameter( ) );

    // Check that computed results match expected results.
    BOOST_CHECK_CLOSE_FRACTION(
                benchmarkKeplerPropagationHistory.rbegin( )->second( 5 ),
                basic_mathematics::computeModulo( computedFinalStateInKeplerianElements( 5 ),
                                                  2.0 * mathematical_constants::PI ), 1.0e-8 );
}

//! Test 2: Comparison of kepprop2b() test output from (GSFC, 2012) using modulo option.
BOOST_AUTO_TEST_CASE( testPropagateKeplerOrbit_Eccentric_kepprop2b_modulo )
{
    // Create expected propagation history.
    PropagationHistory expectedPropagationHistory = getODTBXBenchmarkData( );

    // Set Earth gravitational parameter [m^3 s^-2].
    const double earthGravitationalParameter = 398600.4415e9;

    // Set time step for ODTBX benchmark data.
    const double timeStep = 8640.0;

    // Compute propagation history.
    PropagationHistory computedPropagationHistory;
    computedPropagationHistory[ 0.0 ] = expectedPropagationHistory[ 0.0 ];

    for ( unsigned int i = 1; i < expectedPropagationHistory.size( ); i++ )
    {
        computedPropagationHistory[ static_cast< double >( i ) * timeStep ]
                = propagateKeplerOrbit(
                    computedPropagationHistory[ static_cast< double >( i - 1 ) * timeStep ],
                    timeStep, earthGravitationalParameter  );

        computedPropagationHistory[ static_cast< double >( i ) * timeStep ]( 5 )
                = basic_mathematics::computeModulo(
                    computedPropagationHistory[ static_cast< double >( i ) * timeStep ]( 5 ),
                    2.0 * mathematical_constants::PI );

        // Check that computed results match expected results.
        TUDAT_CHECK_MATRIX_CLOSE_FRACTION(
                    computedPropagationHistory[ static_cast< double >( i ) * timeStep ],
                    expectedPropagationHistory[ static_cast< double >( i ) * timeStep ],
                    1.0e-13 );
    }
}

//! Test 3: Comparison of kepprop2b() test output from (GSFC, 2012), propagating backwards.
BOOST_AUTO_TEST_CASE( testPropagateKeplerOrbit_Eccentric_kepprop2b_backwards )
{
    // Create expected propagation history.
    PropagationHistory expectedPropagationHistory = getODTBXBenchmarkData( );

    // Set Earth gravitational parameter [m^3 s^-2].
    const double earthGravitationalParameter = 398600.4415e9;

    // Set time step for ODTBX benchmark data.
    const double timeStep = 8640.0;

    // Compute propagation history.
    PropagationHistory computedPropagationHistory;
    computedPropagationHistory[ 10.0 * 8640.0 ] = expectedPropagationHistory[ 10.0 * 8640.0 ];

    for ( int i = expectedPropagationHistory.size( ) - 2; i >= 0; i-- )
    {
        computedPropagationHistory[ static_cast< double >( i ) * timeStep ]
                = propagateKeplerOrbit(
                    computedPropagationHistory[ static_cast< double >( i + 1 ) * timeStep ],
                    -timeStep, earthGravitationalParameter );

        computedPropagationHistory[ static_cast< double >( i ) * timeStep ]( 5 )
                = basic_mathematics::computeModulo(
                    computedPropagationHistory[ static_cast< double >( i ) * timeStep ]( 5 ),
                    2.0 * mathematical_constants::PI );

        // Check that computed results match expected results.
        TUDAT_CHECK_MATRIX_CLOSE_FRACTION(
                    computedPropagationHistory[ static_cast< double >( i ) * timeStep ],
                    expectedPropagationHistory[ static_cast< double >( i ) * timeStep ],
                    1.0e-13 );
    }
}

//! Test 4: Comparison of hyperbolic kepler propagation with that of GTOP.
BOOST_AUTO_TEST_CASE( testPropagateKeplerOrbit_hyperbolic_GTOP )
{
    // Load the expected propagation history.
    PropagationHistory expectedPropagationHistory = getGTOPBenchmarkData( );

    // Set the time step for the GTOP benchmark data to 100 days.
    const double timeStep = 86400.0 * 100.0;

    // Compute propagation history.
    PropagationHistory computedPropagationHistory;
    computedPropagationHistory[ 0.0 ] = expectedPropagationHistory[ 0.0 ];

    for ( unsigned int i = 1; i < expectedPropagationHistory.size( ); i++ )
    {
        // Compute next entry.
        computedPropagationHistory[ static_cast< double >( i ) * timeStep ] =
                propagateKeplerOrbit(
                    computedPropagationHistory[ static_cast< double >( i - 1 ) * timeStep ],
                    timeStep, getGTOPGravitationalParameter( ) );

        // Check that computed results match expected results.
        BOOST_CHECK_CLOSE_FRACTION(
                    computedPropagationHistory[ static_cast< double >( i ) * timeStep ]( 5 ),
                    expectedPropagationHistory[ static_cast< double >( i ) * timeStep ]( 5 ),
                    1.0e-15 );
    }
}

//! Test 5: Unit test that failed on versions that caused the old modulo function to crash.
BOOST_AUTO_TEST_CASE( testPropagateKeplerOrbit_FunctionFailingOnOldModuloFunction )
{
    // Set expected true anomaly.
    const double expectedTrueAnomaly = -3.1245538487052089;

    // Set the propagation time.
    const double propagationTime = 8651869.8944624383;

    // Set the gravitational parameter (of the Sun).
    const double gravitationalParameter = 1.32712428e20;

    // Set initial Keplerian elements.
    Eigen::Vector6d keplerElements;
    keplerElements << 56618890355.593132, 0.99961601437304082, 1.0238269559089248,
            3.1526292818328812, 1.5807574453453865, 3.1478950321924795;

    // Propagate Keplerian elements.
    keplerElements = propagateKeplerOrbit(
                keplerElements, propagationTime, gravitationalParameter );

    // Check that computed results match expected results.
    BOOST_CHECK_CLOSE_FRACTION( keplerElements( trueAnomalyIndex ),
                                expectedTrueAnomaly,
                                1.0e-15 );
}

//! Test 6. Propagation test using ODTBX test Kepler elements.
BOOST_AUTO_TEST_CASE( testMeanAnomalyAgainstMeanMotion )
{
    std::vector< double > doubleErrors;
    // Test using double parameters.
    {
        double gravitationalParameter = 398600.4415e9;
        Eigen::Vector6d initialStateInKeplerianElements;

        initialStateInKeplerianElements << 42165.3431351313e3, 0.26248354351331, 0.30281462522101,
                4.71463172847351, 4.85569272927819, 2.37248926702153;
        double timeStep = 600.0;
        double meanMotion = std::sqrt( gravitationalParameter /
                                       std::pow( initialStateInKeplerianElements( 0 ), 3.0 ) );

        double initialMeanAnomaly = convertEccentricAnomalyToMeanAnomaly(
                    convertTrueAnomalyToEccentricAnomaly(
                        initialStateInKeplerianElements( 5 ), initialStateInKeplerianElements( 1 ) ),
                    initialStateInKeplerianElements( 1 ) );

        double propagationTime, propagatedMeanAnomaly;

        Eigen::Vector6d propagatedKeplerElements;


        for( int i = -25; i < 26; i++ )
        {
            propagationTime = static_cast< double >( i ) * timeStep;
            propagatedKeplerElements = propagateKeplerOrbit(
                        initialStateInKeplerianElements, propagationTime, gravitationalParameter );
            propagatedMeanAnomaly = convertEccentricAnomalyToMeanAnomaly(
                        convertTrueAnomalyToEccentricAnomaly(
                            propagatedKeplerElements( 5 ), initialStateInKeplerianElements( 1 ) ),
                        initialStateInKeplerianElements( 1 ) );
            doubleErrors.push_back( meanMotion * propagationTime - ( propagatedMeanAnomaly - initialMeanAnomaly ) );
        }
    }

    std::vector< double > longDoubleErrors;
    // Test using long double parameters.
    {
        long double gravitationalParameter = 398600.4415e9L;
        Eigen::Matrix< long double, 6, 1 > initialStateInKeplerianElements;

        initialStateInKeplerianElements << 42165.3431351313e3L, 0.26248354351331L, 0.30281462522101L,
                4.71463172847351L, 4.85569272927819L, 2.37248926702153L;
        long double timeStep = 600.0L;
        long double meanMotion = std::sqrt( gravitationalParameter /
                                            ( initialStateInKeplerianElements( 0 ) *
                                              initialStateInKeplerianElements( 0 ) *
                                              initialStateInKeplerianElements( 0 ) ) );

        long double initialMeanAnomaly = convertEccentricAnomalyToMeanAnomaly< long double >(
                    convertTrueAnomalyToEccentricAnomaly< long double >(
                        initialStateInKeplerianElements( 5 ), initialStateInKeplerianElements( 1 ) ),
                    initialStateInKeplerianElements( 1 ) );

        long double propagationTime, propagatedMeanAnomaly;

        Eigen::Matrix< long double, 6, 1 > propagatedKeplerElements;


        for( int i = -25; i < 26; i++ )
        {
            propagationTime = static_cast< long double >( i ) * timeStep;

            propagatedKeplerElements = propagateKeplerOrbit< long double >(
                        initialStateInKeplerianElements, propagationTime, gravitationalParameter );
            propagatedMeanAnomaly = convertEccentricAnomalyToMeanAnomaly< long double >(
                        convertTrueAnomalyToEccentricAnomaly< long double >(
                            propagatedKeplerElements( 5 ), initialStateInKeplerianElements( 1 ) ),
                        initialStateInKeplerianElements( 1 ) );

            longDoubleErrors.push_back( static_cast< double >(
                                            meanMotion * propagationTime -
                                            ( propagatedMeanAnomaly - initialMeanAnomaly ) ) );
        }
    }

    for( unsigned int i = 0; i < doubleErrors.size( ); i++ )
    {
        BOOST_CHECK_SMALL( std::fabs( static_cast< double >( longDoubleErrors.at( i ) ) ),
                           static_cast< double >( 5.0 * std::numeric_limits< long double >::epsilon( ) ) );
        BOOST_CHECK_SMALL( std::fabs( static_cast< double >( doubleErrors.at( i ) ) ),
                           static_cast< double >( 5.0 * std::numeric_limits< double >::epsilon( ) ) );
    }
}
} // namespace unit_tests
} // namespace tudat
