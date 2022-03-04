#include "tudat/astro/mission_segments/createTransferTrajectory.h"

namespace tudat
{
namespace mission_segments
{

std::shared_ptr< TransferLegSettings > dsmVelocityBasedLeg( )
{
    return std::make_shared< TransferLegSettings >( dsm_velocity_based_leg );
}


std::shared_ptr< TransferLegSettings > dsmPositionBasedLeg( )
{
    return std::make_shared< TransferLegSettings >( dsm_position_based_leg );
}


std::shared_ptr< TransferLegSettings > unpoweredLeg( )
{
    return std::make_shared< TransferLegSettings >( unpowered_unperturbed_leg );
}

std::shared_ptr< TransferNodeSettings > escapeAndDepartureNode(
        const double departureSemiMajorAxis,
        const double departureEccentricity )
{
    return std::make_shared< EscapeAndDepartureNodeSettings >( departureSemiMajorAxis, departureEccentricity );
}

std::shared_ptr< TransferNodeSettings > swingbyNode(
        const double minimumPeriapsisDistance )
{
    return std::make_shared< SwingbyNodeSettings >( minimumPeriapsisDistance );
}

std::shared_ptr< TransferNodeSettings > captureAndInsertionNode(
        const double captureSemiMajorAxis,
        const double captureEccentricity )
{
    return std::make_shared< CaptureAndInsertionNodeSettings >(
                captureSemiMajorAxis, captureEccentricity );
}

std::shared_ptr< TransferLeg > createTransferLeg (
        const simulation_setup::SystemOfBodies& bodyMap,
        const std::shared_ptr< TransferLegSettings > legSettings,
        const std::string& departureBodyName,
        const std::string& arrivalBodyName,
        const std::string& centralBodyName,
        const std::shared_ptr< TransferNode > departureNode,
        const std::shared_ptr< TransferNode > arrivalNode)
{
    if( bodyMap.count( centralBodyName ) == 0 )
    {
        throw std::runtime_error( "Error when making transfer leg, central body " + centralBodyName  + " not found." );
    }
    else if( bodyMap.at( centralBodyName )->getGravityFieldModel( ) == nullptr )
    {
        throw std::runtime_error( "Error when making transfer leg, central body " + centralBodyName  + " has no gravity field." );
    }

    if( bodyMap.count( departureBodyName ) == 0 )
    {
        throw std::runtime_error( "Error when making transfer leg, departure body " + departureBodyName  + " not found." );
    }
    else if( bodyMap.at( departureBodyName )->getEphemeris( ) == nullptr )
    {
        throw std::runtime_error( "Error when making transfer leg, departure body " + departureBodyName  + " has no ephemeris." );
    }

    if( bodyMap.count( arrivalBodyName ) == 0 )
    {
        throw std::runtime_error( "Error when making transfer leg, arrival body " + arrivalBodyName  + " not found." );
    }
    else if( bodyMap.at( arrivalBodyName )->getEphemeris( ) == nullptr )
    {
        throw std::runtime_error( "Error when making transfer leg, arrival body " + arrivalBodyName  + " has no ephemeris." );
    }


    const double centralBodyGravitationalParameter =
            bodyMap.at( centralBodyName )->getGravityFieldModel( )->getGravitationalParameter( );
    const std::shared_ptr< ephemerides::Ephemeris > departureBodyEphemeris =
            bodyMap.at( departureBodyName )->getEphemeris( );
    const std::shared_ptr< ephemerides::Ephemeris > arrivalBodyEphemeris =
            bodyMap.at( arrivalBodyName )->getEphemeris( );

    std::shared_ptr< TransferLeg >  transferLeg;
    switch( legSettings->legType_ )
    {
    case unpowered_unperturbed_leg:
    {
        transferLeg = std::make_shared< UnpoweredUnperturbedTransferLeg >(
                    departureBodyEphemeris, arrivalBodyEphemeris,
                    centralBodyGravitationalParameter );
        break;
    }
    case dsm_position_based_leg:
    {
        transferLeg = std::make_shared< DsmPositionBasedTransferLeg >(
                    departureBodyEphemeris, arrivalBodyEphemeris,
                    centralBodyGravitationalParameter );
        break;
    }
    case dsm_velocity_based_leg:
    {
        if( departureNode == nullptr )
        {
            throw std::runtime_error( "Error when making dsm_velocity_based_leg, no previous departure node" );
        }
        std::function< Eigen::Vector3d( ) > departureVelocityFunction =
                std::bind( &TransferNode::getOutgoingVelocity, departureNode );

        transferLeg = std::make_shared< DsmVelocityBasedTransferLeg >(
                    departureBodyEphemeris, arrivalBodyEphemeris,
                    centralBodyGravitationalParameter, departureVelocityFunction );
        break;
    }
    default:
        throw std::runtime_error( "Error when making transfer leg, leg type not recognized" );
    }
    return transferLeg;
}

std::shared_ptr<TransferNode> createTransferNode(
        const simulation_setup::SystemOfBodies& bodyMap,
        const std::shared_ptr< TransferNodeSettings > nodeSettings,
        const std::string &nodeBodyName,
        const std::shared_ptr< TransferLeg > incomingTransferLeg,
        const std::shared_ptr< TransferLeg > outgoingTransferLeg,
        const bool nodeComputesIncomingVelocity,
        const bool nodeComputesOutgoingVelocity)
{

    if( bodyMap.count( nodeBodyName ) == 0 )
    {
        throw std::runtime_error( "Error when making transfer node, body " + nodeBodyName  + " not found." );
    }
    else if( bodyMap.at( nodeBodyName )->getGravityFieldModel( ) == nullptr )
    {
        throw std::runtime_error( "Error when making transfer node, body " + nodeBodyName  + " has no gravity field." );
    }
    else if( bodyMap.at( nodeBodyName )->getEphemeris( ) == nullptr )
    {
        throw std::runtime_error( "Error when making transfer node, body " + nodeBodyName  + " has no ephemeris." );
    }

    const double centralBodyGravitationalParameter =
            bodyMap.at( nodeBodyName )->getGravityFieldModel( )->getGravitationalParameter( );
    const std::shared_ptr< ephemerides::Ephemeris > centralBodyEphemeris =
            bodyMap.at( nodeBodyName )->getEphemeris( );

    std::shared_ptr< TransferNode > transferNode;
    switch( nodeSettings->nodeType_ )
    {
    case swingby:
    {
        if( !nodeComputesIncomingVelocity && !nodeComputesOutgoingVelocity )
        {
            std::shared_ptr< SwingbyNodeSettings > swingbySettings =
                    std::dynamic_pointer_cast< SwingbyNodeSettings >( nodeSettings );
            if( swingbySettings == nullptr )
            {
                throw std::runtime_error( "Error when making swingby node, type is inconsistent" );
            }

            if( swingbySettings->minimumPeriapsisRadius_ != swingbySettings->minimumPeriapsisRadius_ )
            {
                throw std::runtime_error("Error when making swingby node, no minimum periapsis radius is provided" );
            }

            std::function< Eigen::Vector3d( ) > incomingVelocityFunction =
                    std::bind( &TransferLeg::getArrivalVelocity, incomingTransferLeg );
            std::function< Eigen::Vector3d( ) > outgoingVelocityFunction =
                    std::bind( &TransferLeg::getDepartureVelocity, outgoingTransferLeg );
            transferNode = std::make_shared< SwingbyWithFixedIncomingFixedOutgoingVelocity >(
                        centralBodyEphemeris,
                        centralBodyGravitationalParameter, swingbySettings->minimumPeriapsisRadius_,
                        incomingVelocityFunction, outgoingVelocityFunction );
        }
        else if ( !nodeComputesIncomingVelocity && nodeComputesOutgoingVelocity )
        {
            std::function< Eigen::Vector3d( ) > incomingVelocityFunction =
                    std::bind( &TransferLeg::getArrivalVelocity, incomingTransferLeg );
            transferNode = std::make_shared< SwingbyWithFixedIncomingFreeOutgoingVelocity >(
                        centralBodyEphemeris,
                        centralBodyGravitationalParameter, incomingVelocityFunction );
        }
        else
        {
            throw std::runtime_error("Invalid swingby node." );
        }


        break;
    }
    case escape_and_departure:
    {
        if( !nodeComputesOutgoingVelocity )
        {
            std::shared_ptr< EscapeAndDepartureNodeSettings > escapeAndDepartureSettings =
                    std::dynamic_pointer_cast< EscapeAndDepartureNodeSettings >( nodeSettings );
            if( escapeAndDepartureSettings == nullptr )
            {
                throw std::runtime_error( "Error when making escape_and_departure node, type is inconsistent" );
            }


            std::function< Eigen::Vector3d( ) > outgoingVelocityFunction =
                    std::bind( &TransferLeg::getDepartureVelocity, outgoingTransferLeg );
            transferNode = std::make_shared< DepartureWithFixedOutgoingVelocityNode >(
                        centralBodyEphemeris,
                        centralBodyGravitationalParameter,
                        escapeAndDepartureSettings->departureSemiMajorAxis_,
                        escapeAndDepartureSettings->departureEccentricity_,
                        outgoingVelocityFunction );
        }
        else
        {
            std::shared_ptr< EscapeAndDepartureNodeSettings > escapeAndDepartureSettings =
                    std::dynamic_pointer_cast< EscapeAndDepartureNodeSettings >( nodeSettings );
            if( escapeAndDepartureSettings == nullptr )
            {
                throw std::runtime_error( "Error when making escape_and_departure node, type is inconsistent" );
            }


            transferNode = std::make_shared< DepartureWithFreeOutgoingVelocityNode >(
                        centralBodyEphemeris,
                        centralBodyGravitationalParameter,
                        escapeAndDepartureSettings->departureSemiMajorAxis_,
                        escapeAndDepartureSettings->departureEccentricity_ );
        }

        break;
    }
    case capture_and_insertion:
    {
        if ( !nodeComputesIncomingVelocity )
        {
            std::shared_ptr< CaptureAndInsertionNodeSettings > captureAndInsertionSettings =
                    std::dynamic_pointer_cast< CaptureAndInsertionNodeSettings >( nodeSettings );
            if( captureAndInsertionSettings == nullptr )
            {
                throw std::runtime_error( "Error when making capture_and_insertion node, type is inconsistent" );
            }

            std::function< Eigen::Vector3d( ) > incomingVelocityFunction =
                    std::bind( &TransferLeg::getArrivalVelocity, incomingTransferLeg );
            transferNode = std::make_shared< CaptureWithFixedIncomingVelocityNode >(
                    centralBodyEphemeris,
                    centralBodyGravitationalParameter,
                    captureAndInsertionSettings->captureSemiMajorAxis_,
                    captureAndInsertionSettings->captureEccentricity_,
                    incomingVelocityFunction );
        }
        else
        {
            throw std::runtime_error("Invalid capture node." );
        }
        break;
    }
    default:
        throw std::runtime_error( "Error when making transfer node, node type not recognized" );
    }
    return transferNode;
}

std::pair< std::vector< std::shared_ptr< TransferLegSettings > >,
std::vector< std::shared_ptr< TransferNodeSettings > > > getMgaTransferTrajectorySettings(
        const std::vector< std::string >& fullBodiesList,
        const TransferLegTypes identicalTransferLegType,
        const std::pair< double, double > departureOrbit,
        const std::pair< double, double > arrivalOrbit,
        const std::map< std::string, double > minimumPericenterRadii )
{
    std::vector< std::shared_ptr< TransferLegSettings > > transferLegSettings;
    std::vector< std::shared_ptr< TransferNodeSettings > > transferNodeSettings;
    getMgaTransferTrajectorySettings(
                transferLegSettings, transferNodeSettings, fullBodiesList, identicalTransferLegType,
                departureOrbit, arrivalOrbit, minimumPericenterRadii );
    return std::make_pair( transferLegSettings, transferNodeSettings );
}

void getMgaTransferTrajectorySettings(
        std::vector< std::shared_ptr< TransferLegSettings > >& transferLegSettings,
        std::vector< std::shared_ptr< TransferNodeSettings > >& transferNodeSettings,
        const std::vector< std::string >& fullBodiesList,
        const TransferLegTypes identicalTransferLegType,
        const std::pair< double, double > departureOrbit,
        const std::pair< double, double > arrivalOrbit,
        const std::map< std::string, double > minimumPericenterRadii )
{
    int numberOfNodes = fullBodiesList.size( );

    transferLegSettings.clear( );
    for( int i = 0; i < numberOfNodes - 1; i++ )
    {
        transferLegSettings.push_back( std::make_shared< TransferLegSettings >( identicalTransferLegType  ) );
    }

    double currentMinimumPericenterRadius = TUDAT_NAN;
    transferNodeSettings.clear( );
    if( departureOrbit.first != departureOrbit.first )
    {
        if( minimumPericenterRadii.count( fullBodiesList.at( 0 ) ) == 0 )
        {
            throw std::runtime_error( "Error when making MGA settings, no pericenter radius provided for " + fullBodiesList.at( 0 ) );
        }
        currentMinimumPericenterRadius = minimumPericenterRadii.at( fullBodiesList.at( 0 ) );
        transferNodeSettings.push_back( swingbyNode( currentMinimumPericenterRadius ) );
    }
    else
    {
        transferNodeSettings.push_back( escapeAndDepartureNode( departureOrbit.first, departureOrbit.second ) );
    }

    for( int i = 1; i < numberOfNodes - 1; i++ )
    {
        if( minimumPericenterRadii.count( fullBodiesList.at( i ) ) == 0 )
        {
            throw std::runtime_error( "Error when making MGA settings, no pericenter radius provided for " + fullBodiesList.at( i ) );
        }
        currentMinimumPericenterRadius = minimumPericenterRadii.at( fullBodiesList.at( i ) );
        transferNodeSettings.push_back( swingbyNode( currentMinimumPericenterRadius ) );
    }

    if( arrivalOrbit.first != arrivalOrbit.first )
    {
        if( minimumPericenterRadii.count( fullBodiesList.at( numberOfNodes - 1 ) ) == 0 )
        {
            throw std::runtime_error( "Error when making MGA settings, no pericenter radius provided for " + fullBodiesList.at( numberOfNodes - 1  ) );
        }
        currentMinimumPericenterRadius = minimumPericenterRadii.at( fullBodiesList.at( numberOfNodes - 1  ) );
        transferNodeSettings.push_back( swingbyNode( currentMinimumPericenterRadius ) );
    }
    else
    {
        transferNodeSettings.push_back( captureAndInsertionNode( arrivalOrbit.first, arrivalOrbit.second ) );
    }


}

void getMgaTransferTrajectorySettingsWithoutDsm(
        std::vector< std::shared_ptr< TransferLegSettings > >& legSettings,
        std::vector< std::shared_ptr< TransferNodeSettings > >& nodeSettings,
        const std::string& departureBody, const std::string& arrivalBody,
        const std::vector< std::string >& flybyBodies,
        const std::pair< double, double > departureOrbit,
        const std::pair< double, double > arrivalOrbit,
        const std::map< std::string, double > minimumPericenterRadii )
{
    std::vector< std::string > fullBodiesList = { departureBody };
    fullBodiesList.insert(
                fullBodiesList.end( ), flybyBodies.begin( ), flybyBodies.end( ) );
    fullBodiesList.push_back( arrivalBody );
    return getMgaTransferTrajectorySettingsWithoutDsm(
                legSettings, nodeSettings, fullBodiesList, departureOrbit, arrivalOrbit, minimumPericenterRadii );

}

void getMgaTransferTrajectorySettingsWithoutDsm(
        std::vector< std::shared_ptr< TransferLegSettings > >& legSettings,
        std::vector< std::shared_ptr< TransferNodeSettings > >& nodeSettings,
        const std::vector< std::string >& fullBodiesList,
        const std::pair< double, double > departureOrbit,
        const std::pair< double, double > arrivalOrbit,
        const std::map< std::string, double > minimumPericenterRadii )
{
    return getMgaTransferTrajectorySettings(
                legSettings, nodeSettings, fullBodiesList, unpowered_unperturbed_leg,
                departureOrbit, arrivalOrbit, minimumPericenterRadii );
}

void getMgaTransferTrajectorySettingsWithPositionBasedDsm(
        std::vector< std::shared_ptr< TransferLegSettings > >& legSettings,
        std::vector< std::shared_ptr< TransferNodeSettings > >& nodeSettings,
        const std::string& departureBody, const std::string& arrivalBody,
        const std::vector< std::string >& flybyBodies,
        const std::pair< double, double > departureOrbit,
        const std::pair< double, double > arrivalOrbit,
        const std::map< std::string, double > minimumPericenterRadii )
{
    std::vector< std::string > fullBodiesList = { departureBody };
    fullBodiesList.insert(
                fullBodiesList.end( ), flybyBodies.begin( ), flybyBodies.end( ) );
    fullBodiesList.push_back( arrivalBody );
    return getMgaTransferTrajectorySettingsWithPositionBasedDsm(
                legSettings, nodeSettings, fullBodiesList, departureOrbit, arrivalOrbit, minimumPericenterRadii );

}

void getMgaTransferTrajectorySettingsWithPositionBasedDsm(
        std::vector< std::shared_ptr< TransferLegSettings > >& legSettings,
        std::vector< std::shared_ptr< TransferNodeSettings > >& nodeSettings,
        const std::vector< std::string >& fullBodiesList,
        const std::pair< double, double > departureOrbit,
        const std::pair< double, double > arrivalOrbit,
        const std::map< std::string, double > minimumPericenterRadii )
{
    return getMgaTransferTrajectorySettings(
                legSettings, nodeSettings, fullBodiesList, dsm_position_based_leg,
                departureOrbit, arrivalOrbit, minimumPericenterRadii );
}

void getMgaTransferTrajectorySettingsWithVelocityBasedDsm(
        std::vector< std::shared_ptr< TransferLegSettings > >& legSettings,
        std::vector< std::shared_ptr< TransferNodeSettings > >& nodeSettings,
        const std::string& departureBody, const std::string& arrivalBody,
        const std::vector< std::string >& flybyBodies,
        const std::pair< double, double > departureOrbit,
        const std::pair< double, double > arrivalOrbit,
        const std::map< std::string, double > minimumPericenterRadii )
{
    std::vector< std::string > fullBodiesList = { departureBody };
    fullBodiesList.insert(
                fullBodiesList.end( ), flybyBodies.begin( ), flybyBodies.end( ) );
    fullBodiesList.push_back( arrivalBody );
    return getMgaTransferTrajectorySettingsWithVelocityBasedDsm(
                legSettings, nodeSettings, fullBodiesList, departureOrbit, arrivalOrbit, minimumPericenterRadii );

}

void getMgaTransferTrajectorySettingsWithVelocityBasedDsm(
        std::vector< std::shared_ptr< TransferLegSettings > >& legSettings,
        std::vector< std::shared_ptr< TransferNodeSettings > >& nodeSettings,
        const std::vector< std::string >& fullBodiesList,
        const std::pair< double, double > departureOrbit,
        const std::pair< double, double > arrivalOrbit,
        const std::map< std::string, double > minimumPericenterRadii )
{
    return getMgaTransferTrajectorySettings(
                legSettings, nodeSettings, fullBodiesList, dsm_velocity_based_leg,
                departureOrbit, arrivalOrbit, minimumPericenterRadii );
}



std::shared_ptr< TransferTrajectory > createTransferTrajectory(
        const simulation_setup::SystemOfBodies& bodyMap,
        const std::vector< std::shared_ptr< TransferLegSettings > >& legSettings,
        const std::vector< std::shared_ptr< TransferNodeSettings > >& nodeSettings,
        const std::vector< std::string >& nodeIds,
        const std::string& centralBody )
{
    if( legSettings.size( ) + 1 != nodeSettings.size( ) )
    {
        throw std::runtime_error( "Error when making transfer trajectory, number of legs ( "
                                  + std::to_string( legSettings.size( ) ) +
                                  " ) and number of nodes ( "
                                  + std::to_string( nodeSettings.size( ) ) +
                                  " ) are incompatible" );
    }

    if( nodeIds.size( ) != nodeSettings.size( ) )
    {
        throw std::runtime_error( "Error when making transfer trajectory, number of nodes ( "
                                  + std::to_string( nodeSettings.size( ) ) +
                                  " ) and number of node names ( "
                                  + std::to_string( nodeIds.size( ) ) +
                                  " ) are incompatible" );
    }


    std::vector< std::shared_ptr< TransferLeg > > legs ( legSettings.size( ), nullptr );
    std::vector< std::shared_ptr< TransferNode > > nodes ( nodeSettings.size( ), nullptr );

    std::cout << "Legs to be created ";
    for( unsigned int i = 0; i < legSettings.size( ); i++ )
    {
        if (legs.at(i) == nullptr)
            std::cout << i << " ";
    }
    std::cout << std::endl << "Nodes to be created";
    for( unsigned int i = 0; i < nodeSettings.size( ); i++ )
    {
        if (nodes.at(i) == nullptr)
            std::cout << i << " ";
    }
    std::cout << std::endl;

    int iteration = 0;

    // Loop over nodes and legs until all are defined
    while ( ( std::find(legs.begin(), legs.end(), nullptr) != legs.end() ) || ( std::find(nodes.begin(), nodes.end(), nullptr) != nodes.end() ) )
    {
        std::cout << "Iteration " << iteration << std::endl;
        ++iteration;

        // First node
        if ( nodes.at(0) == nullptr )
        {
            // If node doesn't require input from the following leg, then create node
            if ( legRequiresInputFromPreviousNode.at( legSettings.at(0)->legType_ ) )
            {
                std::cout << "Creating node " << 0 << "a " << std::endl;
                nodes.at(0) = createTransferNode(
                        bodyMap, nodeSettings.at(0), nodeIds.at(0),
                        nullptr, nullptr, false, true);
            }
            // If node requires input from following leg but following leg is already created, then create node
            else if ( legs.at(0) != nullptr )
            {
                std::cout << "Creating node " << 0 << "b" << std::endl;
                nodes.at(0) = createTransferNode(
                        bodyMap, nodeSettings.at(0), nodeIds.at(0),
                        nullptr, legs.at(0), false, false);
            }
        }


        // Legs and intermediate nodes
        for( unsigned int i = 0; i < legSettings.size( ); i++ )
        {
            // Creation of leg
            // Don't create leg if it requires input from previous node which is not defined or if it requires input from
            // following node which is not defined
            if ( (legs.at(i) == nullptr) && !(legRequiresInputFromPreviousNode.at( legSettings.at(i)->legType_ ) && nodes.at(i) == nullptr) &&
                 !(legRequiresInputFromFollowingNode.at( legSettings.at(i)->legType_ ) && nodes.at(i+1) == nullptr))
            {
                std::cout << "Creating leg " << i << std::endl;
                legs.at(i) = createTransferLeg(
                        bodyMap, legSettings.at(i), nodeIds.at(i), nodeIds.at(i + 1),
                        centralBody, nodes.at(i), nodes.at(i+1) );
            }

            // Creation of node (all nodes except first and last)
            // Don't create node if it requires input from previous leg which is not defined or if it requires input from
            // following leg which is not defined
            if ( i < legSettings.size( ) - 1 )
            {
                bool nodeRequiresInputFromPreviousLeg = !legRequiresInputFromFollowingNode.at( legSettings.at(i)->legType_);
                bool nodeRequiresInputFromFollowingLeg = !legRequiresInputFromPreviousNode.at( legSettings.at(i+1)->legType_);
                if ( (nodes.at(i+1) == nullptr) && !(nodeRequiresInputFromPreviousLeg && legs.at(i) == nullptr ) &&
                     !(nodeRequiresInputFromFollowingLeg && legs.at(i+1) == nullptr ) )
                {
                    std::cout << "Creating node " << i+1 << ": "<< !nodeRequiresInputFromPreviousLeg << " " << !nodeRequiresInputFromFollowingLeg << std::endl;
                    nodes.at(i+1) = createTransferNode(
                            bodyMap, nodeSettings.at(i+1), nodeIds.at(i+1), legs.at(i),
                            legs.at(i+1), !nodeRequiresInputFromPreviousLeg, !nodeRequiresInputFromFollowingLeg);
                }
            }
        }

        //  Last node
        // nodeComputesOutgoingVelocity is set to true for swingby nodes; for capture nodes the value doesn't matter
        if ( nodes.at(legSettings.size( )) == nullptr)
        {
            // If node doesn't require input from the previous leg, then create node
            if ( legRequiresInputFromFollowingNode.at( legSettings.at(legSettings.size( ) - 1)->legType_ ) )
            {
                std::cout << "Creating node " << legSettings.size( ) << "a" << std::endl;
                nodes.at(legSettings.size( )) = createTransferNode(
                        bodyMap, nodeSettings.at(legSettings.size( ) ), nodeIds.at(legSettings.size( )),
                        nullptr, nullptr, true, true);
            }
            // If node requires input from previous leg but previous leg is already created, then create node
            else if ( legs.at(legSettings.size( ) - 1) != nullptr )
            {
                std::cout << "Creating node " << legSettings.size( ) << "b" << std::endl;
                nodes.at(legSettings.size( )) = createTransferNode(
                        bodyMap, nodeSettings.at(legSettings.size( )), nodeIds.at(legSettings.size( )),
                        legs.at(legSettings.size( ) - 1), nullptr, false, true);
            }
        }


        std::cout << "Legs ";
        for( unsigned int i = 0; i < legSettings.size( ); i++ )
        {
            if (legs.at(i) == nullptr)
                std::cout << i << " ";
        }
        std::cout << std::endl << "Nodes ";
        for( unsigned int i = 0; i < nodeSettings.size( ); i++ )
        {
            if (nodes.at(i) == nullptr)
                std::cout << i << " ";
        }
        std::cout << std::endl;


        std::cout << (std::find(legs.begin(), legs.end(), nullptr) != legs.end()) << " " <<
            (std::find(nodes.begin(), nodes.end(), nullptr) != nodes.end()) << std::endl << std::endl;

        if (iteration > 5)
            throw std::runtime_error( "Something wrong!" );
    }

    std::cout << "Created Legs ";
    for( unsigned int i = 0; i < legSettings.size( ); i++ )
        std::cout << legSettings.at(i)->legType_ << " ";
    std::cout << std::endl << "Created Nodes ";
    for( unsigned int i = 0; i < nodeSettings.size( ); i++ )
        std::cout << nodeSettings.at(i)->nodeType_ << " ";
    std::cout << std::endl;

    ///////////////////////////////////////////////////////////////////////////////////
    /*
    std::vector< std::shared_ptr< TransferLeg > > legs;
    std::vector< std::shared_ptr< TransferNode > > nodes;


    for( unsigned int i = 0; i < legSettings.size( ); i++ )
    {
        if( legRequiresInputFromPreviousNode.at(legSettings.at(i)->legType_ ) )
        {
            nodes.push_back(
                    createTransferNode(
                            bodyMap, nodeSettings.at(i), nodeIds.at(i), (i == 0 ? nullptr : legs.at(i - 1)),
                            nullptr, false, true));
            legs.push_back(
                    createTransferLeg(
                            bodyMap, legSettings.at(i),
                            nodeIds.at(i), nodeIds.at(i + 1), centralBody,
                            nodes.at(i), std::shared_ptr< TransferNode >( )));
        }
        else
        {
            legs.push_back(
                    createTransferLeg(
                            bodyMap, legSettings.at(i),
                            nodeIds.at(i), nodeIds.at(i + 1), centralBody,
                            std::shared_ptr< TransferNode >( ), std::shared_ptr< TransferNode >( )));


            nodes.push_back(
                    createTransferNode(
                            bodyMap, nodeSettings.at(i), nodeIds.at(i),
                            (i == 0 ? nullptr : legs.at(i - 1)), legs.at(i),
                            false, false));


        }
    }

    if( nodeSettings.at( legSettings.size( ) )->nodeType_ == capture_and_insertion )
    {
        nodes.push_back(
                createTransferNode(
                        bodyMap, nodeSettings.at(legSettings.size()),
                        nodeIds.at(legSettings.size()),
                        legs.at(legSettings.size() - 1), nullptr, false, false));
    }
    else
    {
        nodes.push_back(
                createTransferNode(
                        bodyMap, nodeSettings.at(legSettings.size()),
                        nodeIds.at(legSettings.size()),
                        legs.at(legSettings.size() - 1), nullptr, false, true));
    }
    */
    return std::make_shared< TransferTrajectory >( legs, nodes );

}


void getParameterVectorDecompositionIndices(
        const std::vector< std::shared_ptr< TransferLegSettings > >& legSettings,
        const std::vector< std::shared_ptr< TransferNodeSettings > >& nodeSettings,
        std::vector< std::pair< int, int > >& legParameterIndices,
        std::vector< std::pair< int, int > >& nodeParameterIndices )
{
    // First N parameters are times/TOFs
    int currentParameterIndex = nodeSettings.size( );

    for( unsigned int i = 0; i < nodeSettings.size( ); i++ )
    {
        switch( nodeSettings.at( i )->nodeType_  )
        {
        case swingby:
            if( legRequiresInputFromPreviousNode.at(legSettings.at(i )->legType_ ) )
            {
                nodeParameterIndices.push_back( std::make_pair( currentParameterIndex, 3 ) );
                currentParameterIndex += 3;
            }
            else
            {
                nodeParameterIndices.push_back( std::make_pair( currentParameterIndex, 0 ) );
            }
            break;
        case escape_and_departure:
            if( legRequiresInputFromPreviousNode.at(legSettings.at(i )->legType_ ) )
            {
                nodeParameterIndices.push_back( std::make_pair( currentParameterIndex, 3 ) );
                currentParameterIndex += 3;
            }
            else
            {
                nodeParameterIndices.push_back( std::make_pair( currentParameterIndex, 0 ) );
            }
            break;
        case capture_and_insertion:
            nodeParameterIndices.push_back( std::make_pair( currentParameterIndex, 0 ) );
            break;
        }

        if( i != legSettings.size( ) )
        {
            switch( legSettings.at( i )->legType_  )
            {
            case unpowered_unperturbed_leg:
                legParameterIndices.push_back( std::make_pair( currentParameterIndex, 0 ) );
                break;
            case dsm_position_based_leg:
                legParameterIndices.push_back( std::make_pair( currentParameterIndex, 4 ) );
                currentParameterIndex += 4;
                break;
            case dsm_velocity_based_leg:
                legParameterIndices.push_back( std::make_pair( currentParameterIndex, 1 ) );
                currentParameterIndex += 1;
                break;
            }
        }
    }
}

void printTransferParameterDefinition(
        const std::vector< std::shared_ptr< TransferLegSettings > >& legSettings,
        const std::vector< std::shared_ptr< TransferNodeSettings > >& nodeSettings )
{
    std::vector< std::vector< std::string > > legParameterDefinitions;
    std::vector< std::vector< std::string > > nodeParameterDefinitions;

    for( unsigned int i = 0; i < legSettings.size( ); i++ )
    {
        std::vector< std::string  > currentLegIds;
        switch( legSettings.at( i )->legType_  )
        {
        case unpowered_unperturbed_leg:
            break;
        case dsm_position_based_leg:
            currentLegIds.push_back( "DSM (position-based) Time-of-flight fraction" );
            currentLegIds.push_back( "DSM (position-based) Dimensionless radius" );
            currentLegIds.push_back( "DSM (position-based) In-plane angle" );
            currentLegIds.push_back( "DSM (position-based) Out-of-plane angle" );
            break;
        case dsm_velocity_based_leg:
            currentLegIds.push_back( "DSM (velocity-based) Time-of-flight fraction" );
            break;
        }
        legParameterDefinitions.push_back( currentLegIds );
    }

    for( unsigned int i = 0; i < nodeSettings.size( ); i++ )
    {
        std::vector< std::string  > currentNodeIds;
        switch( nodeSettings.at( i )->nodeType_  )
        {
        case swingby:
        {
            bool useSwingbyParameters = false;
            if( i == legSettings.size( ) )
            {
                useSwingbyParameters = true;
            }
            else if( legRequiresInputFromPreviousNode.at(legSettings.at(i )->legType_ ) )
            {
                useSwingbyParameters = true;
            }
            if( useSwingbyParameters )
            {
                currentNodeIds.push_back( "Swingby periapsis" );
                currentNodeIds.push_back( "Swingby orbit-orientation rotation" );
                currentNodeIds.push_back( "Swingby Delta V" );
            }
            break;
        }
        case escape_and_departure:
            if( legRequiresInputFromPreviousNode.at(legSettings.at(i )->legType_ ) )
            {
                currentNodeIds.push_back( "Excess velocity magnitude" );
                currentNodeIds.push_back( "Excess velocity in-plane angle" );
                currentNodeIds.push_back( "Excess velocity out-of-plane angle" );
            }
            break;
        case capture_and_insertion:
            break;
        }
        nodeParameterDefinitions.push_back( currentNodeIds );
    }

    int parameterIndex = 0;
    for( unsigned int i = 0; i < nodeSettings.size( ); i++ )
    {
        std::cout << "Parameter "<<parameterIndex<<": Node time "<<i<<std::endl;
        parameterIndex++;
    }

    for( unsigned int i = 0; i < nodeParameterDefinitions.size( ); i++ )
    {
        for( unsigned j = 0; j < nodeParameterDefinitions.at( i ).size( ); j++ )
        {
            std::cout << "Parameter "<<parameterIndex<<": Node "<<i<<" "<< nodeParameterDefinitions.at( i ).at( j )<<std::endl;
            parameterIndex++;
        }
    }


    for( unsigned int i = 0; i < legParameterDefinitions.size( ); i++ )
    {

        for( unsigned j = 0; j < legParameterDefinitions.at( i ).size( ); j++ )
        {
            std::cout << "Parameter "<<parameterIndex<<": Leg  "<<i<<" "<< legParameterDefinitions.at( i ).at( j )<<std::endl;
            parameterIndex++;
        }

    }
    std::cout<<std::endl;
}


} // namespace mission_segments

} // namespace tudat
