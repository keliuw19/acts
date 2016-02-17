/**
 * @file ParameterSetTests.cxx
 */

// STL include(s)
#include <cmath>
#include <memory>
#include <random>

// Boost include(s)
#define BOOST_TEST_MODULE ParameterSet Tests
#include <boost/test/included/unit_test.hpp>
#include <boost/variant.hpp>

// ATS include(s)
#include "ParameterSet/ParameterSet.h"
#include "Algebra/AlgebraDefinitions.h"

// Test include(s)
#include "DummyParameterDefs.h"

/**
 * @brief Ats namespace
 */
namespace Ats
{
  /**
   * @brief Namespace for ATS unit tests
   */
  namespace Test
  {
    /// @cond
    namespace
    {
      // tolerance used for floating point comparison in this translation unit
      const double tol = 1e-6;

      double get_cyclic_value(double value,double min,double max)
      {
        return value - (max - min) * std::floor((value - min)/(max - min));
      }

      double get_cyclic_difference(double a,double b,double min,double max)
      {
        const double period = max - min;
        const double half_period = period/2;
        a = get_cyclic_value(a,min,max);
        b = get_cyclic_value(b,min,max);
        double raw_diff = a - b;
        double diff = (raw_diff > half_period) ? raw_diff - period : ((raw_diff < -half_period) ? period + raw_diff : raw_diff);
        return diff;
      }

      void check_residuals_for_bound_parameters()
      {
        const double max = parameter_traits<ParPolicy,ParDefs::theta>::pMax();
        const double min = parameter_traits<ParPolicy,ParDefs::theta>::pMin();
        double theta_1 = 0.7*M_PI;
        double theta_2 = 0.4*M_PI;
        AtsVectorD<1> dTheta;
        dTheta << (theta_1 - theta_2);

        // both parameters inside bounds, difference is positive
        ParameterSet<ParPolicy,ParDefs::theta> bound1(nullptr,theta_1);
        ParameterSet<ParPolicy,ParDefs::theta> bound2(nullptr,theta_2);
        BOOST_CHECK(bound1.residual(bound2).isApprox(dTheta,tol));

        // both parameters inside bound, difference negative
        dTheta << (theta_2 - theta_1);
        BOOST_CHECK(bound2.residual(bound1).isApprox(dTheta,tol));

        // one parameter above upper bound, difference positive
        theta_1 = max + 1;
        bound1.setParameter<ParDefs::theta>(theta_1);
        dTheta << max - theta_2;
        BOOST_CHECK(bound1.residual(bound2).isApprox(dTheta,tol));

        // one parameter above upper bound, difference negative
        dTheta << theta_2 - max;
        BOOST_CHECK(bound2.residual(bound1).isApprox(dTheta,tol));

        // one parameter below lower bound, difference positive
        theta_1 = min - 1;
        bound1.setParameter<ParDefs::theta>(theta_1);
        dTheta << theta_2 - min;
        BOOST_CHECK(bound2.residual(bound1).isApprox(dTheta,tol));

        // one parameter below lower bound, difference negative
        dTheta << min - theta_2;
        BOOST_CHECK(bound1.residual(bound2).isApprox(dTheta,tol));

        // both parameters outside bounds, both below
        theta_1 = min - 1;
        theta_2 = min - 2;
        bound1.setParameter<ParDefs::theta>(theta_1);
        bound2.setParameter<ParDefs::theta>(theta_2);
        dTheta << 0;
        BOOST_CHECK(bound1.residual(bound2).isApprox(dTheta,tol));

        // both parameters outside bounds, both above
        theta_1 = max + 1;
        theta_2 = max + 2;
        bound1.setParameter<ParDefs::theta>(theta_1);
        bound2.setParameter<ParDefs::theta>(theta_2);
        dTheta << 0;
        BOOST_CHECK(bound1.residual(bound2).isApprox(dTheta,tol));

        // both parameters outside bounds, one above, one below
        theta_1 = max + 1;
        theta_2 = min - 2;
        bound1.setParameter<ParDefs::theta>(theta_1);
        bound2.setParameter<ParDefs::theta>(theta_2);
        dTheta << max - min;
        BOOST_CHECK(bound1.residual(bound2).isApprox(dTheta,tol));
        dTheta << min - max;
        BOOST_CHECK(bound2.residual(bound1).isApprox(dTheta,tol));
      }

