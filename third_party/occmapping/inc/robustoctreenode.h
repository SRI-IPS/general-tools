/*
 * Copyright (c) 2013-2014, Konstantin Schauwecker
 *
 * This code is based on the original OctoMap implementation. The original
 * copyright notice and source code license are as follows:
 *
 * Copyright (c) 2009-2013, K.M. Wurm and A. Hornung, University of Freiburg
 * All rights reserved.
 * License: New BSD
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of Freiburg nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OCCMAPPING_ROBUSTOCTREENODE_H_
#define OCCMAPPING_ROBUSTOCTREENODE_H_

#include <octomap/OcTreeDataNode.h>
#include <octomap/octomap_utils.h>

namespace occmapping {

/// An octree node that counts its observations and can be pruned if not observed often enough.
/// The node's value represents the probability that the node is occupied, and ranges from 0 to 1.
class RobustOcTreeNode : public octomap::OcTreeDataNode<float> {
 public:
  static constexpr auto kDefaultValue = static_cast<float>(0.5);

  RobustOcTreeNode() : OcTreeDataNode<float>(kDefaultValue) {}
  RobustOcTreeNode(const RobustOcTreeNode& rhs) : OcTreeDataNode<float>(rhs) {}

  /// Get the probability that the node is occupied, probability from 0 to 1.
  inline float getOccupancy() const { return value; }
  /// Set the probability that the node is occupied, probability from 0 to 1.
  inline void setOccupancy(float p) { value = p; }

  /// Set the probability that the node is occupied, in log odds.
  inline void setLogOdds(float log_odds) { value = octomap::probability(log_odds); }
  /// Get the probability that the node is occupied, in log odds.
  inline float getLogOdds() const { return octomap::logodds(value); }

  /// Returns the average occupancy probability value of all children.
  double getMeanChildOccupancy() const;
  /// Returns the maximum occupancy probability value of all children.
  double getMaxChildOccupancy() const;

  /// Update this node's occupancy according to its children's maximum occupancy.
  inline void updateOccupancyChildren() {
    // Use the most conservative value.
    setOccupancy(getMaxChildOccupancy());
  }

  std::istream& readData(std::istream& s);  // override
  std::ostream& writeData(std::ostream& s) const;  // override
};

}  // namespace occmapping

#endif  // OCCMAPPING_ROBUSTOCTREENODE_H_
