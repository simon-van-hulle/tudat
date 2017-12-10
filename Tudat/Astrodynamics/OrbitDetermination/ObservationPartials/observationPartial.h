/*    Copyright (c) 2010-2017, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#ifndef TUDAT_OBSERVATIONPARTIAL_H
#define TUDAT_OBSERVATIONPARTIAL_H

#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/assign/list_of.hpp>

#include <Eigen/Core>

#include "Tudat/Basics/basicTypedefs.h"

#include "Tudat/Astrodynamics/ObservationModels/observableTypes.h"
#include "Tudat/Astrodynamics/ObservationModels/linkTypeDefs.h"
#include "Tudat/Astrodynamics/OrbitDetermination/EstimatableParameters/estimatableParameter.h"
#include "Tudat/Astrodynamics/OrbitDetermination/EstimatableParameters/observationBiasParameter.h"
#include "Tudat/Mathematics/BasicMathematics/mathematicalConstants.h"

namespace tudat
{

namespace observation_partials
{

//! Base class for scaling position partials to observable partials.
/*!
 *  Base class for scaling position partials to observable partials. For observables computed from the three-dimensional
 *  positions of reference points, the partial derivatives are computed from the partials of these positions. Each
 *  observation model requires a specific derived class of PositionPartialScaling, that computes how position partials
 *  are converted to ('scaled to') observable partials
 */
class PositionPartialScaling
{
public:

    //! Destructor
    virtual ~PositionPartialScaling( ){ }

    //! Update the scaling object to the current times and states (pure virtual).
    /*!
     *  Update the scaling object to the current times and states (pure virtual).
     *  \param linkEndStates List of states at each link end during observation.
     *  \param times List of times at each link end during observation.
     *  \param fixedLinkEnd Link end at which observation time is defined, i.e. link end for which associated time
     *  is kept constant when computing observable.
     *  \param currentObservation Value of observation for which partial scaling is to be computed
     */
    virtual void update( const std::vector< Eigen::Vector6d >& linkEndStates,
                         const std::vector< double >& times,
                         const observation_models::LinkEndType fixedLinkEnd,
                         const Eigen::VectorXd currentObservation ) = 0;
};

//! Base class of partial derivative of an observable w.r.t. an estimated parameter.
/*!
 *  Base class of partial derivative of an observable w.r.t. an estimated parameter.
 *  Calculated partial function must be defined in each derived class for calculation of specific derivative.
 */
template< int ObservationSize >
class ObservationPartial
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param parameterIdentifier Parameter id of for specifc parameter of which partial is computed by object.
     */
    ObservationPartial( const estimatable_parameters::EstimatebleParameterIdentifier parameterIdentifier ):
        parameterIdentifier_( parameterIdentifier ){ }

    //! Virtual destructor.
    /*!
     *  Virtual destrcutor.
     */
    virtual ~ObservationPartial( ) { }

    //! Pure virtual function to calculate the observation partial(s) at required time(s) and state(s)
    /*!
     *  Pure virtual function to calculate the observation partial(s) at required time(s) and state(s). States and times
     *  are typically obtained from evaluation of associated observation model.
     *  Derived class functions implement this for a specific observable.
     *  \param states Link end states. Index maps to link end for a given ObsevableType through getLinkEndIndex function.
     *  \param times Link end times.
     *  \param linkEndOfFixedTime Link end that is kept fixed when computing the observable.
     *  \param currentObservation Value of the observation for which the partial is to be computed (default NaN for
     *  compatibility purposes).
     *  \return Vector of pairs containing partial values and associated times.
     */
    virtual std::vector< std::pair< Eigen::Matrix< double, ObservationSize, Eigen::Dynamic >, double > > calculatePartial(
            const std::vector< Eigen::Vector6d >& states,
            const std::vector< double >& times,
            const observation_models::LinkEndType linkEndOfFixedTime = observation_models::receiver,
            const Eigen::Matrix< double, ObservationSize, 1 >& currentObservation =
            Eigen::Matrix< double, ObservationSize, 1 >::Constant( TUDAT_NAN ) ) = 0;

    //! Function to get parameter id of for specifc parameter of which partial is computed by object.
    /*!
     * Function to get parameter id of for specifc parameter of which partial is computed by object.
     * \return Parameter id of for specifc parameter of which partial is computed by object.
     */
    estimatable_parameters::EstimatebleParameterIdentifier getParameterIdentifier( )
    {
        return parameterIdentifier_;
    }


protected:

    //! Parameter id of for specifc parameter of which partial is computed by object.
    estimatable_parameters::EstimatebleParameterIdentifier parameterIdentifier_;


};