      void check_residuals_for_cyclic_parameters()
      {
        const double max = parameter_traits<ParPolicy,ParDefs::phi>::pMax();
        const double min = parameter_traits<ParPolicy,ParDefs::phi>::pMin();


        double phi_1 = 0.7*M_PI;
        double phi_2 = 0.4*M_PI;
        AtsVectorD<1> dPhi;
        dPhi << (phi_1 - phi_2);

        ParameterSet<ParPolicy,ParDefs::phi> cyclic1(nullptr,phi_1);
        ParameterSet<ParPolicy,ParDefs::phi> cyclic2(nullptr,phi_2);

        // no boundary crossing, difference is positive
        BOOST_CHECK(cyclic1.residual(cyclic2).isApprox(dPhi,tol));

        // no boundary crossing, difference is negative
        BOOST_CHECK(cyclic2.residual(cyclic1).isApprox(-dPhi,tol));

        // forward boundary crossing
        phi_1 = -0.9 * M_PI;
        cyclic1.setParameter<ParDefs::phi>(phi_1);
        dPhi << get_cyclic_difference(phi_1,phi_2,min,max);
        BOOST_CHECK(cyclic1.residual(cyclic2).isApprox(dPhi,tol));
        BOOST_CHECK(cyclic2.residual(cyclic1).isApprox(-dPhi,tol));

        // backward boundary crossing
        phi_1 = 0.7*M_PI;
        phi_2 = -0.9*M_PI;
        cyclic1.setParameter<ParDefs::phi>(phi_1);
        cyclic2.setParameter<ParDefs::phi>(phi_2);
        dPhi << get_cyclic_difference(phi_1,phi_2,min,max);
        BOOST_CHECK(cyclic1.residual(cyclic2).isApprox(dPhi,tol));
        BOOST_CHECK(cyclic2.residual(cyclic1).isApprox(-dPhi,tol));
      }

      void random_residual_tests()
      {
        // random number generators
        std::default_random_engine e;
        std::uniform_real_distribution<float> uniform_dist(-1000, 300);

        const double theta_max = parameter_traits<ParPolicy,ParDefs::theta>::pMax();
        const double theta_min = parameter_traits<ParPolicy,ParDefs::theta>::pMin();
        const double phi_max   = parameter_traits<ParPolicy,ParDefs::phi>::pMax();
        const double phi_min   = parameter_traits<ParPolicy,ParDefs::phi>::pMin();

        AtsVectorD<5> parValues_1;
        AtsVectorD<5> parValues_2;
        FullParameterSet<ParPolicy> parSet_1(nullptr,parValues_1);
        FullParameterSet<ParPolicy> parSet_2(nullptr,parValues_2);
        AtsVectorD<5> residual;
        const unsigned int toys = 1000;
        for(unsigned int i = 0; i < toys; ++i)
        {
          const double loc1_1  = uniform_dist(e);
          const double loc2_1  = uniform_dist(e);
          const double phi_1   = uniform_dist(e);
          const double theta_1 = uniform_dist(e);
          const double qop_1   = uniform_dist(e);
          parValues_1 << loc1_1, loc2_1, phi_1, theta_1, qop_1;
          parSet_1.setParameters(parValues_1);

          const double loc1_2  = uniform_dist(e);
          const double loc2_2  = uniform_dist(e);
          const double phi_2   = uniform_dist(e);
          const double theta_2 = uniform_dist(e);
          const double qop_2   = uniform_dist(e);
          parValues_2 << loc1_2, loc2_2, phi_2, theta_2, qop_2;
          parSet_2.setParameters(parValues_2);

          const double delta_loc1  = loc1_1 - loc1_2;
          const double delta_loc2  = loc2_1 - loc2_2;
          const double delta_phi   = phi_1 - phi_2;
          // for theta make sure that the difference calculation considers the restricted value range
          const double delta_theta = (theta_1 > theta_max ? theta_max : (theta_1 < theta_min ? theta_min : theta_1)) - (theta_2 > theta_max ? theta_max : (theta_2 < theta_min ? theta_min : theta_2));
          const double delta_qop   = qop_1 - qop_2;
          residual = parSet_1.residual(parSet_2);

          // local parameters are unbound -> check for usual difference
          if(fabs(residual(0) - delta_loc1) > tol) { BOOST_CHECK(false); break;}
          if(fabs(residual(1) - delta_loc2) > tol) { BOOST_CHECK(false); break;}
          // phi is a cyclic parameter -> check that (unsigned) difference is not larger than half period
          // check that corrected(corrected(phi_2) + residual) == corrected(phi_1)
          if(fabs(get_cyclic_value(get_cyclic_value(phi_2,phi_min,phi_max) + residual(2),phi_min,phi_max) - get_cyclic_value(phi_1,phi_min,phi_max)) > tol or fabs(residual(2)) > (phi_max - phi_min)/2) { BOOST_CHECK(false); break;}
          // theta is bound -> check that (unsigned) difference is not larger then allowed range, check corrected difference
          if(fabs(residual(3) - delta_theta) > tol or fabs(residual(3)) > (theta_max - theta_min)) { BOOST_CHECK(false); break;}
          // qop is unbound -> check usual difference
          if(fabs(residual(4) - delta_qop) > tol) { BOOST_CHECK(false); break;}
        }
      }
    }
    /// @endcond

