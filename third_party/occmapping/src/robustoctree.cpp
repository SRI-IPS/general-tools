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

#include <cmath>

#include <chrono>
#include <iostream>

#include <exception>
#include <list>

#include "robustoctree.h"

#define MODEL_Z_ERROR  // remove this constant if depth error should not be modeled

// #define this to enable debugging the calcVisibilities() graph traversal logic.
#undef OCC_DEBUG_CALC_VISIBILITIES
#undef OCC_PROFILE_CALC_VISIBILITIES
#undef OCC_PROFILE_COMPUTE_UPDATE
#undef OCC_PROFILE_INSERT_POINT_CLOUD

namespace occmapping {

// A dummy value we use to represent the root key of a ray.
octomap::OcTreeKey RobustOcTree::ROOT_KEY(0xFFFF, 0xFFFF, 0xFFFF);

RobustOcTree::StaticMemberInitializer RobustOcTree::ocTreeMemberInit;

RobustOcTree::RobustOcTree() : Base(RobustOcTreeParameters().octreeResolution) {
  initFromParameters();
}

RobustOcTree::RobustOcTree(const RobustOcTreeParameters& parameters)
    : Base(parameters.octreeResolution), parameters(parameters) {
  initFromParameters();
}

RobustOcTree::~RobustOcTree() { delete lookup; }

void RobustOcTree::initFromParameters() {
  // We keep the inverse probabilities too
  probMissIfOccupied = 1.0 - parameters.probHitIfOccupied;
  probMissIfNotOccupied = 1.0 - parameters.probHitIfNotOccupied;
  probMissIfNotVisible = 1.0 - parameters.probHitIfNotVisible;

  // Convert minimum and maximum height to octree keys
  octomap::OcTreeKey key;
  if (!coordToKeyChecked(0, 0, parameters.minHeight, key))
    minZKey = 0;
  else
    minZKey = key[2];

  if (!coordToKeyChecked(0, 0, parameters.maxHeight, key))
    maxZKey = 0xFFFF;
  else
    maxZKey = key[2];

  // Generate probability look-up table.
  // Some parameters are hard coded here, but you propability don't want to change them anyway.
  lookup = new ProbabilityLookup(parameters.octreeResolution, parameters.depthErrorResolution, 0.1,
                                 parameters.maxRange, 0, 1, 0.05, parameters.depthErrorScale);

  // Hopefully makes accesses to the hash faster
  voxelUpdateCache.max_load_factor(0.75);
  voxelUpdateCache.rehash(1000);
}

void RobustOcTree::insertPointCloud(const octomap::Pointcloud& scan,
                                    const octomap::point3d& sensor_origin,
                                    const octomap::point3d& forwardVec, bool lazy_eval) {
#ifdef OCC_PROFILE_INSERT_POINT_CLOUD
  static auto max_calc_vis_t = 0;
  static auto calc_vis_t_sum = 0;
  static auto calc_vis_count = 0;
  ++calc_vis_count;
  auto t0 = std::chrono::high_resolution_clock::now();
#endif

  // Do ray casting stuff
  computeUpdate(scan, sensor_origin, forwardVec);

#ifdef OCC_PROFILE_INSERT_POINT_CLOUD
  auto t1 = std::chrono::high_resolution_clock::now();
#endif

  // Calculate a visibility for all voxels that will be updated
  calcVisibilities(sensor_origin);

#ifdef OCC_PROFILE_INSERT_POINT_CLOUD
  auto t2 = std::chrono::high_resolution_clock::now();
  auto calc_vis_t = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
  calc_vis_t_sum += calc_vis_t;
  if (calc_vis_t > max_calc_vis_t) {
    max_calc_vis_t = calc_vis_t;
  }
#endif

  // Perform update
  for (OccPrevKeySet::iterator it = voxelUpdateCache.begin(); it != voxelUpdateCache.end(); it++) {
    float visibility = visibilities[it->key];
    if (visibility >= parameters.visibilityClampingMin)
      updateNode(it->key, visibility, it->occupancy, lazy_eval);
  }

#ifdef OCC_PROFILE_INSERT_POINT_CLOUD
  auto t3 = std::chrono::high_resolution_clock::now();
  std::cerr << "InsertPointCloud: computeUpdate: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
            << " ms, calcVisibilities: " << calc_vis_t << " ms (max: " << max_calc_vis_t
            << " ms, avg: " << calc_vis_t_sum / calc_vis_count << " ms),"
            << " updateNode(s): "
            << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count() << " ms"
            << std::endl;
#endif
}

void RobustOcTree::calcKeyVisibility(const OccPrevKey& key, const octomap::OcTreeKey& originKey) {
  KeyVisMap::iterator visIter = visibilities.find(key.prevKey);
  if (visIter == visibilities.end()) {
    assert(false && "visIter not found");
    OCTOMAP_WARNING_STR("visIter not found!!!");
    return;
  }

  // Find occupancies for the three neighbor nodes bordering
  // the visible faces
  octomap::OcTreeKey currKey = key.key;

  int step[3] = {currKey[0] > originKey[0] ? 1 : -1, currKey[1] > originKey[1] ? 1 : -1,
                 currKey[2] > originKey[2] ? 1 : -1};
  octomap::OcTreeKey neighborKeys[3] = {
      octomap::OcTreeKey(currKey[0] - step[0], currKey[1], currKey[2]),
      octomap::OcTreeKey(currKey[0], currKey[1] - step[1], currKey[2]),
      octomap::OcTreeKey(currKey[0], currKey[1], currKey[2] - step[2])};

  float occupancies[3] = {0.5F, 0.5F, 0.5F};

  for (unsigned int i = 0; i < 3; i++) {
    RobustOcTreeNode* neighborNode = search(neighborKeys[i]);
    if (neighborNode != NULL) {
      occupancies[i] = neighborNode->getOccupancy();
      if (occupancies[i] <= parameters.clampingThresholdMin) occupancies[i] = 0;
    }
  }

  // Get the probabilities that the current voxel is occluded by its neighbors
  float occlusionProb = std::min(occupancies[0], std::min(occupancies[1], occupancies[2]));
  float visibility =
      visIter->second * (parameters.probVisibleIfOccluded * occlusionProb +
                         parameters.probVisibleIfNotOccluded * (1.0F - occlusionProb));

  // clamp visibility
  if (visibility > parameters.visibilityClampingMax) visibility = 1.0F;

  visibilities[currKey] = visibility;
}

void RobustOcTree::calcVisibilities(const octomap::point3d& sensor_origin) {
#ifdef OCC_PROFILE_CALC_VISIBILITIES
  auto voxel_update_cache_size = voxelUpdateCache.size();
#endif
#ifdef OCC_DEBUG_CALC_VISIBILITIES
  std::cerr << "voxelUpdateCache size: " << voxelUpdateCache.size() << std::endl;
#endif

  // Find the origin key
  octomap::OcTreeKey originKey;
  if (!coordToKeyChecked(sensor_origin, originKey)) {
    OCTOMAP_WARNING_STR("origin coordinates ( " << sensor_origin
                                                << ") out of bounds in calcVisibilities");
    return;
  }

  visibilities.clear();
  visibilities[ROOT_KEY] = 1.0;

  keyGraph.clear();
#ifdef OCC_PROFILE_CALC_VISIBILITIES
  auto t0 = std::chrono::high_resolution_clock::now();
#endif
  // Stores the parent's iterator and the child iterator that is currently being processed.
  auto parentStack = std::stack<
      std::pair<KeyGraphMap::iterator, std::list<const OccPrevKey *>::iterator>>{};
  auto visitedKeys = std::unordered_set<octomap::OcTreeKey, octomap::OcTreeKey::KeyHash>{};
  for (const auto& cacheKey : voxelUpdateCache) {
    keyGraph[cacheKey.prevKey].push_back(&cacheKey);
  }
#ifdef OCC_PROFILE_CALC_VISIBILITIES
  auto t1 = std::chrono::high_resolution_clock::now();
  auto create_key_graph_t = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
#endif

#ifdef OCC_PROFILE_CALC_VISIBILITIES
  auto key_graph_size = keyGraph.size();
#endif
#ifdef OCC_DEBUG_CALC_VISIBILITIES
  std::cerr << "keyGraph size: " << keyGraph.size() << std::endl;
#endif

#ifdef OCC_PROFILE_CALC_VISIBILITIES
  auto calc_key_visibilities_count = 0;
  auto calc_key_visibilities_t_sum = 0;
  t0 = std::chrono::high_resolution_clock::now();
#endif
  auto currNode = keyGraph.find(ROOT_KEY);
  if (currNode == keyGraph.end()) {
    OCTOMAP_WARNING_STR("Empty key graph in calcVisibilities");
    return;
  }
  visitedKeys.insert(currNode->first);
  auto childIt = currNode->second.begin();
  while (true) {
    if (childIt == currNode->second.end()) {
      if (parentStack.empty()) {
        // We are done!
        break;
      }
#ifdef OCC_DEBUG_CALC_VISIBILITIES
      std::cerr << "Leaving child " << *childIt << std::endl;
#endif
      // Advance to the next child of the parent.
      currNode = parentStack.top().first;
      childIt = parentStack.top().second;
      parentStack.pop();
      ++childIt;
      continue;
    }
#ifdef OCC_DEBUG_CALC_VISIBILITIES
    std::cerr << "Updating visibility for child " << *childIt << std::endl;
#endif
#ifdef OCC_PROFILE_CALC_VISIBILITIES
    ++calc_key_visibilities_count;
    auto calc_key_visibility_t0 = std::chrono::high_resolution_clock::now();
#endif
    calcKeyVisibility(**childIt, originKey);
#ifdef OCC_PROFILE_CALC_VISIBILITIES
    auto calc_key_visibility_t1 = std::chrono::high_resolution_clock::now();
    calc_key_visibilities_t_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(
                                       calc_key_visibility_t1 - calc_key_visibility_t0)
                                       .count();
#endif
    auto childNode = keyGraph.find((*childIt)->key);
    if (childNode == keyGraph.end()) {
      // This child has no sub-children to recurse to, so just advance to the next child.
      ++childIt;
#ifdef OCC_DEBUG_CALC_VISIBILITIES
      std::cerr << "Child has no children, advancing to child " << *childIt << std::endl;
#endif
      continue;
    }
    if (visitedKeys.count(childNode->first)) {
      // TODO(kgreenek): Verify whether this can actually happen.
      assert(false && "Cycle detected in key graph");
      OCTOMAP_WARNING_STR("Cycle detected in key graph");
      ++childIt;
      continue;
    }
#ifdef OCC_DEBUG_CALC_VISIBILITIES
    std::cerr << "Recursing into child " << *childIt << std::endl;
#endif
    // Recurse down into the childNode.
    parentStack.push({currNode, childIt});
    currNode = childNode;
    visitedKeys.insert(currNode->first);
    childIt = currNode->second.begin();
  }
#ifdef OCC_PROFILE_CALC_VISIBILITIES
  t1 = std::chrono::high_resolution_clock::now();
  auto key_graph_traversal_t =
      std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

  std::cerr << "calcVisibilities:\n"
            << "  voxelUpdateCache size: " << voxel_update_cache_size << "\n"
            << "  keyGraph size: " << key_graph_size << ", create_t: " << create_key_graph_t
            << " ms\n"
            << "  key_graph_traversal_t: " << key_graph_traversal_t << "\n"
            << "  calcKeyVisibility: count: " << calc_key_visibilities_count
            << ", total_t: " << calc_key_visibilities_t_sum / 1000000
            << " ms, avg_t: " << calc_key_visibilities_t_sum / calc_key_visibilities_count / 1000
            << " us" << std::endl;
#endif
}

void RobustOcTree::insertPointCloud(const octomap::Pointcloud& pc,
                                    const octomap::point3d& sensor_origin,
                                    const octomap::pose6d& frame_origin, bool lazy_eval) {
  // performs transformation to data and sensor origin first
  octomap::Pointcloud transformed_scan(pc);
  transformed_scan.transform(frame_origin);
  octomap::point3d transformed_sensor_origin = frame_origin.transform(sensor_origin);
  octomap::point3d forwardVec =
      frame_origin.transform(sensor_origin + octomap::point3d(0, 1.0, 0.0)) -
      transformed_sensor_origin;

  // Process transformed point cloud
  insertPointCloud(transformed_scan, transformed_sensor_origin, forwardVec, lazy_eval);
}

void RobustOcTree::computeUpdate(const octomap::Pointcloud& scan, const octomap::point3d& origin,
                                 const octomap::point3d& forwardVec) {
  voxelUpdateCache.clear();
  prevKeyray.reset();

#ifdef OCC_PROFILE_COMPUTE_UPDATE
  auto compute_ray_keys_sum = 0;
  auto compute_ray_keys_count = 0;
  auto update_cache_sum = 0;
  auto update_cache_count = 0;
#endif

  for (int i = 0; i < (int)scan.size(); ++i) {
    // Compute a new ray
    const octomap::point3d& p = scan[i];

#ifdef OCC_PROFILE_COMPUTE_UPDATE
    auto t0 = std::chrono::high_resolution_clock::now();
#endif

    computeRayKeys(origin, p, forwardVec);

#ifdef OCC_PROFILE_COMPUTE_UPDATE
    auto t1 = std::chrono::high_resolution_clock::now();
    compute_ray_keys_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    ++compute_ray_keys_count;
#endif

    // Insert cells into the update cache if they haven't been
    // part of a previous ray, or if their occupancy is higher than
    // for any previous ray.

    // First perform lookup against previous ray
    OccPrevKeyRay::iterator currIt = keyray.begin();
    OccPrevKeyRay::iterator currEnd =
        prevKeyray.size() > keyray.size() ? keyray.end() : keyray.begin() + prevKeyray.size();

    for (OccPrevKeyRay::iterator prevIt = prevKeyray.begin(); currIt != currEnd;
         currIt++, prevIt++) {
      if (currIt->key == prevIt->key) {
        // Cell already exists in previous ray
        if (currIt->occupancy > prevIt->occupancy) {
          OccPrevKeySet::iterator updateIter = voxelUpdateCache.find(*currIt);
          OccPrevKeySet::iterator insertHint = voxelUpdateCache.erase(updateIter);
          voxelUpdateCache.insert(insertHint, *currIt);
        } else {
          currIt->occupancy = prevIt->occupancy;  // Keep best occupancy for next ray
        }
      } else {
        // Perform full lookup against hash map
        OccPrevKeySet::iterator updateIter = voxelUpdateCache.find(*currIt);
        if (updateIter == voxelUpdateCache.end()) {
          voxelUpdateCache.insert(*currIt);
        } else if (currIt->occupancy > updateIter->occupancy) {
          OccPrevKeySet::iterator insertHint = voxelUpdateCache.erase(updateIter);
          voxelUpdateCache.insert(insertHint, *currIt);
        } else {
          currIt->occupancy = updateIter->occupancy;  // Keep best occupancy for next ray
        }
      }
    }

#ifdef OCC_PROFILE_COMPUTE_UPDATE
    auto t2 = std::chrono::high_resolution_clock::now();
#endif

    // Lookup remaining cells against hash map
    for (; currIt != keyray.end(); currIt++) {
      OccPrevKeySet::iterator updateIter = voxelUpdateCache.find(*currIt);
      if (updateIter == voxelUpdateCache.end()) {
        voxelUpdateCache.insert(*currIt);
      } else if (currIt->occupancy > updateIter->occupancy) {
        OccPrevKeySet::iterator insertHint = voxelUpdateCache.erase(updateIter);
        voxelUpdateCache.insert(insertHint, *currIt);
      } else {
        currIt->occupancy = updateIter->occupancy;  // Keep best occupancy for next ray
      }
    }

#ifdef OCC_PROFILE_COMPUTE_UPDATE
    auto t3 = std::chrono::high_resolution_clock::now();
    update_cache_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
    ++update_cache_count;
#endif

    keyray.swap(prevKeyray);
  }

#ifdef OCC_PROFILE_COMPUTE_UPDATE
  std::cerr << "computeUpdate: computeRayKeys: " << compute_ray_keys_sum / 1000000
            << " ms, voxelUpdateCache: " << update_cache_sum / 1000000 << " ms"
            << std::endl;
#endif
}

void RobustOcTree::computeRayKeys(const octomap::point3d& origin, const octomap::point3d& end,
                                  const octomap::point3d& forwardVec) {
  // see "A Faster Voxel Traversal Algorithm for Ray Tracing" by Amanatides & Woo
  // basically: DDA in 3D

  keyray.reset();

  octomap::point3d direction = (end - origin);
  double length = direction.norm();
  direction /= length;  // normalize vector

  octomap::OcTreeKey key_origin, key_end;
  if (!coordToKeyChecked(origin, key_origin)) {
    OCTOMAP_WARNING_STR("origin coordinates ( " << origin << " -> " << end
                                                << ") out of bounds in computeRayKeys");
    return;
  }
  if (!coordToKeyChecked(end, key_end) &&
      !coordToKeyChecked(origin + direction * (2.0 * parameters.maxRange), key_end)) {
    OCTOMAP_WARNING_STR("end coordinates ( " << origin << " -> " << end
                                             << ") out of bounds in computeRayKeys");
    return;
  }

  // Initialization phase -------------------------------------------------------

  int step[3];
  double tMax[3];
  double tDelta[3];

  octomap::OcTreeKey prev_key = ROOT_KEY;
  octomap::OcTreeKey current_key = key_origin;

  for (unsigned int i = 0; i < 3; ++i) {
    // compute step direction
    if (direction(i) > 0.0)
      step[i] = 1;
    else if (direction(i) < 0.0)
      step[i] = -1;
    else
      step[i] = 0;

    // compute tMax, tDelta
    if (step[i] != 0) {
      // corner point of voxel (in direction of ray)
      double voxelBorder = keyToCoord(current_key[i]);
      voxelBorder += step[i] * resolution * 0.5;

      tMax[i] = (voxelBorder - origin(i)) / direction(i);
      tDelta[i] = this->resolution / fabs(direction(i));
    } else {
      tMax[i] = std::numeric_limits<double>::max();
      tDelta[i] = std::numeric_limits<double>::max();
    }
  }

  // for speedup:
  octomap::point3d origin_scaled = origin;
  origin_scaled /= resolution;

  double length_to_key = (keyToCoord(key_end) - origin).norm();
  double length_scaled = (std::min(length_to_key, parameters.maxRange)) / resolution;
  length_scaled = length_scaled * length_scaled;
  double maxrange_scaled = parameters.maxRange / resolution;
  maxrange_scaled *= maxrange_scaled;

  // Conversion factor from length to depth (length of one z-step)
  double lengthToDepth = forwardVec.x() * direction.x() + forwardVec.y() * direction.y() +
                         forwardVec.z() * forwardVec.z();

  double depth = length / lengthToDepth;
  double probApproxMaxDistScaled, probApproxMaxDist;
  std::vector<float>* lookupEntry = lookup->lookupEntry(depth);
  if (lookupEntry == NULL) {
    probApproxMaxDist = std::numeric_limits<double>::max();
    probApproxMaxDistScaled = maxrange_scaled;
  } else {
    probApproxMaxDist =
        lengthToDepth * (depth - (lookupEntry->size() / 2.0) * parameters.depthErrorResolution);
    probApproxMaxDistScaled = probApproxMaxDist / resolution;
    probApproxMaxDistScaled =
        std::min(probApproxMaxDistScaled * probApproxMaxDistScaled, maxrange_scaled);
  }
  double treeMax05 = tree_max_val - 0.5F;
  // Incremental phase	---------------------------------------------------------

  unsigned int dim = 0;
  double squareDistFromOrigVec[3] = {(current_key[0] - treeMax05 - origin_scaled(0)) *
                                         (current_key[0] - treeMax05 - origin_scaled(0)),
                                     (current_key[1] - treeMax05 - origin_scaled(1)) *
                                         (current_key[1] - treeMax05 - origin_scaled(1)),
                                     (current_key[2] - treeMax05 - origin_scaled(2)) *
                                         (current_key[2] - treeMax05 - origin_scaled(2))};

  while (true) {
    // Calculate distance from origin
    double squareDistFromOrig =
        squareDistFromOrigVec[0] + squareDistFromOrigVec[1] + squareDistFromOrigVec[2];

#ifdef MODEL_Z_ERROR
    if (squareDistFromOrig < probApproxMaxDistScaled) {
      // Use approximate probabilities
      keyray.addOccPrevKey(0.0, current_key, prev_key);
    } else if (squareDistFromOrig >= maxrange_scaled) {
      // The point is too far away. Lets stop.
      break;
    } else {
      // Detailed calculation
      int index = int((sqrt(squareDistFromOrig) * resolution - probApproxMaxDist) /
                          (parameters.depthErrorResolution * lengthToDepth) +
                      0.5);

      double occupancyFactor = 1.0;
      bool done = false;
      if (index >= (int)lookupEntry->size()) {
        done = true;
        // Continue to make sure we integrate one full hit
      } else {
        occupancyFactor = (*lookupEntry)[index];
      }

      keyray.addOccPrevKey(occupancyFactor, current_key, prev_key);

      if (done) break;
    }
#else
    // reached endpoint?
    if (current_key == key_end) {
      keyray.addOccPrevKey(1.0, current_key, prev_key);
      break;
    } else if (squareDistFromOrig >= length_scaled) {
      // We missed it :-(
      if (length_to_key < parameters.maxRange) keyray.addOccPrevKey(1.0, key_end, prev_key);
      break;
    } else {
      keyray.addOccPrevKey(0.0, current_key, prev_key);
    }
#endif

    // find minimum tMax:
    if (tMax[0] < tMax[1]) {
      if (tMax[0] < tMax[2])
        dim = 0;
      else
        dim = 2;
    } else {
      if (tMax[1] < tMax[2])
        dim = 1;
      else
        dim = 2;
    }

    // advance in direction "dim"
    prev_key = current_key;
    current_key[dim] += step[dim];
    tMax[dim] += tDelta[dim];
    squareDistFromOrigVec[dim] = current_key[dim] - treeMax05 - origin_scaled(dim);
    squareDistFromOrigVec[dim] *= squareDistFromOrigVec[dim];

    assert(current_key[dim] < 2 * this->tree_max_val);

    if (current_key[2] < minZKey || current_key[2] > maxZKey) break;  // Exceeded min or max height

    // TODO(kgreenek): Re-enable this assert once a17hq/a17#150 is fixed. This assert is being
    // triggered for some datasets, when it shouldn't be.
#if 0
    assert(keyray.size() < keyray.sizeMax() - 1);
#else
    if (keyray.size() >= keyray.sizeMax() - 1) {
      std::cerr << "WARNING: keyraw.size >= keyraw.sizeMax(). Skipping scan..." << std::endl;
      break;
    }
#endif
  }
}

RobustOcTreeNode* RobustOcTree::updateNode(const octomap::OcTreeKey& key, float visibility,
                                           float occupancy, bool lazy_eval) {
  // early abort (no change will happen).
  // may cause an overhead in some configuration, but more often helps
  RobustOcTreeNode* leaf = this->search(key);
  // no change: node already at minimum threshold
  if (leaf != NULL && occupancy == 0.0F && leaf->getOccupancy() <= parameters.clampingThresholdMin)
    return leaf;

  bool createdRoot = false;
  if (this->root == NULL) {
    this->root = new RobustOcTreeNode();
    this->tree_size++;
    createdRoot = true;
  }

  return updateNodeRecurs(this->root, visibility, createdRoot, key, 0, occupancy, lazy_eval);
}

RobustOcTreeNode* RobustOcTree::updateNodeRecurs(RobustOcTreeNode* node, float visibility,
                                                 bool node_just_created,
                                                 const octomap::OcTreeKey& key, unsigned int depth,
                                                 float occupancy, bool lazy_eval) {
  // Differences to OccupancyOcTreeBase implementation: only cosmetic

  unsigned int pos = computeChildIdx(key, this->tree_depth - 1 - depth);
  bool created_node = false;

  assert(node);

  // follow down to last level
  if (depth < this->tree_depth) {
    if (!nodeChildExists(node, pos)) {
      // child does not exist, but maybe it's a pruned node?
      if ((!nodeHasChildren(node)) && !node_just_created) {
        // current node does not have children AND it is not a new node
        // -> expand pruned node
        expandNode(node);
        this->tree_size += 8;
        this->size_changed = true;
      } else {
        // not a pruned node, create requested child
        createNodeChild(node, pos);
        this->tree_size++;
        this->size_changed = true;
        created_node = true;
      }
    }

    if (lazy_eval)
      return updateNodeRecurs(getNodeChild(node, pos), visibility, created_node, key, depth + 1,
                              occupancy, lazy_eval);
    else {
      RobustOcTreeNode* retval = updateNodeRecurs(getNodeChild(node, pos), visibility, created_node,
                                                  key, depth + 1, occupancy, lazy_eval);
      // prune node if possible, otherwise set own probability
      // note: combining both did not lead to a speedup!
      if (pruneNode(node))
        this->tree_size -= 8;
      else
        node->updateOccupancyChildren();

      return retval;
    }
  }

  // at last level, update node, end of recursion
  else {
    if (use_change_detection) {
      bool occBefore = this->isNodeOccupied(node);
      updateNodeOccupancy(node, visibility, occupancy);

      if (node_just_created) {  // new node
        changed_keys.insert(std::pair<octomap::OcTreeKey, bool>(key, true));
      } else if (occBefore != this->isNodeOccupied(node)) {  // occupancy changed, track it
        auto it = changed_keys.find(key);
        if (it == changed_keys.end()) {
          changed_keys.insert(std::pair<octomap::OcTreeKey, bool>(key, false));
        } else if (it->second == false) {
          // This happens when the value changes back to what it was, so we can just remove it,
          // because it has no longer changed since the last time changed_keys was cleared.
          changed_keys.erase(it);
        }
      }
    } else {
      updateNodeOccupancy(node, visibility, occupancy);
    }
    return node;
  }
}

void RobustOcTree::updateNodeOccupancy(RobustOcTreeNode* occupancyNode, float probVisible,
                                       float occupancy) {
  float probOccupied = occupancyNode->getOccupancy();

  // Decide which update procedure to perform
  if (occupancy == 0.0F) {
    // Perform update for certainly free voxels
    float probMiss = calcOccupiedProbability(probOccupied, probVisible, probMissIfOccupied,
                                             probMissIfNotOccupied, probMissIfNotVisible);
    occupancyNode->setOccupancy(
        probMiss > parameters.clampingThresholdMin ? probMiss : parameters.clampingThresholdMin);
  } else {
    float probHit =
        calcOccupiedProbability(probOccupied, probVisible, parameters.probHitIfOccupied,
                                parameters.probHitIfNotOccupied, parameters.probHitIfNotVisible);
    if (occupancy == 1.0F) {
      // Perform update for certainly occupied voxels
      occupancyNode->setOccupancy(
          probHit < parameters.clampingThresholdMax ? probHit : parameters.clampingThresholdMax);
    } else {
      // We are not certain, but we know the occupancy probability.
      // Lets interpolate between the update for free and occupied voxels.
      float probMiss =
          calcOccupiedProbability(probOccupied, probVisible, probMissIfOccupied,
                                  probMissIfNotOccupied, parameters.probHitIfNotVisible);
      occupancyNode->setOccupancy(occupancy * probHit + (1.0F - occupancy) * probMiss);

      // Perform probability clamping
      if (occupancyNode->getOccupancy() < parameters.clampingThresholdMin)
        occupancyNode->setOccupancy(parameters.clampingThresholdMin);
      else if (occupancyNode->getOccupancy() > parameters.clampingThresholdMax)
        occupancyNode->setOccupancy(parameters.clampingThresholdMax);
    }
  }
}

float RobustOcTree::calcOccupiedProbability(float probOccupied, float probVisible,
                                            float probMeasIfOccupied, float probMeasIfNotOccupied,
                                            float probMeasNotVisible) {
  // This is our update equation. See our paper for a derivation of this formula (Eq. 4 - 6).
  return (probMeasIfOccupied * probVisible * probOccupied +
          probMeasNotVisible * (1.0F - probVisible) * probOccupied) /
         (probMeasIfOccupied * probVisible * probOccupied +
          probMeasIfNotOccupied * probVisible * (1.0F - probOccupied) +
          probMeasNotVisible * (1.0F - probVisible));
}

}  // namespace occmapping
