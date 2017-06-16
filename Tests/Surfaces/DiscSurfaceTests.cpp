// This file is part of the ACTS project.
//
// Copyright (C) 2016 ACTS project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define BOOST_TEST_MODULE Disc Surface Tests

#include <boost/test/included/unit_test.hpp>
// leave blank line

#include <boost/test/data/test_case.hpp>
// leave blank line

#include <boost/test/output_test_stream.hpp>
// leave blank line

//
#include "ACTS/Material/HomogeneousSurfaceMaterial.hpp"
#include "ACTS/Surfaces/DiscSurface.hpp"
#include "ACTS/Utilities/Definitions.hpp"
//
#include "DetectorElementStub.hpp"
//
#include <limits>

namespace utf    = boost::unit_test;
namespace tt     = boost::test_tools;
const double NaN = std::numeric_limits<double>::quiet_NaN();

namespace Acts {

namespace Test {
  // using boost::test_tools::output_test_stream;

  BOOST_AUTO_TEST_SUITE(Surfaces);
  /// Unit tests for creating DiscSurface object
  BOOST_AUTO_TEST_CASE(DiscSurface_constructors_test)
  {
    // default constructor is deleted
    // scaffolding...
    double rMin(1.0), rMax(5.0), halfPhiSector(M_PI / 8.);
    //
    /// Test DiscSurface fully specified constructor but no transform
    BOOST_CHECK_NO_THROW(DiscSurface(nullptr, rMin, rMax, halfPhiSector));
    //
    /// Test DiscSurface constructor with default halfPhiSector
    BOOST_CHECK_NO_THROW(DiscSurface(nullptr, rMin, rMax));
    //
    /// Test DiscSurface constructor with a transform specified
    Translation3D translation{0., 1., 2.};
    auto          pTransform = std::make_shared<const Transform3D>(translation);
    BOOST_CHECK_NO_THROW(DiscSurface(pTransform, rMin, rMax, halfPhiSector));
    //
    /// Copy constructed DiscSurface
    DiscSurface anotherDiscSurface(pTransform, rMin, rMax, halfPhiSector);
    // N.B. Just using BOOST_CHECK_NO_THROW(DiscSurface(anotherDiscSurface))
    // tries to call
    // the (deleted) default constructor.
    DiscSurface copiedDiscSurface(anotherDiscSurface);
    BOOST_TEST_MESSAGE("Copy constructed DiscSurface ok");
    //
    /// Copied and transformed DiscSurface
    BOOST_CHECK_NO_THROW(DiscSurface(anotherDiscSurface, *pTransform));
  }