    /**
     * @brief Unit test for Ats::anonymous_namespace{ParameterSet.h}::are_sorted helper
     *
     * The test checks for correct behavior in the following cases (always using @c int
     * as value type):
     * -# test: ordered strictly ascending, input: ordered strictly ascending
     * -# test: ordered strictly ascending, input: unordered
     * -# test: ordered strictly ascending, input: ordered weakly ascending
     * -# test: ordered weakly ascending, input: ordered strictly ascending
     * -# test: ordered weakly ascending, input: unordered
     * -# test: ordered weakly ascending, input: ordered weakly ascending
     * -# test: ordered strictly descending, input: ordered strictly descending
     * -# test: ordered strictly descending, input: unordered
     * -# test: ordered strictly descending, input: ordered weakly descending
     * -# test: ordered weakly descending, input: ordered strictly descending
     * -# test: ordered weakly descending, input: unordered
     * -# test: ordered weakly descending, input: ordered weakly descending
     */
    BOOST_AUTO_TEST_CASE(are_sorted_helper_tests)
    {
      // strictly ascending
      BOOST_CHECK((are_sorted<true,true,int,-1,3,4,12>::value));
      BOOST_CHECK((not are_sorted<true,true,int,-1,13,4>::value));
      BOOST_CHECK((not are_sorted<true,true,int,-1,4,4,7>::value));
      // weakly ascending
      BOOST_CHECK((are_sorted<true,false,int,-1,3,4,12>::value));
      BOOST_CHECK((not are_sorted<true,false,int,-1,13,4>::value));
      BOOST_CHECK((are_sorted<true,false,int,-1,4,4,7>::value));
      // strictly descending
      BOOST_CHECK((are_sorted<false,true,int,1,-3,-4,-12>::value));
      BOOST_CHECK((not are_sorted<false,true,int,1,-13,-4>::value));
      BOOST_CHECK((not are_sorted<false,true,int,1,-4,-4>::value));
      // weakly descending
      BOOST_CHECK((are_sorted<false,false,int,1,-3,-4,-12>::value));
      BOOST_CHECK((not are_sorted<false,false,int,-1,-13,-4>::value));
      BOOST_CHECK((are_sorted<false,false,int,-1,-4,-4,-7>::value));
    }

