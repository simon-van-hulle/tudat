/*    Copyright (c) 2010-2019, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */
//
//#define BOOST_TEST_DYN_LINK
//#define BOOST_TEST_MAIN

#include <string>
#include <thread>

#include <limits>

#include <boost/test/unit_test.hpp>

#include "tudat/simulation/estimation_setup/fitOrbitToEphemeris.h"
//
//
//namespace tudat
//{
//namespace unit_tests
//{
//BOOST_AUTO_TEST_SUITE( test_fit_to_spice )

//Using declarations.
using namespace tudat;
using namespace tudat::observation_models;
using namespace tudat::orbit_determination;
using namespace tudat::estimatable_parameters;
using namespace tudat::simulation_setup;
using namespace tudat::basic_astrodynamics;
//
//BOOST_AUTO_TEST_CASE( test_FitToSpice )
//{
int main( )
{
    //Load spice kernels.
    spice_interface::loadStandardSpiceKernels( );

    spice_interface::loadSpiceKernelInTudat(
        tudat::paths::get_spice_kernels_path( )  + "/mgs_map1_ipng_mgs95j.bsp" );

    double initialTime = DateTime( 1999, 3, 10, 0, 0, 0.0 ).epoch< double >( );
    double finalTime = DateTime( 1999, 3, 12, 0, 0, 0.0 ).epoch< double >( );

    std::vector< std::string > bodyNames;
    bodyNames.push_back( "Earth" );
    bodyNames.push_back( "Mars" );
    bodyNames.push_back( "Sun" );
    bodyNames.push_back( "Moon" );
    bodyNames.push_back( "Jupiter" );
    bodyNames.push_back( "Saturn" );

    BodyListSettings bodySettings =
        getDefaultBodySettings( bodyNames, "Mars" );
    bodySettings.addSettings( "MGS" );
    bodySettings.at( "MGS" )->ephemerisSettings = std::make_shared< InterpolatedSpiceEphemerisSettings >(
        initialTime - 3600.0, finalTime + 3600.0, 30.0, "Mars" );

    for( unsigned int i = 0; i < 2; i++ )
    {

        // Create bodies needed in simulation
        SystemOfBodies bodies = createSystemOfBodies( bodySettings );

        // Create radiation pressure settings
        double referenceAreaRadiation = 4.0;
        double radiationPressureCoefficient = 1.2;
        std::vector<std::string> occultingBodies;
        occultingBodies.push_back( "Mars" );
        std::shared_ptr<RadiationPressureInterfaceSettings> asterixRadiationPressureSettings =
            std::make_shared<CannonBallRadiationPressureInterfaceSettings>(
                "Sun", referenceAreaRadiation, radiationPressureCoefficient, occultingBodies );

        // Create and set radiation pressure settings
        bodies.at( "MGS" )->setRadiationPressureInterface(
            "Sun", createRadiationPressureInterface(
                asterixRadiationPressureSettings, "MGS", bodies ));
        bodies.at( "MGS" )->setConstantBodyMass( 2000.0 );

        // Set accelerations between bodies that are to be taken into account.
        SelectedAccelerationMap accelerationMap;
        std::map<std::string, std::vector<std::shared_ptr<AccelerationSettings> > > accelerationsOfGrail;
        accelerationsOfGrail[ "Sun" ].push_back( std::make_shared<AccelerationSettings>( point_mass_gravity ));
        accelerationsOfGrail[ "Sun" ].push_back(
            std::make_shared<AccelerationSettings>( cannon_ball_radiation_pressure ));
        accelerationsOfGrail[ "Earth" ].push_back( std::make_shared<AccelerationSettings>( point_mass_gravity ));
        accelerationsOfGrail[ "Mars" ].push_back( std::make_shared<SphericalHarmonicAccelerationSettings>( 64, 64 ));
        accelerationsOfGrail[ "Mars" ].push_back( empiricalAcceleration( ));
        accelerationsOfGrail[ "Moon" ].push_back( std::make_shared<AccelerationSettings>( point_mass_gravity ));
        accelerationsOfGrail[ "Jupiter" ].push_back( std::make_shared<AccelerationSettings>( point_mass_gravity ));
        accelerationsOfGrail[ "Saturn" ].push_back( std::make_shared<AccelerationSettings>( point_mass_gravity ));

        accelerationMap[ "MGS" ] = accelerationsOfGrail;

        // Set bodies for which initial state is to be estimated and integrated.
        std::vector<std::string> bodiesToEstimate = { "MGS" };
        std::vector<std::string> centralBodies = { "Mars" };

        AccelerationMap accelerationModelMap = createAccelerationModelsMap(
            bodies, accelerationMap, bodiesToEstimate, centralBodies );

        std::vector<std::shared_ptr<estimatable_parameters::EstimatableParameterSettings> > additionalParameterNames;
        if( i > 0 )
        {
            additionalParameterNames.push_back( estimatable_parameters::radiationPressureCoefficient( "MGS" ));

            std::map<basic_astrodynamics::EmpiricalAccelerationComponents,
                std::vector<basic_astrodynamics::EmpiricalAccelerationFunctionalShapes> > empiricalComponentsToEstimate;
            empiricalComponentsToEstimate[ radial_empirical_acceleration_component ].push_back( constant_empirical );
            empiricalComponentsToEstimate[ radial_empirical_acceleration_component ].push_back( sine_empirical );
            empiricalComponentsToEstimate[ radial_empirical_acceleration_component ].push_back( cosine_empirical );

            empiricalComponentsToEstimate[ along_track_empirical_acceleration_component ].push_back( constant_empirical );
            empiricalComponentsToEstimate[ along_track_empirical_acceleration_component ].push_back( sine_empirical );
            empiricalComponentsToEstimate[ along_track_empirical_acceleration_component ].push_back( cosine_empirical );

            empiricalComponentsToEstimate[ across_track_empirical_acceleration_component ].push_back( constant_empirical );
            empiricalComponentsToEstimate[ across_track_empirical_acceleration_component ].push_back( sine_empirical );
            empiricalComponentsToEstimate[ across_track_empirical_acceleration_component ].push_back( cosine_empirical );

            additionalParameterNames.push_back( std::make_shared<EmpiricalAccelerationEstimatableParameterSettings>(
                "MGS", "Mars", empiricalComponentsToEstimate ));
        }

        std::shared_ptr<EstimationOutput<> > estimationOutput =
            createBestFitToCurrentEphemeris( bodies, accelerationModelMap, bodiesToEstimate, centralBodies,
                                             numerical_integrators::rungeKuttaFixedStepSettings( 120.0,
                                                                                                 numerical_integrators::rungeKuttaFehlberg78 ),
                                             initialTime, finalTime, 600.0, additionalParameterNames );
    }


}
//
//BOOST_AUTO_TEST_SUITE_END( )
//
//}
//
//}
