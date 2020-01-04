/*
 * Copyright (c) 2013-2014, Konstantin Schauwecker
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

#ifndef OCCMAPPING_ROBUSTOCTREE_H_
#define OCCMAPPING_ROBUSTOCTREE_H_

#include <unordered_map>

#include <octomap/OcTree.h>
#include <octomap/OcTreeBase.h>

#include "occprevkeyray.h"
#include "probabilitylookup.h"
#include "robustoctreenode.h"
#include "robustoctreeparameters.h"

namespace occmapping {

/// A more noise robust occupancy octree designed specifically to handle data from stereo vision.
class RobustOcTree : public octomap::OcTreeBase<RobustOcTreeNode> {
 public:
  RobustOcTree();
  RobustOcTree(const RobustOcTreeParameters& parameters);

  virtual ~RobustOcTree();

  // Methods for processing scans
  virtual void insertPointCloud(const octomap::Pointcloud& scan,
                                const octomap::point3d& sensor_origin,
                                const octomap::point3d& forwardVec, bool lazy_eval = false);

  virtual void insertPointCloud(const octomap::Pointcloud& pc,
                                const octomap::point3d& sensor_origin,
                                const octomap::pose6d& frame_origin, bool lazy_eval = false);

  // virtual constructor: creates a new object of same type
  virtual RobustOcTree* create() const { return new RobustOcTree(parameters); }

  std::string getTreeType() const {
    return "OcTree";  // Ensures compatibility with OcTree
  }

  // Performs thresholding of the occupancy probability for the given node
  inline bool isNodeOccupied(const RobustOcTreeNode& node) const {
    return node.getOccupancy() > parameters.occupancyProbThreshold;
  }

  inline bool isNodeOccupied(const RobustOcTreeNode* node) const { return isNodeOccupied(*node); }

  //-- change detection on occupancy:
  /// track or ignore changes while inserting scans (default: ignore)
  void enableChangeDetection(bool enable) { use_change_detection = enable; }
  bool isChangeDetectionEnabled() const { return use_change_detection; }
  /// Reset the set of changed keys. Call this after you obtained all changed nodes.
  void resetChangeDetection() { changed_keys.clear(); }

  /**
   * Iterator to traverse all keys of changed nodes.
   * you need to enableChangeDetection() first. Here, an OcTreeKey always
   * refers to a node at the lowest tree level (its size is the minimum tree resolution)
   */
  octomap::KeyBoolMap::const_iterator changedKeysBegin() const { return changed_keys.begin(); }

  /// Iterator to traverse all keys of changed nodes.
  octomap::KeyBoolMap::const_iterator changedKeysEnd() const { return changed_keys.end(); }

  /// Number of changes since last reset.
  size_t numChangesDetected() const { return changed_keys.size(); }

 private:
  typedef octomap::OcTreeBase<RobustOcTreeNode> Base;
  typedef std::unordered_map<octomap::OcTreeKey, float, octomap::OcTreeKey::KeyHash> KeyVisMap;
  typedef std::unordered_map<octomap::OcTreeKey, std::list<const OccPrevKey*>,
                             octomap::OcTreeKey::KeyHash>
      KeyGraphMap;

  /**
   * Static member object which ensures that this OcTree's prototype
   * ends up in the classIDMapping only once.
   */
  class StaticMemberInitializer {
   public:
    StaticMemberInitializer() {
      RobustOcTree* tree = new RobustOcTree();
      octomap::AbstractOcTree::registerTreeType(tree);
    }
  };
  // Ensure static initialization (only once).
  static StaticMemberInitializer ocTreeMemberInit;

  static octomap::OcTreeKey ROOT_KEY;

  RobustOcTreeParameters parameters;
  float probMissIfOccupied, probMissIfNotOccupied;
  float probMissIfNotVisible;
  OccPrevKeyRay keyray, prevKeyray;
  OccPrevKeySet voxelUpdateCache;
  KeyVisMap visibilities;
  KeyGraphMap keyGraph;
  unsigned short int maxZKey, minZKey;
  ProbabilityLookup* lookup;

  bool use_change_detection = false;
  /// Set of leaf keys (lowest level) which changed since last resetChangeDetection
  octomap::KeyBoolMap changed_keys;

  // Performs initialization using the parameters attribute
  void initFromParameters();

  // Casts all rays and fills the update cache
  void computeUpdate(const octomap::Pointcloud& scan, const octomap::point3d& origin,
                     const octomap::point3d& forwardVec);

  // Casts a single ray and stores it in keyray
  void computeRayKeys(const octomap::point3d& origin, const octomap::point3d& end,
                      const octomap::point3d& forwardVec);

  // Manipulate occupancy probability of node directly
  RobustOcTreeNode* updateNodeRecurs(RobustOcTreeNode* node, float visibility,
                                     bool node_just_created, const octomap::OcTreeKey& key,
                                     unsigned int depth, float occupancy, bool lazy_eval);
  RobustOcTreeNode* updateNode(const octomap::OcTreeKey& key, float visibility, float occupancy,
                               bool lazy_eval);
  inline void updateNodeOccupancy(RobustOcTreeNode* occupancyNode, float occupancy,
                                           float measProbIfNotOcc);

  // Calculates the new occupancy probability
  inline float calcOccupiedProbability(float probOccupied, float probVisible,
                                                float probMeasIfOccupied,
                                                float probMeasIfNotOccupied,
                                                float probMeasNotVisible);

  // Computes a visibility value for all nodes in voxelUpdateCache
  void calcVisibilities(const octomap::point3d& sensor_origin);
  // Computes a visibility value for the given key.
  void calcKeyVisibility(const OccPrevKey& key, const octomap::OcTreeKey& originKey);
};

}  // namespace occmapping

#endif  // OCCMAPPING_ROBUSTOCTREE_H_