    /**
     * @brief Unit test for Ats::anonymous_namespace{ParameterSet.h}::are_within helper
     *
     * The test checks for correct behavior in the following cases (always using @c int
     * as value type):
     * -# all values within (MIN,MAX)
     * -# all values within [MIN,MAX)
     * -# one value < MIN
     * -# multiple values < MIN
     * -# one value > MAX
     * -# multiple values > Max
     * -# one value == MAX
     * -# contains values < MIN and >= MAX
     */
    BOOST_AUTO_TEST_CASE(are_within_helper_tests)
    {
      BOOST_CHECK((are_within<int,0,10,1,3,7,2>::value));
      BOOST_CHECK((are_within<int,0,10,1,3,0,2>::value));
      BOOST_CHECK((not are_within<int,0,10,-1,3,7,2>::value));
      BOOST_CHECK((not are_within<int,0,10,-1,3,7,-2>::value));
      BOOST_CHECK((not are_within<int,0,10,1,3,17,2>::value));
      BOOST_CHECK((not are_within<int,0,10,1,3,17,12>::value));
      BOOST_CHECK((not are_within<int,0,10,1,10>::value));
      BOOST_CHECK((not are_within<int,0,10,1,-2,10,14>::value));
    }

    /**
     * @brief Unit test for checking consistency of ParameterSet class
     *
     * The following functions are tested to yield the expected result/behavior:
     * -# ParameterSet::size
     * -# ParameterSet::contains
     * -# ParameterSet::getParameter
     * -# ParameterSet::getParameters
     * -# ParameterSet::getCovariance
     * -# ParameterSet::setCovariance
     * -# ParameterSet::setParameter
     */
    BOOST_AUTO_TEST_CASE(parset_consistency_tests)
    {
      // check template parameter based information
      BOOST_CHECK((ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2>::size() == 2));

      // covariance matrix
      AtsSymMatrixD<3> cov;
      cov<< 1,0,0,
          0,1.2,0.2,
          0,0.2,0.7;
      std::unique_ptr<AtsSymMatrixD<3> > pCovMatrix = std::make_unique<AtsSymMatrixD<3> >(cov);

      // parameter values
      double loc1 = 0.5;
      double loc2 = -0.2;
      double phi  = 0.3*M_PI;  // this should be within [-M_PI,M_PI) to avoid failed tests due to angle range corrections
      AtsVectorD<3> parValues(loc1,loc2,phi);

      // parameter set with covariance matrix
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> parSet_with_cov(std::move(pCovMatrix),loc1,loc2,phi);

      // check number and type of stored parameters
      BOOST_CHECK(parSet_with_cov.size() == 3);
      BOOST_CHECK(parSet_with_cov.contains<ParDefs::loc1>());
      BOOST_CHECK(parSet_with_cov.contains<ParDefs::loc2>());
      BOOST_CHECK(parSet_with_cov.contains<ParDefs::phi>());
      BOOST_CHECK(not parSet_with_cov.contains<ParDefs::theta>());
      BOOST_CHECK(not parSet_with_cov.contains<ParDefs::qop>());

      // check stored parameter values
      BOOST_CHECK(parSet_with_cov.getParameter<ParDefs::loc1>() == loc1);
      BOOST_CHECK(parSet_with_cov.getParameter<ParDefs::loc2>() == loc2);
      BOOST_CHECK(parSet_with_cov.getParameter<ParDefs::phi>() == phi);
      BOOST_CHECK(parSet_with_cov.getParameters() == parValues);

      // check stored covariance
      BOOST_CHECK(parSet_with_cov.getCovariance() != 0);
      BOOST_CHECK(*parSet_with_cov.getCovariance() == cov);

      // same parameter set without covariance matrix
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> parSet_without_cov(nullptr,parValues);

      BOOST_CHECK(parSet_without_cov.getCovariance() == 0);
      BOOST_CHECK(parSet_without_cov.getParameters() == parSet_with_cov.getParameters());

      // set new covariance matrix
      parSet_without_cov.setCovariance(std::make_unique<AtsSymMatrixD<3> >(cov));

      BOOST_CHECK(parSet_without_cov.getCovariance() != 0);
      BOOST_CHECK(*parSet_without_cov.getCovariance() == cov);

      // set new parameter values
      double newLoc1 = 0.1;
      double newLoc2 = 0.6;
      double newPhi  = -0.15*M_PI;
      parValues << newLoc1, newLoc2, newPhi;
      parSet_with_cov.setParameter<ParDefs::loc1>(newLoc1);
      parSet_with_cov.setParameter<ParDefs::loc2>(newLoc2);
      parSet_with_cov.setParameter<ParDefs::phi>(newPhi);

      BOOST_CHECK(parSet_with_cov.getParameter<ParDefs::loc1>() == newLoc1);
      BOOST_CHECK(parSet_with_cov.getParameter<ParDefs::loc2>() == newLoc2);
      BOOST_CHECK(parSet_with_cov.getParameter<ParDefs::phi>() == newPhi);
      BOOST_CHECK(parSet_with_cov.getParameters() == parValues);
    }