//! Class for computing the derivative of any observable w.r.t. a constant absolute observation bias
/*!
 *  Class for computing the derivative of any observable w.r.t. a constant absolute observation bias. Note that this partial is
 *  distinct from most other ObservationPartial partial derived classes, as its implementation is based on the parameter
 *  (constant observation bias), not the type of observable: the implementation is identical for each observable.
 */
template< int ObservationSize >
class ObservationPartialWrtConstantAbsoluteBias: public ObservationPartial< ObservationSize >
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param observableType Observable type for which the bias is active.
     * \param linkEnds Observation link ends for which the bias is active.
     */
    ObservationPartialWrtConstantAbsoluteBias( const observation_models::ObservableType observableType,
                                               const observation_models::LinkEnds& linkEnds ):
        ObservationPartial< ObservationSize >(
            std::make_pair( estimatable_parameters::constant_additive_observation_bias, linkEnds.begin( )->second ) ),
        observableType_( observableType ), linkEnds_( linkEnds )
    {
        // Compute partial (vector of ObservationSize with 1.0 entries).
        constantPartial_ = Eigen::Matrix< double, ObservationSize, 1 >::Constant( 1.0 );
    }

    //! Destructor
    ~ObservationPartialWrtConstantAbsoluteBias( ){ }

    //! Function to calculate the observation partial w.r.t. constant absolute bias
    /*!
     *  Function to calculate the observation partial w.r.t. constant absolute bias. Note that output is independent of input.
     *  Associated time defined at times[ 0 ].
     *  \param states Link end states (unused).
     *  \param times Link end times  (unused).
     *  \param linkEndOfFixedTime Link end that is kept fixed when computing the observable  (unused).
     *  \param currentObservation Value of the observation for which the partial is to be computed  (unused).
     *  \return Vector of pairs containing partial values and associated times.
     */
    std::vector< std::pair< Eigen::Matrix< double, ObservationSize, Eigen::Dynamic >, double > > calculatePartial(
            const std::vector< Eigen::Vector6d >& states,
            const std::vector< double >& times,
            const observation_models::LinkEndType linkEndOfFixedTime = observation_models::receiver,
            const Eigen::Matrix< double, ObservationSize, 1 >& currentObservation =
            Eigen::Matrix< double, ObservationSize, 1 >::Zero( ) )
    {
        return { std::make_pair( constantPartial_, times.at( 0 ) ) };
    }

private:

    //! Observable type for which the bias is active.
    observation_models::ObservableType observableType_;

    //!  Observation link ends for which the bias is active.
    observation_models::LinkEnds linkEnds_;

    //! Observation partial: constant for all conditions.
    Eigen::Matrix< double, ObservationSize, 1 > constantPartial_;

};

template< int ObservationSize >
class ObservationPartialWrtArcWiseAbsoluteBias: public ObservationPartial< ObservationSize >
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param observableType Observable type for which the bias is active.
     * \param linkEnds Observation link ends for which the bias is active.
     */
    ObservationPartialWrtArcWiseAbsoluteBias( const observation_models::ObservableType observableType,
                                               const observation_models::LinkEnds& linkEnds,
                                               const boost::shared_ptr< interpolators::LookUpScheme< double > > arcLookupScheme,
                                               const int linkEndIndex,
                                               const int numberOfArcs ):
        ObservationPartial< ObservationSize >(
            std::make_pair( estimatable_parameters::arcwise_constant_additive_observation_bias, linkEnds.begin( )->second ) ),
        observableType_( observableType ), linkEnds_( linkEnds ), arcLookupScheme_( arcLookupScheme ),
        linkEndIndex_( linkEndIndex ), numberOfArcs_( numberOfArcs )
    {
        // Compute partial (vector of ObservationSize with 1.0 entries).
        constantPartial_ = Eigen::Matrix< double, ObservationSize, 1 >::Constant( 1.0 );

        totalPartial_ = Eigen::VectorXd::Zero( ObservationSize * numberOfArcs_ );
    }

    //! Destructor
    ~ObservationPartialWrtArcWiseAbsoluteBias( ){ }

    //! Function to calculate the observation partial w.r.t. constant absolute bias
    /*!
     *  Function to calculate the observation partial w.r.t. constant absolute bias. Note that output is independent of input.
     *  Associated time defined at times[ 0 ].
     *  \param states Link end states (unused).
     *  \param times Link end times  (unused).
     *  \param linkEndOfFixedTime Link end that is kept fixed when computing the observable  (unused).
     *  \param currentObservation Value of the observation for which the partial is to be computed  (unused).
     *  \return Vector of pairs containing partial values and associated times.
     */
    std::vector< std::pair< Eigen::Matrix< double, ObservationSize, Eigen::Dynamic >, double > > calculatePartial(
            const std::vector< Eigen::Vector6d >& states,
            const std::vector< double >& times,
            const observation_models::LinkEndType linkEndOfFixedTime = observation_models::receiver,
            const Eigen::Matrix< double, ObservationSize, 1 >& currentObservation =
            Eigen::Matrix< double, ObservationSize, 1 >::Zero( ) )
    {
        int currentIndex = arcLookupScheme_->findNearestLowerNeighbour( times.at( linkEndIndex_ ) );

        totalPartial_.setZero( );
        totalPartial_.segment( currentIndex * ObservationSize, ObservationSize ) = constantPartial_;

        return { std::make_pair( totalPartial_, times.at( linkEndIndex_ ) ) };
    }

