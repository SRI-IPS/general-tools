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

#include "robustoctreenode.h"

namespace occmapping {

double RobustOcTreeNode::getMeanChildOccupancy() const {
  auto count = 0;
  auto sum = 0.0;
  for (auto i = 0; i < 8; ++i)
    if (children[i] != nullptr) {
      sum += static_cast<RobustOcTreeNode *>(children[i])->getOccupancy();
      count++;
    }
  if (count > 0) {
    return sum / count;
  }
  return 0;
}

double RobustOcTreeNode::getMaxChildOccupancy() const {
  auto max_occupancy = static_cast<float>(0);
  for (auto i = 0; i < 8; ++i) {
    if (children[i] != nullptr) {
      auto occupancy = static_cast<RobustOcTreeNode *>(children[i])->getOccupancy();
      if (occupancy > max_occupancy) {
        max_occupancy = occupancy;
      }
    }
  }
  return max_occupancy;
}


std::ostream& RobustOcTreeNode::writeData(std::ostream& s) const {
  // Convert value to logodds so the *.ot file can be read as an octomap::OcTree.
  auto log_odds = getLogOdds();
  s.write(reinterpret_cast<const char *>(&log_odds), sizeof(log_odds));
  return s;
}

std::istream& RobustOcTreeNode::readData(std::istream& s) {
  auto log_odds = static_cast<float>(0.0);
  s.read(reinterpret_cast<char *>(&log_odds), sizeof(log_odds));
  // We converted to logodds to be compatible with OctoMap::OcTree, so need to convert back here.
  setLogOdds(log_odds);
  return s;
}

}  // namespace occmapping