  /// Unit tests of all named methods
  BOOST_AUTO_TEST_CASE(DiscSurface_properties_test, *utf::expected_failures(2))
  {
    Vector3D                     origin3D{0, 0, 0};
    std::shared_ptr<const Transform3D> pTransform;  // nullptr
    double                       rMin(1.0), rMax(5.0), halfPhiSector(M_PI / 8.);
    DiscSurface discSurfaceObject(pTransform, rMin, rMax, halfPhiSector);
    //
    /// Test type
    BOOST_CHECK(discSurfaceObject.type() == Surface::Disc);
    //
    /// Test normal, no local position specified
    Vector3D zAxis{0, 0, 1};
    BOOST_CHECK(discSurfaceObject.normal() == zAxis);
    //
    /// Test normal, local position specified
    Vector2D lpos(2.0, 0.05);
    BOOST_CHECK(discSurfaceObject.normal(lpos) == zAxis);
    //
    /// Test binningPosition
    // auto binningPosition=
    // discSurfaceObject.binningPosition(BinningValue::binRPhi );
    // std::cout<<binningPosition<<std::endl;
    BOOST_CHECK(discSurfaceObject.binningPosition(BinningValue::binRPhi)
                == origin3D);
    //
    /// Test bounds
    BOOST_TEST(discSurfaceObject.bounds().type() = SurfaceBounds::Disc);
    //
    /// Test isOnSurface()
    Vector3D point3DNotInSector{0.0, 1.2, 0};
    Vector3D point3DOnSurface{1.2, 0.0, 0};
    BOOST_TEST(discSurfaceObject.isOnSurface(point3DNotInSector, true)
               == false);  // passes
    BOOST_TEST(discSurfaceObject.isOnSurface(point3DOnSurface, true)
               == true);  // passes
    //
    /// Test localToGlobal
    Vector3D returnedPosition{NaN, NaN, NaN};
    Vector3D expectedPosition{1.2, 0, 0};
    Vector2D rPhiOnDisc{1.2, 0.0};
    Vector2D rPhiNotInSector{1.2, M_PI};  // outside sector at Phi=0, +/- pi/8
    Vector3D ignoredMomentum{NaN, NaN, NaN};
    discSurfaceObject.localToGlobal(
        rPhiOnDisc, ignoredMomentum, returnedPosition);
    BOOST_TEST(returnedPosition.isApprox(expectedPosition),
               "LocalToGlobal for rPhiOnDisc");
    //
    discSurfaceObject.localToGlobal(
        rPhiNotInSector, ignoredMomentum, returnedPosition);
    Vector3D expectedNonPosition{-1.2, 0, 0};
    BOOST_TEST(returnedPosition.isApprox(expectedNonPosition));
    //
    /// Test globalToLocal
    Vector2D returnedLocalPosition{NaN, NaN};
    Vector2D expectedLocalPosition{1.2, 0.0};
    BOOST_TEST(discSurfaceObject.globalToLocal(
        point3DOnSurface, ignoredMomentum, returnedLocalPosition));  // pass
    BOOST_TEST(returnedLocalPosition.isApprox(expectedLocalPosition));
    //
    BOOST_TEST(discSurfaceObject.globalToLocal(
                   point3DNotInSector, ignoredMomentum, returnedLocalPosition)
               == false);  // test fails
    //
    Vector3D pointOutsideRadius{0.0, 100., 0};
    BOOST_TEST(discSurfaceObject.globalToLocal(
                   pointOutsideRadius, ignoredMomentum, returnedLocalPosition)
               == false);  // fails
    //
    /// Test localPolarToCartesian
    Vector2D rPhi1_1{std::sqrt(2.), M_PI / 4.};
    Vector2D cartesian1_1{1., 1.};
    BOOST_TEST(discSurfaceObject.localPolarToCartesian(rPhi1_1).isApprox(
        cartesian1_1));
    //
    /// Test localCartesianToPolar
    BOOST_TEST(discSurfaceObject.localCartesianToPolar(cartesian1_1)
                   .isApprox(rPhi1_1));
    //
    /// Test localPolarToLocalCartesian
    BOOST_TEST(discSurfaceObject.localPolarToLocalCartesian(rPhi1_1).isApprox(
        cartesian1_1));
    //
    /// Test localCartesianToGlobal
    Vector3D cartesian3D1_1{1., 1., 0.};
    BOOST_TEST(discSurfaceObject.localCartesianToGlobal(cartesian1_1)
                   .isApprox(cartesian3D1_1));
    //
    /// Test globalToLocalCartesian
    BOOST_TEST(discSurfaceObject.globalToLocalCartesian(cartesian3D1_1)
                   .isApprox(cartesian1_1));
    //
    /// Test pathCorrection
    double   projected3DMomentum = std::sqrt(3.) * 1.e6;
    Vector3D momentum{
        projected3DMomentum, projected3DMomentum, projected3DMomentum};
    Vector3D ignoredPosition{NaN, NaN, NaN};
    BOOST_TEST(discSurfaceObject.pathCorrection(ignoredPosition, momentum)
                   == std::sqrt(3),
               tt::tolerance(0.01));
    //
    /// intersectionEstimate
    Vector3D globalPosition{1.2, 0.0, -10.};
    Vector3D direction{0., 0., 1.};  // must be normalised
    Vector3D expected{1.2, 0.0, 0.0};
    // intersect is a struct of (Vector3D) position, pathLength, distance and
    // (bool) valid
    auto intersect
        = discSurfaceObject.intersectionEstimate(globalPosition, direction);
    Intersection expectedIntersect{Vector3D{1.2, 0., 0.}, 10., true, 0.0};
    BOOST_TEST(intersect.valid);
    BOOST_TEST(intersect.position.isApprox(expectedIntersect.position));
    BOOST_TEST(intersect.pathLength == expectedIntersect.pathLength);
    BOOST_TEST(intersect.distance == expectedIntersect.distance);
    //
    /// Test name
    boost::test_tools::output_test_stream nameOuput;
    nameOuput << discSurfaceObject.name();
    BOOST_TEST(nameOuput.is_equal("Acts::DiscSurface"));
  }
  //
  /// Unit test for testing DiscSurface assignment and equality
  BOOST_AUTO_TEST_CASE(DiscSurface_assignment_test)
  {
    Vector3D                     origin3D{0, 0, 0};
    std::shared_ptr<const Transform3D> pTransform;  // nullptr
    double                       rMin(1.0), rMax(5.0), halfPhiSector(M_PI / 8.);
    DiscSurface discSurfaceObject(pTransform, rMin, rMax, halfPhiSector);
    DiscSurface assignedDisc(nullptr, NaN, NaN, NaN);
    //
    BOOST_CHECK_NO_THROW(assignedDisc = discSurfaceObject);
    BOOST_CHECK(assignedDisc == discSurfaceObject);
  }
  BOOST_AUTO_TEST_SUITE_END();

}  // end of namespace Test

}  // end of namespace Acts