    /**
     * @brief Unit test for copy/assignment/swap in ParameterSet class
     *
     * The behavior of the following functions is checked:
     * -# ParameterSet::ParameterSet
     * -# ParameterSet::operator=
     * -# ParameterSet::swap
     */
    BOOST_AUTO_TEST_CASE(parset_copy_assignment_tests)
    {
      // covariance matrix
      AtsSymMatrixD<3> cov;
      cov<< 1,0,0,
          0,1.2,0.2,
          0,0.2,0.7;
      std::unique_ptr<AtsSymMatrixD<3> > pCovMatrix = std::make_unique<AtsSymMatrixD<3> >(cov);

      // parameter values
      double loc1 = 0.5;
      double loc2 = -0.2;
      double phi  = 0.3*M_PI;  // this should be within [-M_PI,M_PI) to avoid failed tests due to angle range corrections
      AtsVectorD<3> first_parValues(loc1,loc2,phi);

      // parameter set with covariance matrix
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> first(std::move(pCovMatrix),loc1,loc2,phi);

      // check copy constructor
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> copy(first);
      BOOST_CHECK(first == copy);

      // check move constructor
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> moved(std::move(copy));
      BOOST_CHECK(first == moved);

      // check assignment operator
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> assigned = moved;
      BOOST_CHECK(assigned == moved);

      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> other(nullptr,0,1.7,-0.15);
      BOOST_CHECK(assigned != other);
      assigned = other;
      BOOST_CHECK(assigned == other);

      // check for self-assignment
      assigned = assigned;
      BOOST_CHECK(assigned == other);

      // check move assignment
      BOOST_CHECK(first != assigned);
      first = ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi>(assigned);
      BOOST_CHECK(first == assigned);

      // check swap method
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> lhs(std::move(pCovMatrix),loc1,loc2,phi);
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> rhs(nullptr,2*loc1,2*loc2,2*phi);
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> lhs_copy = lhs;
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> rhs_copy = rhs;

      BOOST_CHECK(lhs != rhs && lhs == lhs_copy && rhs == rhs_copy);
      using std::swap;
      swap(lhs,rhs);
      BOOST_CHECK(lhs != rhs && rhs == lhs_copy && lhs == rhs_copy);
    }

