/*! \file compositeSurfaceGeometry.cpp
 *    This file contains the definition of the CompositeSurfaceGeometry class
 *
 *    Path              : /Mathematics/GeometricShapes/
 *    Version           : 5
 *    Check status      : Checked
 *
 *    Author            : D. Dirkx
 *    Affiliation       : Delft University of Technology
 *    E-mail address    : d.dirkx@tudelft.nl
 *
 *    Checker           : K. Kumar
 *    Affiliation       : Delft University of Technology
 *    E-mail address    : K.Kumar@tudelft.nl
 *
 *    Date created      : 25 November, 2010
 *    Last modified     : 5 September, 2011
 *
 *    References
 *
 *    Notes
 *      The code uses pointers to pointers to represent an array of pointers,
 *      instead of the vector or map from the STL for heritage reasons. In
 *      future, it might be fruitful to consider the use of standard STL
 *      containers instead.
 *
 *    Copyright (c) 2010-2011 Delft University of Technology.
 *
 *    This software is protected by national and international copyright.
 *    Any unauthorized use, reproduction or modification is unlawful and
 *    will be prosecuted. Commercial and non-private application of the
 *    software in any form is strictly prohibited unless otherwise granted
 *    by the authors.
 *
 *    The code is provided without any warranty; without even the implied
 *    warranty of merchantibility or fitness for a particular purpose.
 *
 *    Changelog
 *      YYMMDD    Author            Comment
 *      102511    D. Dirkx          First version of file.
 *      110119    K. Kumar          Minor comments changes; path updated;
 *                                  Doxygen comments updated; updated function
 *                                  arguments to use references and unsigned
 *                                  ints; added "End of file" comment; minor
 *                                  changes to layout.
 *      110204    K. Kumar          Minor comment and layout modifications;
 *                                  corrected Doxygen comments.
 *      110810    J. Leloux         Corrected doxygen documentation.
 *      110905    S. Billemont      Reorganized includes.
 *                                  Moved (con/de)structors and getter/setters to header.
 */

// Include statements.
#include "Mathematics/GeometricShapes/compositeSurfaceGeometry.h"

//! Default destructor.
CompositeSurfaceGeometry::~CompositeSurfaceGeometry( )
{
    // Delete single surface geometry list.
    delete [ ] singleSurfaceGeometryList_;
    singleSurfaceGeometryList_ = NULL;

    // Delete composite surface geometry list.
    delete [ ] compositeSurfaceGeometryList_ ;
    compositeSurfaceGeometryList_ = NULL;
}

//! Overload ostream to print class information.
std::ostream &operator<<( std::ostream &stream,
                          CompositeSurfaceGeometry& compositeSurfaceGeometry )
{
    stream << "This is a composite surface geometry." << std::endl;
    stream << "The number of SingleSurfaceGeometries is: "
           << compositeSurfaceGeometry.numberOfSingleSurfaceGeometries_
           << std::endl;
    stream << "The number of CompositeSurfaceGeometries is: "
           << compositeSurfaceGeometry.numberOfCompositeSurfaceGeometries_
           << std::endl;

    // Return stream.
    return stream;
}

// End of file.
