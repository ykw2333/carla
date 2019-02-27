// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"
#include "carla/geom/CubicPolynomial.h"

namespace carla {
namespace road {
namespace element {

  // The lane offset record defines a lateral shift of the lane reference
  // line(which is usually identical to the road reference line). This may be
  // used for an easy implementation of a (local)lateralshift of the lanes
  // relative to the road’s referenceline. Especially the modeling of
  // inner-city layouts or "2+1" cross-country road layouts can be
  // facilitated considerably by this feature.
  class RoadInfoLaneOffset : public RoadInfo {
  public:

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    RoadInfoLaneOffset(
        double s,
        double a,
        double b,
        double c,
        double d)
      : RoadInfo(s),
        _offset(a, b, c, d, s) {}

    const geom::CubicPolynomial &GetPolynomial() const {
      return _offset;
    }

    double ComputeOffset(const double &dist) const {
      return _offset.Evaluate(dist);
    }

  private:

    geom::CubicPolynomial _offset;

  };

} // namespace element
} // namespace road
} // namespace carla