private:

    //! Observable type for which the bias is active.
    observation_models::ObservableType observableType_;

    //!  Observation link ends for which the bias is active.
    observation_models::LinkEnds linkEnds_;

    boost::shared_ptr< interpolators::LookUpScheme< double > > arcLookupScheme_;

    int linkEndIndex_;

    int numberOfArcs_;

    //! Observation partial for singe arc: constant for all conditions.
    Eigen::Matrix< double, ObservationSize, 1 > constantPartial_;

    Eigen::VectorXd totalPartial_;

};

//! Class for computing the derivative of any observable w.r.t. a constant relative observation bias
/*!
 *  Class for computing the derivative of any observable w.r.t. a constant relative observation bias. Note that this partial is
 *  distinct from most other ObservationPartial partial derived classes, as its implementation is based on the parameter
 *  (constant observation bias), not the type of observable: the implementation is identical for each observable.
 */
template< int ObservationSize >
class ObservationPartialWrtConstantRelativeBias: public ObservationPartial< ObservationSize >
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param observableType Observable type for which the bias is active.
     * \param linkEnds Observation link ends for which the bias is active.
     */
    ObservationPartialWrtConstantRelativeBias( const observation_models::ObservableType observableType,
                                               const observation_models::LinkEnds& linkEnds ):
        ObservationPartial< ObservationSize >(
            std::make_pair( estimatable_parameters::constant_additive_observation_bias, linkEnds.begin( )->second ) ),
        observableType_( observableType ), linkEnds_( linkEnds )
    {  }

    //! Destructor
    ~ObservationPartialWrtConstantRelativeBias( ){ }

    //! Function to calculate the observation partial w.r.t. constant relative bias
    /*!
     *  Function to calculate the observation partial w.r.t. constant bias. Note that output is independent of input times and
     *  states. Associated time defined at times[ 0 ].
     *  \param states Link end states (unused).
     *  \param times Link end times  (unused).
     *  \param linkEndOfFixedTime Link end that is kept fixed when computing the observable  (unused).
     *  \param currentObservation Value of the observation for which the partial is to be computed.
     *  \return Vector of pairs containing partial values and associated times.
     */
    std::vector< std::pair< Eigen::Matrix< double, ObservationSize, Eigen::Dynamic >, double > > calculatePartial(
            const std::vector< Eigen::Vector6d >& states,
            const std::vector< double >& times,
            const observation_models::LinkEndType linkEndOfFixedTime = observation_models::receiver,
            const Eigen::Matrix< double, ObservationSize, 1 >& currentObservation =
            Eigen::Matrix< double, ObservationSize, 1 >::Constant( TUDAT_NAN ) )
    {
        return { std::make_pair( currentObservation, times.at( 0 ) ) };
    }

private:

    //! Observable type for which the bias is active.
    observation_models::ObservableType observableType_;

    //!  Observation link ends for which the bias is active.
    observation_models::LinkEnds linkEnds_;
};

