// This file is part of the ACTS project.
//
// Copyright (C) 2016 ACTS project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef ACTS_MAGNETICFIELD_CONSTANTBFIELD_H
#define ACTS_MAGNETICFIELD_CONSTANTBFIELD_H 1

#include "ACTS/MagneticField/concept/AnyFieldLookup.hpp"
#include "ACTS/Utilities/Definitions.hpp"

namespace Acts {

/// @ingroup MagneticField
/// @brief returns a given constant field value at every point
///
/// This class implements a simple constant magnetic field. The
/// magnetic field value has to be set at creation time, but can
/// be updated later on.
class ConstantBField final
{
public:
  /// @brief struct representing smallest grid unit in magnetic field grid
  /// Implementation of the field cell concept for the constant magnetic
  /// field.
  /// @note The field cell for the constant magnetic field is the same
  /// everywhere. The FieldCell for the constant magnetic field is only
  /// implemented for consistency.
  struct FieldCell
  {
  public:
    /// @brief construct constant magnetic field cell from components
    ///
    /// @param [in] Bx magnetic field component in global x-direction
    /// @param [in] By magnetic field component in global y-direction
    /// @param [in] Bz magnetic field component in global z-direction
    FieldCell(double Bx, double By, double Bz) : m_BField(Bx, By, Bz) {}

    /// @brief retrieve field at given position
    ///
    /// @param [in] position global 3D position
    /// @return magnetic field value at the given position
    ///
    /// @note The field is the same everywhere for a constant B-Field
    Vector3D
    getField(const Vector3D& /*position*/) const
    {
      return m_BField;
    }

    /// @brief check whether given 3D position is inside this field cell
    ///
    /// @param [in] position global 3D position
    /// @return @c true if position is inside the current field cell,
    ///         otherwise @c false
    /// @note The method will always return true for the constant B-Field
    bool
    isInside(const Vector3D& /*position*/) const
    {
      return true;
    }

  private:
    /// magnetic field vector
    Vector3D m_BField;
  };

  /// @brief construct constant magnetic field from field vector
  ///
  /// @param [in] B magnetic field vector in global coordinate system
  explicit ConstantBField(Vector3D B)
    : m_BField(std::move(B)), m_fieldCell(B.x(), B.y(), B.z())
  {
  }

  /// @brief construct constant magnetic field from components
  ///
  /// @param [in] Bx magnetic field component in global x-direction
  /// @param [in] By magnetic field component in global y-direction
  /// @param [in] Bz magnetic field component in global z-direction
  ConstantBField(double Bx, double By, double Bz)
    : m_BField(Bx, By, Bz), m_fieldCell(Bx, By, Bz)
  {
  }

  /// @brief retrieve magnetic field value
  ///
  /// @param [in] position global position
  /// @return magnetic field vector
  ///
  /// @note The @p position is ignored and only kept as argument to provide
  ///       a consistent interface with other magnetic field services.
  Vector3D
  getField(const Vector3D& /*position*/) const
  {
    return m_BField;
  }

  /// @brief retrieve field cell for given position
  ///
  /// @param [in] position global 3D position
  /// @return field cell containing the given global position
  ///
  concept::AnyFieldCell<>
  getFieldCell(const Vector3D& /*position*/) const
  {
    return m_fieldCell;
  }

  /// @brief retrieve magnetic field value & its gradient
  ///
  /// @param [in]  position   global position
  /// @param [out] derivative gradient of magnetic field vector as (3x3) matrix
  /// @return magnetic field vector
  ///
  /// @note The @p position is ignored and only kept as argument to provide
  ///       a consistent interface with other magnetic field services.
  /// @note currently the derivative is not calculated
  /// @todo return derivative
  Vector3D
  getFieldGradient(const Vector3D& /*position*/,
                   ActsMatrixD<3, 3>& /*derivative*/) const
  {
    return m_BField;
  }

  /// @brief check whether given 3D position is inside look-up domain
  ///
  /// @param [in] position global 3D position
  /// @return @c true if position is inside the defined look-up grid,
  ///         otherwise @c false
  /// @note The method will always return true for the constant B-Field
  bool
  isInside(const Vector3D& position) const
  {
    return m_fieldCell.isInside(position);
  }

  /// @brief update magnetic field vector from components
  ///
  /// @param [in] Bx magnetic field component in global x-direction
  /// @param [in] By magnetic field component in global y-direction
  /// @param [in] Bz magnetic field component in global z-direction
  void
  setField(double Bx, double By, double Bz)
  {
    m_BField << Bx, By, Bz;
  }

  /// @brief update magnetic field vector
  ///
  /// @param [in] B magnetic field vector in global coordinate system
  void
  setField(const Vector3D& B)
  {
    m_BField = B;
  }

private:
  /// magnetic field vector
  Vector3D m_BField;
  /// The field cell
  FieldCell m_fieldCell;
};
}  // end of namespace Acts

#endif  //> ! ACTS_MAGNETICFIELD_CONSTANTBFIELD_H