    /**
     *  @brief Unit test for comparison operators in ParameterSet
     *
     *  @sa ParameterSet::operator==, ParameterSet::operator!=
     */
    BOOST_AUTO_TEST_CASE(parset_comparison_tests)
    {
      // covariance matrix
      AtsSymMatrixD<3> cov;
      cov<< 1,0,0,
          0,1.2,0.2,
          0,0.2,0.7;
      std::unique_ptr<AtsSymMatrixD<3> > pCovMatrix = std::make_unique<AtsSymMatrixD<3> >(cov);

      // parameter values
      double loc1 = 0.5;
      double loc2 = -0.2;
      double phi  = 0.3*M_PI;  // this should be within [-M_PI,M_PI) to avoid failed tests due to angle range corrections

      // parameter set with covariance matrix
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> first(std::move(pCovMatrix),loc1,loc2,phi);
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi> second(nullptr,2*loc1,2*loc2,2*phi);

      // check self comparison
      BOOST_CHECK(first == first);
      BOOST_CHECK(not (first != first));

      // check mutual exclusivity
      BOOST_CHECK(first != second);
      BOOST_CHECK(not (first == second));
      first = second;
      BOOST_CHECK(first == second);

      // check that comparison fails for unequal parameter values
      second.setParameter<ParDefs::loc1>(3*loc1);
      BOOST_CHECK(first != second);
      first = second;
      BOOST_CHECK(first == second);

      second.setParameter<ParDefs::loc2>(3*loc2);
      BOOST_CHECK(first != second);
      first = second;
      BOOST_CHECK(first == second);

      second.setParameter<ParDefs::phi>(3*phi);
      BOOST_CHECK(first != second);
      first = second;
      BOOST_CHECK(first == second);

      // check that comparison fails for unequal covariance matrices
      second.setCovariance(std::make_unique<AtsSymMatrixD<3> >(cov));
      BOOST_CHECK(first != second);
      first = second;
      BOOST_CHECK(first == second);

      cov(0,0) = 2 * cov(0,0);
      second.setCovariance(std::make_unique<AtsSymMatrixD<3> >(cov));
      BOOST_CHECK(first != second);
      first = second;
      BOOST_CHECK(first == second);
    }

    /**
     * @brief Unit test for projection matrices in ParameterSet
     *
     * Checks the correctness of the projection matrices from the full parameter space
     * onto different parameter sub-spaces
     *
     * @sa ParameterSet::projector
     */
    BOOST_AUTO_TEST_CASE(parset_projection_tests)
    {
      AtsMatrixD<1,5> phi_proj;
      phi_proj << 0,0,1,0,0;

      AtsMatrixD<2,5> loc1_qop_proj;
      loc1_qop_proj << 1,0,0,0,0,
                       0,0,0,0,1;

      AtsMatrixD<2,5> loc2_theta_proj;
      loc2_theta_proj << 0,1,0,0,0,
                         0,0,0,1,0;

      AtsMatrixD<3,5> loc1_loc2_phi_proj;
      loc1_loc2_phi_proj << 1,0,0,0,0,
                            0,1,0,0,0,
                            0,0,1,0,0;

      AtsMatrixD<4,5> loc1_phi_theta_qop_proj;
      loc1_phi_theta_qop_proj << 1,0,0,0,0,
                                 0,0,1,0,0,
                                 0,0,0,1,0,
                                 0,0,0,0,1;

      AtsMatrixD<5,5> loc1_loc2_phi_theta_qop_proj;
      loc1_loc2_phi_theta_qop_proj << 1,0,0,0,0,
                                      0,1,0,0,0,
                                      0,0,1,0,0,
                                      0,0,0,1,0,
                                      0,0,0,0,1;

      BOOST_CHECK((ParameterSet<ParPolicy,ParDefs::phi>::projector() == phi_proj));
      BOOST_CHECK((ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::qop>::projector() == loc1_qop_proj));
      BOOST_CHECK((ParameterSet<ParPolicy,ParDefs::loc2,ParDefs::theta>::projector() == loc2_theta_proj));
      BOOST_CHECK((ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi>::projector() == loc1_loc2_phi_proj));
      BOOST_CHECK((ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::phi,ParDefs::theta,ParDefs::qop>::projector() == loc1_phi_theta_qop_proj));
      BOOST_CHECK((ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::phi,ParDefs::theta,ParDefs::qop>::projector() == loc1_loc2_phi_theta_qop_proj));
    }