template< int ObservationSize >
class ObservationPartialWrtArcWiseRelativeBias: public ObservationPartial< ObservationSize >
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param observableType Observable type for which the bias is active.
     * \param linkEnds Observation link ends for which the bias is active.
     */
    ObservationPartialWrtArcWiseRelativeBias( const observation_models::ObservableType observableType,
                                               const observation_models::LinkEnds& linkEnds,
                                               const boost::shared_ptr< interpolators::LookUpScheme< double > > arcLookupScheme,
                                               const int linkEndIndex,
                                               const int numberOfArcs ):
        ObservationPartial< ObservationSize >(
            std::make_pair( estimatable_parameters::arcwise_constant_relative_observation_bias, linkEnds.begin( )->second ) ),
        observableType_( observableType ), linkEnds_( linkEnds ), arcLookupScheme_( arcLookupScheme ),
        linkEndIndex_( linkEndIndex ), numberOfArcs_( numberOfArcs )
    {
       totalPartial_ = Eigen::VectorXd::Zero( ObservationSize * numberOfArcs_ );
    }

    //! Destructor
    ~ObservationPartialWrtArcWiseRelativeBias( ){ }

    //! Function to calculate the observation partial w.r.t. constant absolute bias
    /*!
     *  Function to calculate the observation partial w.r.t. constant absolute bias. Note that output is independent of input.
     *  Associated time defined at times[ 0 ].
     *  \param states Link end states (unused).
     *  \param times Link end times  (unused).
     *  \param linkEndOfFixedTime Link end that is kept fixed when computing the observable  (unused).
     *  \param currentObservation Value of the observation for which the partial is to be computed  (unused).
     *  \return Vector of pairs containing partial values and associated times.
     */
    std::vector< std::pair< Eigen::Matrix< double, ObservationSize, Eigen::Dynamic >, double > > calculatePartial(
            const std::vector< Eigen::Vector6d >& states,
            const std::vector< double >& times,
            const observation_models::LinkEndType linkEndOfFixedTime = observation_models::receiver,
            const Eigen::Matrix< double, ObservationSize, 1 >& currentObservation =
            Eigen::Matrix< double, ObservationSize, 1 >::Zero( ) )
    {
        int currentIndex = arcLookupScheme_->findNearestLowerNeighbour( times.at( linkEndIndex_ ) );

        totalPartial_.setZero( );
        totalPartial_.segment( currentIndex * ObservationSize, ObservationSize ) = currentObservation;

        return { std::make_pair( totalPartial_, times.at( linkEndIndex_ ) ) };
    }

private:

    //! Observable type for which the bias is active.
    observation_models::ObservableType observableType_;

    //!  Observation link ends for which the bias is active.
    observation_models::LinkEnds linkEnds_;

    boost::shared_ptr< interpolators::LookUpScheme< double > > arcLookupScheme_;

    int linkEndIndex_;

    int numberOfArcs_;

    Eigen::VectorXd totalPartial_;

};

//! Typedef for map of observation partials.
/*!
 *  Typedef for map of observation partials, for an observable of size 1.
 *  The key of the map is a pair denoting the start index of the parameter
 *  in the list of estimated parameters and the number of indices of the parameter
 *  wrt which the current partial (corresponding value in map) is taken.
 */
typedef std::map< std::pair< int, int >, boost::shared_ptr< ObservationPartial< 1 > > > SingleLinkObservationPartialList;

//! Typedef for map of observation partials.
/*!
 *  Typedef for map of observation partials, for an observable of size 2.
 *  The key of the map is a pair denoting the start index of the parameter
 *  in the list of estimated parameters and the number of indices of the parameter
 *  wrt which the current partial (corresponding value in map) is taken.
 */
typedef std::map< std::pair< int, int >, boost::shared_ptr< ObservationPartial< 2 > > > SingleLinkObservationTwoPartialList;

//! Typedef for map of observation partials.
/*!
 *  Typedef for map of observation partials, for an observable of size 3.
 *  The key of the map is a pair denoting the start index of the parameter
 *  in the list of estimated parameters and the number of indices of the parameter
 *  wrt which the current partial (corresponding value in map) is taken.
 */
typedef std::map< std::pair< int, int >, boost::shared_ptr< ObservationPartial< 3 > > > SingleLinkObservationThreePartialList;


//! Function to create partials of observation w.r.t. a link property.
/*!
 *  Function to create partials of observation w.r.t. a link property, e.g. a parameter that does not influence either link end's
 *  dynamics, only the observable itself, such as observation biases and clock parameters.
 *  \param linkEnds Link ends of observable for which partial is to be made.
 *  \param observableType Type of observable for which partial is to be made.
 *  \param parameterToEstimate Parameter w.r.t. which the partial is to be taken
 *  \param useBiasPartials Boolean to denote whether this function should create partials w.r.t. observation bias parameters
 *  \return Object that computes the partial of the observation w.r.t. parameterToEstimate (NULL if no dependency).
 */
