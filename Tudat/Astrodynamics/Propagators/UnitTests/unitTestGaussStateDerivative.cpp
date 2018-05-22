/*    Copyright (c) 2010-2018, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#define BOOST_TEST_MAIN

#include <limits>
#include <string>
#include <thread>

#include <boost/make_shared.hpp>
#include <boost/test/unit_test.hpp>

#include "Tudat/SimulationSetup/tudatSimulationHeader.h"
namespace tudat
{
namespace unit_tests
{

BOOST_AUTO_TEST_SUITE( test_gauss_propagator )

// Test Gauss propagator for point mass central body.
BOOST_AUTO_TEST_CASE( testGaussPopagatorForPointMassCentralBodies )
{
    for( int propagatorType = 0; propagatorType < 2; propagatorType++ )
    {
        // Test simulation for different central body cases
        //    for( unsigned int simulationCase = 1; simulationCase < 2; simulationCase++ )
        //    {
        //Using declarations.
        using namespace tudat::interpolators;
        using namespace tudat::numerical_integrators;
        using namespace tudat::spice_interface;
        using namespace tudat::simulation_setup;
        using namespace tudat::basic_astrodynamics;
        using namespace tudat::orbital_element_conversions;
        using namespace tudat::propagators;


        //Load spice kernels.
        spice_interface::loadStandardSpiceKernels( );

        // Define bodies in simulation.
        unsigned int totalNumberOfBodies = 7;
        std::vector< std::string > bodyNames;
        bodyNames.resize( totalNumberOfBodies );
        bodyNames[ 0 ] = "Earth";
        bodyNames[ 1 ] = "Mars";
        bodyNames[ 2 ] = "Sun";
        bodyNames[ 3 ] = "Venus";
        bodyNames[ 4 ] = "Moon";
        bodyNames[ 5 ] = "Mercury";
        bodyNames[ 6 ] = "Jupiter";

        double initialEphemerisTime = 1.0E7;
        double finalEphemerisTime = 2.0E7;
        double maximumTimeStep = 250.0;
        double buffer = 5.0 * maximumTimeStep;

        // Create bodies needed in simulation
        std::map< std::string, boost::shared_ptr< BodySettings > > bodySettings =
                getDefaultBodySettings( bodyNames, initialEphemerisTime - buffer , finalEphemerisTime + buffer );

        for(  std::map< std::string, boost::shared_ptr< BodySettings > >::iterator bodySettingIterator =
              bodySettings.begin( ); bodySettingIterator != bodySettings.end( ); bodySettingIterator++ )
        {
            bodySettingIterator->second->ephemerisSettings->resetFrameOrientation( "J2000" );
            bodySettingIterator->second->rotationModelSettings->resetOriginalFrame( "J2000" );
        }

        NamedBodyMap bodyMap = createBodies( bodySettings );

        setGlobalFrameBodyEphemerides( bodyMap, "SSB", "J2000" );

        // Set accelerations between bodies that are to be taken into account.
        SelectedAccelerationMap accelerationMap;
        std::map< std::string, std::vector< boost::shared_ptr< AccelerationSettings > > > accelerationsOfEarth;
        accelerationsOfEarth[ "Sun" ].push_back( boost::make_shared< AccelerationSettings >( central_gravity ) );
        accelerationsOfEarth[ "Moon" ].push_back( boost::make_shared< AccelerationSettings >( central_gravity ) );
        accelerationsOfEarth[ "Jupiter" ].push_back( boost::make_shared< AccelerationSettings >( central_gravity ) );
        accelerationMap[ "Earth" ] = accelerationsOfEarth;

        std::map< std::string, std::vector< boost::shared_ptr< AccelerationSettings > > > accelerationsOfMars;
        accelerationsOfMars[ "Sun" ].push_back( boost::make_shared< AccelerationSettings >( central_gravity ) );
        accelerationsOfMars[ "Venus" ].push_back( boost::make_shared< AccelerationSettings >( central_gravity ) );
        accelerationsOfMars[ "Jupiter" ].push_back( boost::make_shared< AccelerationSettings >( central_gravity ) );
        accelerationMap[ "Mars" ] = accelerationsOfMars;

        std::map< std::string, std::vector< boost::shared_ptr< AccelerationSettings > > > accelerationsOfVenus;
        accelerationsOfVenus[ "Sun" ].push_back( boost::make_shared< AccelerationSettings >( central_gravity ) );
        accelerationsOfVenus[ "Mars" ].push_back( boost::make_shared< AccelerationSettings >( central_gravity ) );
        accelerationsOfVenus[ "Jupiter" ].push_back( boost::make_shared< AccelerationSettings >( central_gravity ) );
        accelerationMap[ "Venus" ] = accelerationsOfVenus;

        // Propagate Earth, Mars and Moon
        std::vector< std::string > bodiesToPropagate;
        bodiesToPropagate.push_back( "Earth" );
        bodiesToPropagate.push_back( "Mars" );
        bodiesToPropagate.push_back( "Venus" );

        unsigned int numberOfNumericalBodies = bodiesToPropagate.size( );

        // Define central bodies: all Sun for simulationCase = 0, Earth and Mars: Sun, Moon: Earth for simulationCase = 1
        std::vector< std::string > centralBodies;
        std::map< std::string, std::string > centralBodyMap;
        centralBodies.resize( numberOfNumericalBodies );
        for( unsigned int i = 0; i < numberOfNumericalBodies; i++ )
        {
            centralBodies[ i ] = "Sun";
            //        if( i == 2 )
            //        {
            //            centralBodies[ i ] = "Earth";
            //        }
            //        else
            {
                centralBodies[ i ] = "Sun";
            }
            centralBodyMap[ bodiesToPropagate[ i ] ] = centralBodies[ i ];
        }


        // Get initial states for bodies.
        Eigen::VectorXd systemInitialState = Eigen::VectorXd( bodiesToPropagate.size( ) * 6 );
        for( unsigned int i = 0; i < numberOfNumericalBodies ; i++ )
        {
            systemInitialState.segment( i * 6 , 6 ) =
                    bodyMap[ bodiesToPropagate[ i ] ]->getStateInBaseFrameFromEphemeris( initialEphemerisTime ) -
                    bodyMap[ centralBodies[ i ] ]->getStateInBaseFrameFromEphemeris( initialEphemerisTime );
        }

        // Avoid degradation of performance in Kepler element conversions
        Eigen::Vector6d earthInitialKeplerElements =
                convertCartesianToKeplerianElements(
                    Eigen::Vector6d( systemInitialState.segment( 0, 6 ) ), bodyMap.at( "Earth" )->getGravityFieldModel( )->getGravitationalParameter( ) +
                    bodyMap.at( "Sun" )->getGravityFieldModel( )->getGravitationalParameter( ) );

        earthInitialKeplerElements( 2 ) = mathematical_constants::PI - earthInitialKeplerElements( 2 );
        earthInitialKeplerElements( 4 ) = earthInitialKeplerElements( 4 ) - 0.4;
        systemInitialState.segment( 0, 6 ) =
                convertKeplerianToCartesianElements(
                    earthInitialKeplerElements, bodyMap.at( "Earth" )->getGravityFieldModel( )->getGravitationalParameter( ) +
                    bodyMap.at( "Sun" )->getGravityFieldModel( )->getGravitationalParameter( ) );

        // Create acceleratiuon models.
        AccelerationMap accelerationModelMap = createAccelerationModelsMap(
                    bodyMap, accelerationMap, centralBodyMap );

        // Create integrator settings.
        boost::shared_ptr< IntegratorSettings< > > integratorSettings =
                boost::make_shared< IntegratorSettings< > >
                ( rungeKutta4,
                  initialEphemerisTime, 250.0 );

        // Create propagation settings (Cowell)
        boost::shared_ptr< TranslationalStatePropagatorSettings< double > > propagatorSettings =
                boost::make_shared< TranslationalStatePropagatorSettings< double > >
                ( centralBodies, accelerationModelMap, bodiesToPropagate, systemInitialState, finalEphemerisTime );

        // Propagate orbit with Cowell method
        SingleArcDynamicsSimulator< double > dynamicsSimulator2(
                    bodyMap, integratorSettings, propagatorSettings, true, false, true );

        // Define ephemeris interrogation settings.
        double initialTestTime = initialEphemerisTime;
        double finalTestTime = finalEphemerisTime;
        double testTimeStep = 1000.0;

        // Get resutls of Cowell integration at given times.
        double currentTestTime = initialTestTime;
        std::map< double, Eigen::Matrix< double, 18, 1 > > cowellIntegrationResults;
        bodyMap[ "Earth" ]->recomputeStateOnNextCall( );
        bodyMap[ "Mars" ]->recomputeStateOnNextCall( );
        bodyMap[ "Venus" ]->recomputeStateOnNextCall( );

        while( currentTestTime < finalTestTime )
        {
            cowellIntegrationResults[ currentTestTime ].segment( 0, 6 ) =
                    bodyMap[ "Earth" ]->getStateInBaseFrameFromEphemeris( currentTestTime );
            cowellIntegrationResults[ currentTestTime ].segment( 6, 6 ) =
                    bodyMap[ "Mars" ]->getStateInBaseFrameFromEphemeris( currentTestTime );
            cowellIntegrationResults[ currentTestTime ].segment( 12, 6 ) =
                    bodyMap[ "Venus" ]->getStateInBaseFrameFromEphemeris( currentTestTime );

            currentTestTime += testTimeStep;
        }

        TranslationalPropagatorType translationalPropagatorType = undefined_translational_propagator;
        if( propagatorType == 0 )
        {
            translationalPropagatorType = gauss_modified_equinoctial;
        }
        else
        {
            translationalPropagatorType = gauss_keplerian;
        }

        // Create propagation settings (Gauss)
        propagatorSettings = boost::make_shared< TranslationalStatePropagatorSettings< double > >
                ( centralBodies, accelerationModelMap, bodiesToPropagate, systemInitialState, finalEphemerisTime, translationalPropagatorType );

        // Propagate orbit with Gauss method
        SingleArcDynamicsSimulator< double > dynamicsSimulator(
                    bodyMap, integratorSettings, propagatorSettings, true, false, true );

        // Get resutls of Gauss integration at given times.
        currentTestTime = initialTestTime;
        std::map< double, Eigen::Matrix< double, 18, 1 > > gaussIntegrationResults;
        while( currentTestTime < finalTestTime )
        {
            gaussIntegrationResults[ currentTestTime ].segment( 0, 6 ) =
                    bodyMap[ "Earth" ]->getStateInBaseFrameFromEphemeris( currentTestTime );
            gaussIntegrationResults[ currentTestTime ].segment( 6, 6 ) =
                    bodyMap[ "Mars" ]->getStateInBaseFrameFromEphemeris( currentTestTime );
            gaussIntegrationResults[ currentTestTime ].segment( 12, 6 ) =
                    bodyMap[ "Venus" ]->getStateInBaseFrameFromEphemeris( currentTestTime );
            currentTestTime += testTimeStep;
        }

        // Compare results of Cowell and Gauss propagations
        std::map< double, Eigen::Matrix< double, 18, 1 > >::iterator gaussIterator = gaussIntegrationResults.begin( );
        Eigen::Vector6d currentCartesianState, currentKeplerianStateDifference;
        std::map< double, Eigen::Matrix< double, 18, 1 > >::iterator cowellIterator = cowellIntegrationResults.begin( );
        for( unsigned int i = 0; i < gaussIntegrationResults.size( ); i++ )
        {

            for( int j= 0; j< 3; j++ )
            {
                BOOST_CHECK_SMALL( ( gaussIterator->second - cowellIterator->second ).segment( j, 1 )( 0 ), 0.01 );
            }

            for( int j = 6; j < 9; j++ )
            {
                BOOST_CHECK_SMALL( ( gaussIterator->second - cowellIterator->second ).segment( j, 1 )( 0 ), 0.01 );
            }

            for( int j = 12; j < 15; j++ )
            {
                BOOST_CHECK_SMALL( ( gaussIterator->second - cowellIterator->second ).segment( j, 1 )( 0 ), 0.075 );
            }

            for( int j = 3; j < 6; j++ )
            {
                BOOST_CHECK_SMALL( ( gaussIterator->second - cowellIterator->second ).segment( j, 1 )( 0 ), 1.0E-8 );
            }

            for( int j = 9; j < 12; j++ )
            {
                BOOST_CHECK_SMALL( ( gaussIterator->second - cowellIterator->second ).segment( j, 1 )( 0 ), 1.0E-8 );

            }

            for( int j = 15; j < 18; j++ )
            {
                BOOST_CHECK_SMALL( ( gaussIterator->second - cowellIterator->second ).segment( j, 1 )( 0 ), 1.0E-6 );
            }
            gaussIterator++;
            cowellIterator++;
        }
    }
}

// Test Gauss propagator for point mass, and spherical harmonics central body.
BOOST_AUTO_TEST_CASE( testGaussPopagatorForSphericalHarmonicCentralBodies )
{
    using namespace tudat;
    using namespace simulation_setup;
    using namespace propagators;
    using namespace numerical_integrators;
    using namespace orbital_element_conversions;
    using namespace basic_mathematics;
    using namespace gravitation;
    using namespace numerical_integrators;

    // Load Spice kernels.
    spice_interface::loadStandardSpiceKernels( );

    // Set simulation time settings.
    const double simulationStartEpoch = 0.0;
    const double simulationEndEpoch = tudat::physical_constants::JULIAN_DAY;

    for( int propagatorType = 0; propagatorType < 2; propagatorType++ )
    {
        TranslationalPropagatorType translationalPropagatorType;
        if( propagatorType == 0 )
        {
            translationalPropagatorType = gauss_modified_equinoctial;
        }
        else
        {
            translationalPropagatorType = gauss_keplerian;
        }
        for( unsigned int simulationCase = 0; simulationCase < 4; simulationCase++ )
        {
            std::cout << "Simulation case : " << simulationCase << std::endl << std::endl;


            // Define body settings for simulation.
            std::vector< std::string > bodiesToCreate;
            bodiesToCreate.push_back( "Sun" );
            bodiesToCreate.push_back( "Earth" );
            bodiesToCreate.push_back( "Moon" );
            bodiesToCreate.push_back( "Mars" );
            bodiesToCreate.push_back( "Venus" );

            // Create body objects.
            std::map< std::string, boost::shared_ptr< BodySettings > > bodySettings =
                    getDefaultBodySettings( bodiesToCreate, simulationStartEpoch - 300.0, simulationEndEpoch + 300.0 );
            for( unsigned int i = 0; i < bodiesToCreate.size( ); i++ )
            {
                bodySettings[ bodiesToCreate.at( i ) ]->ephemerisSettings->resetFrameOrientation( "J2000" );
                bodySettings[ bodiesToCreate.at( i ) ]->rotationModelSettings->resetOriginalFrame( "J2000" );
            }
            NamedBodyMap bodyMap = createBodies( bodySettings );

            // Create spacecraft object.
            bodyMap[ "Vehicle" ] = boost::make_shared< simulation_setup::Body >( );
            bodyMap[ "Vehicle" ]->setConstantBodyMass( 400.0 );
            bodyMap[ "Vehicle" ]->setEphemeris( boost::make_shared< ephemerides::TabulatedCartesianEphemeris< > >(
                                                    boost::shared_ptr< interpolators::OneDimensionalInterpolator
                                                    < double, Eigen::Vector6d  > >( ), "Earth", "J2000" ) );
            boost::shared_ptr< RadiationPressureInterfaceSettings > vehicleRadiationPressureSettings =
                    boost::make_shared< CannonBallRadiationPressureInterfaceSettings >(
                        "Sun", 4.0, 1.2, boost::assign::list_of( "Earth" ) );
            bodyMap[ "Vehicle" ]->setRadiationPressureInterface(
                        "Sun", createRadiationPressureInterface(
                            vehicleRadiationPressureSettings, "Vehicle", bodyMap ) );


            // Finalize body creation.
            setGlobalFrameBodyEphemerides( bodyMap, "SSB", "J2000" );

            // Define propagator settings variables.
            SelectedAccelerationMap accelerationMap;
            std::vector< std::string > bodiesToPropagate;
            std::vector< std::string > centralBodies;

            // Define propagation settings.
            std::map< std::string, std::vector< boost::shared_ptr< AccelerationSettings > > > accelerationsOfVehicle;

            // Use only central gravity for Earth
            if( simulationCase < 2 )
            {
                accelerationsOfVehicle[ "Earth" ].push_back( boost::make_shared< AccelerationSettings >(
                                                                 basic_astrodynamics::central_gravity ) );
            }
            // Use spherical harmonics for Earth
            else
            {
                accelerationsOfVehicle[ "Earth" ].push_back(
                            boost::make_shared< SphericalHarmonicAccelerationSettings >( 5, 5 ) );

            }

            // Use perturbations other than Earth gravity
            if( simulationCase % 2 == 0 )
            {
                accelerationsOfVehicle[ "Sun" ].push_back( boost::make_shared< AccelerationSettings >(
                                                               basic_astrodynamics::central_gravity ) );
                accelerationsOfVehicle[ "Moon" ].push_back( boost::make_shared< AccelerationSettings >(
                                                                basic_astrodynamics::central_gravity ) );
                accelerationsOfVehicle[ "Mars" ].push_back( boost::make_shared< AccelerationSettings >(
                                                                basic_astrodynamics::central_gravity ) );
                accelerationsOfVehicle[ "Venus" ].push_back( boost::make_shared< AccelerationSettings >(
                                                                 basic_astrodynamics::central_gravity ) );
                accelerationsOfVehicle[ "Sun" ].push_back( boost::make_shared< AccelerationSettings >(
                                                               basic_astrodynamics::cannon_ball_radiation_pressure ) );
            }
            accelerationMap[  "Vehicle" ] = accelerationsOfVehicle;
            bodiesToPropagate.push_back( "Vehicle" );
            centralBodies.push_back( "Earth" );
            basic_astrodynamics::AccelerationMap accelerationModelMap = createAccelerationModelsMap(
                        bodyMap, accelerationMap, bodiesToPropagate, centralBodies );

            // Set Keplerian elements for Vehicle.
            Eigen::Vector6d vehicleInitialStateInKeplerianElements;
            vehicleInitialStateInKeplerianElements( semiMajorAxisIndex ) = 8000.0E3;
            vehicleInitialStateInKeplerianElements( eccentricityIndex ) = 0.1;
            vehicleInitialStateInKeplerianElements( inclinationIndex ) = unit_conversions::convertDegreesToRadians( 85.3 );
            vehicleInitialStateInKeplerianElements( argumentOfPeriapsisIndex )
                    = unit_conversions::convertDegreesToRadians( 235.7 );
            vehicleInitialStateInKeplerianElements( longitudeOfAscendingNodeIndex )
                    = unit_conversions::convertDegreesToRadians( 23.4 );
            vehicleInitialStateInKeplerianElements( trueAnomalyIndex ) = unit_conversions::convertDegreesToRadians( 139.87 );

            double earthGravitationalParameter = bodyMap.at( "Earth" )->getGravityFieldModel( )->getGravitationalParameter( );
            const Eigen::Vector6d vehicleInitialState = convertKeplerianToCartesianElements(
                        vehicleInitialStateInKeplerianElements, earthGravitationalParameter );

            // Define propagator settings (Cowell)
            boost::shared_ptr< TranslationalStatePropagatorSettings< double > > propagatorSettings =
                    boost::make_shared< TranslationalStatePropagatorSettings< double > >
                    ( centralBodies, accelerationModelMap, bodiesToPropagate, vehicleInitialState, simulationEndEpoch );

            // Define integrator settings.
            const double fixedStepSize = 5.0;
            boost::shared_ptr< IntegratorSettings< > > integratorSettings =
                    boost::make_shared< IntegratorSettings< > >
                    ( rungeKutta4, 0.0, fixedStepSize );

            // Propagate orbit with Cowell method
            SingleArcDynamicsSimulator< double > dynamicsSimulator2(
                        bodyMap, integratorSettings, propagatorSettings, true, false, true );

            // Define ephemeris interrogation settings.
            double initialTestTime = simulationStartEpoch;
            double finalTestTime = simulationEndEpoch - 10.0 * fixedStepSize;
            double testTimeStep = 1.0E4;

            // Get resutls of Cowell integration at given times.
            double currentTestTime = initialTestTime;
            std::map< double, Eigen::Matrix< double, 6, 1 > > cowellIntegrationResults;
            while( currentTestTime < finalTestTime )
            {
                cowellIntegrationResults[ currentTestTime ].segment( 0, 6 ) =
                        bodyMap[ "Vehicle" ]->getEphemeris( )->getCartesianState( currentTestTime );

                currentTestTime += testTimeStep;
            }

            // Create propagation settings (Gauss)
            propagatorSettings = boost::make_shared< TranslationalStatePropagatorSettings< double > >
                    ( centralBodies, accelerationModelMap, bodiesToPropagate, vehicleInitialState, simulationEndEpoch, translationalPropagatorType );

            // Propagate orbit with Gauss method
            SingleArcDynamicsSimulator< double > dynamicsSimulator(
                        bodyMap, integratorSettings, propagatorSettings, true, false, true );

            // Get resutls of Gauss integration at given times.
            currentTestTime = initialTestTime;
            std::map< double, Eigen::Matrix< double, 6, 1 > > gaussIntegrationResults;
            while( currentTestTime < finalTestTime )
            {
                gaussIntegrationResults[ currentTestTime ].segment( 0, 6 ) =
                        bodyMap[ "Vehicle" ]->getEphemeris( )->getCartesianState( currentTestTime );
                currentTestTime += testTimeStep;
            }

            // Compare results of Cowell and Gauss propagations
            std::map< double, Eigen::Matrix< double, 6, 1 > >::iterator gaussIterator = gaussIntegrationResults.begin( );
            std::map< double, Eigen::Matrix< double, 6, 1 > >::iterator cowellIterator = cowellIntegrationResults.begin( );
            for( unsigned int i = 0; i < gaussIntegrationResults.size( ); i++ )
            {
                //std::cout << ( gaussIterator->second - cowellIterator->second ).transpose( ) << std::endl;
                for( int j= 0; j< 3; j++ )
                {
                    BOOST_CHECK_SMALL( ( gaussIterator->second - cowellIterator->second )( j ), 0.02 );
                }

                for( int j = 3; j < 6; j++ )
                {
                    BOOST_CHECK_SMALL( ( gaussIterator->second - cowellIterator->second )( j ), 1.0E-5 );

                }
                gaussIterator++;
                cowellIterator++;
            }
        }
    }
}
BOOST_AUTO_TEST_SUITE_END( )


}

}