    /**
     * @brief Unit test for residuals between different ParameterSet objects
     *
     * The result of the residual calculation between two ParameterSet objects is checked.
     * A test of the automatic correction of stored parameter values for out-of-bounds/cyclic
     * corrections is also implemented.
     *
     * @sa ParameterSet::residual, ParameterSet::getParameter
     */
    BOOST_AUTO_TEST_CASE(parset_residual_tests)
    {
      // check unbound parameter type
      const double large_number = 12443534120;
      const double small_number = -924342675;
      const double normal_number = 1.234;
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::loc2,ParDefs::qop> unbound(nullptr,small_number,large_number,normal_number);
      BOOST_CHECK(unbound.getParameter<ParDefs::loc1>() == small_number);
      BOOST_CHECK(unbound.getParameter<ParDefs::loc2>() == large_number);
      BOOST_CHECK(unbound.getParameter<ParDefs::qop>() == normal_number);

      // check bound parameter type
      ParameterSet<ParPolicy,ParDefs::theta> bound(nullptr,small_number);
      BOOST_CHECK((bound.getParameter<ParDefs::theta>() == parameter_traits<ParPolicy,ParDefs::theta>::pMin()));
      bound.setParameter<ParDefs::theta>(large_number);
      BOOST_CHECK((bound.getParameter<ParDefs::theta>() == parameter_traits<ParPolicy,ParDefs::theta>::pMax()));
      bound.setParameter<ParDefs::theta>(normal_number);
      BOOST_CHECK((bound.getParameter<ParDefs::theta>() == normal_number));

      // check cyclic parameter type
      ParameterSet<ParPolicy,ParDefs::phi> cyclic(nullptr,small_number);
      // calculate expected results
      const double min = parameter_traits<ParPolicy,ParDefs::phi>::pMin();
      const double max = parameter_traits<ParPolicy,ParDefs::phi>::pMax();
      // check that difference between original phi and stored phi is a multiple of the cyclic period
      double multiple = (cyclic.getParameter<ParDefs::phi>() - small_number)/(max - min);
      BOOST_CHECK(cyclic.getParameter<ParDefs::phi>() >= min);
      BOOST_CHECK(cyclic.getParameter<ParDefs::phi>() < max);
      BOOST_CHECK(fabs(multiple - std::floor(multiple + 0.5)) < tol);

      cyclic.setParameter<ParDefs::phi>(large_number);
      multiple = (cyclic.getParameter<ParDefs::phi>() - large_number)/(max - min);
      BOOST_CHECK(cyclic.getParameter<ParDefs::phi>() >= min);
      BOOST_CHECK(cyclic.getParameter<ParDefs::phi>() < max);
      BOOST_CHECK(fabs(multiple - std::floor(multiple + 0.5)) < tol);

      cyclic.setParameter<ParDefs::phi>(normal_number);
      multiple = (cyclic.getParameter<ParDefs::phi>() - normal_number)/(max - min);
      BOOST_CHECK(cyclic.getParameter<ParDefs::phi>() >= min);
      BOOST_CHECK(cyclic.getParameter<ParDefs::phi>() < max);
      BOOST_CHECK(fabs(multiple - std::floor(multiple + 0.5)) < tol);

      // check residual calculation

      // input numbers
      const double first_loc1 = 0.3;
      const double first_phi = 0.9 * M_PI;
      const double first_theta = 0.7 * M_PI;

      const double second_loc1 = 2.7;
      const double second_phi = -0.9 * M_PI;
      const double second_theta = 0.35 * M_PI;

      const double full_loc1 = -0.45;
      const double full_loc2 = 12.3;
      const double full_phi = -0.3*M_PI;
      const double full_theta = 0.9 * M_PI;
      const double full_qop = 0.0012;

      // expected results for residual second wrt first
      const double delta_loc1 = second_loc1 - first_loc1;
      const double delta_phi = get_cyclic_difference(second_phi,first_phi,min,max);
      const double delta_theta = second_theta - first_theta;
      AtsVectorD<3> residuals(delta_loc1,delta_phi,delta_theta);

      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::phi,ParDefs::theta> first(nullptr,first_loc1,first_phi,first_theta);
      ParameterSet<ParPolicy,ParDefs::loc1,ParDefs::phi,ParDefs::theta> second(nullptr,second_loc1,second_phi,second_theta);
      BOOST_CHECK((residuals == second.residual(first)));

      // some more checks for bound variables
      check_residuals_for_bound_parameters();

      // some more checks for cyclic variables
      check_residuals_for_cyclic_parameters();

      // inspecific residual tests with random numbers
      random_residual_tests();
    }
  } // end of namespace Test
} // end of namespace Ats