template< int ObservationSize >
boost::shared_ptr< ObservationPartial< ObservationSize > > createObservationPartialWrtLinkProperty(
        const observation_models::LinkEnds& linkEnds,
        const observation_models::ObservableType observableType,
        const boost::shared_ptr< estimatable_parameters::EstimatableParameter< Eigen::VectorXd > > parameterToEstimate,
        const bool useBiasPartials = true )
{
    boost::shared_ptr< ObservationPartial< ObservationSize > > observationPartial;

    // Check parameter type
    switch( parameterToEstimate->getParameterName( ).first )
    {
    case estimatable_parameters::constant_additive_observation_bias:
    {
        if( useBiasPartials )
        {
            // Check input consistency
            boost::shared_ptr< estimatable_parameters::ConstantObservationBiasParameter > constantBias =
                    boost::dynamic_pointer_cast< estimatable_parameters::ConstantObservationBiasParameter >(
                        parameterToEstimate );
            if( constantBias == NULL )
            {
                throw std::runtime_error( "Error when making partial w.r.t. observation bias, type is inconsistent" );
            }
            else
            {
                // Check dependency between parameter and link properties.
                if( linkEnds == constantBias->getLinkEnds( ) && observableType == constantBias->getObservableType( ) )
                {
                    observationPartial = boost::make_shared< ObservationPartialWrtConstantAbsoluteBias< ObservationSize > >(
                                observableType, linkEnds );
                }
            }
        }
        break;
    }
    case estimatable_parameters::arcwise_constant_additive_observation_bias:
    {
        if( useBiasPartials )
        {
            // Check input consistency
            boost::shared_ptr< estimatable_parameters::ArcWiseObservationBiasParameter > arcwiseBias =
                    boost::dynamic_pointer_cast< estimatable_parameters::ArcWiseObservationBiasParameter >(
                        parameterToEstimate );
            if( arcwiseBias == NULL )
            {
                throw std::runtime_error( "Error when making partial w.r.t. arcwise observation bias, type is inconsistent" );
            }
            else
            {
                // Check dependency between parameter and link properties.
                if( linkEnds == arcwiseBias->getLinkEnds( ) && observableType == arcwiseBias->getObservableType( ) )
                {
                    observationPartial = boost::make_shared< ObservationPartialWrtArcWiseAbsoluteBias< ObservationSize > >(
                                observableType, linkEnds,
                                arcwiseBias->getLookupScheme( ),
                                arcwiseBias->getLinkEndIndex( ),
                                arcwiseBias->getArcStartTimes( ).size( ) );
                }
            }
        }
        break;
    }
    case estimatable_parameters::constant_relative_observation_bias:
    {
        if( useBiasPartials )
        {
            // Check input consistency
            boost::shared_ptr< estimatable_parameters::ConstantRelativeObservationBiasParameter > constantBias =
                    boost::dynamic_pointer_cast< estimatable_parameters::ConstantRelativeObservationBiasParameter >(
                        parameterToEstimate );
            if( constantBias == NULL )
            {
                throw std::runtime_error( "Error when making partial w.r.t. observation bias, type is inconsistent" );
            }
            else
            {
                // Check dependency between parameter and link properties.
                if( linkEnds == constantBias->getLinkEnds( ) && observableType == constantBias->getObservableType( ) )
                {
                    observationPartial = boost::make_shared< ObservationPartialWrtConstantRelativeBias< ObservationSize > >(
                                observableType, linkEnds );

                }
            }
            break;
        }
    }
    case estimatable_parameters::arcwise_constant_relative_observation_bias:
    {
        if( useBiasPartials )
        {
            // Check input consistency
            boost::shared_ptr< estimatable_parameters::ArcWiseRelativeObservationBiasParameter > arcwiseBias =
                    boost::dynamic_pointer_cast< estimatable_parameters::ArcWiseRelativeObservationBiasParameter >(
                        parameterToEstimate );
            if( arcwiseBias == NULL )
            {
                throw std::runtime_error( "Error when making partial w.r.t. arcwise relative observation bias, type is inconsistent" );
            }
            else
            {
                // Check dependency between parameter and link properties.
                if( linkEnds == arcwiseBias->getLinkEnds( ) && observableType == arcwiseBias->getObservableType( ) )
                {
                    observationPartial = boost::make_shared< ObservationPartialWrtArcWiseRelativeBias< ObservationSize > >(
                                observableType, linkEnds,
                                arcwiseBias->getLookupScheme( ),
                                arcwiseBias->getLinkEndIndex( ),
                                arcwiseBias->getArcStartTimes( ).size( ) );
                }
            }
        }
        break;
    }
    default:
        break;
    }

    return observationPartial;
}

}

}

#endif // OBSERVATIONPARTIAL_H
