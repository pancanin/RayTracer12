#ifndef AABB_TREE_H
#define AABB_TREE_H

#include <vector>
#include <stack>
#include <iostream>

#include "sampling/Triangle.h"
#include "AABB.h"
#include "sampling/IntersectionData.h"
#include "utils/TypeDefs.h"

struct Node {
	Node(const std::vector<Triangle>& triangles, bool isLeaf, Vec3& min, Vec3& max, int child1, int child2) :
		triangles(triangles),
		isLeaf(isLeaf),
		box(AABB(min, max)),
		child1Idx(child1),
		child2Idx(child2) {}
	Node() = default;

	AABB box;
	std::vector<Triangle> triangles;
	bool isLeaf;
	int child1Idx{};
	int child2Idx{};
};

struct AABBTree {
	inline AABBTree(const std::vector<Object>& objects, int32_t triangleCount = 0): triangleCount(triangleCount) {
		std::cout << "Building optimisation structure...\n";
		AABB aabb;
		std::vector<Triangle> triangles;
		triangles.reserve(triangleCount);
		materialMap.reserve(objects.size());
		for (size_t i = 0; i < objects.size(); i++) {
			for (const Triangle& tri : objects[i].triangles) {
				aabb.expand(tri);
				triangles.push_back(tri);
			}
			materialMap.push_back(objects[i].mat);
		}
		buildTree(triangles, aabb, 0, -1, -1, 0);

		std::cout << "Done building optimisation structure...\n";
	}

	inline IntersectionData intersectAABBTree(const Ray& ray) {
		std::vector<Node> boxStack;
		boxStack.reserve(maxDepth + 1);

		boxStack.push_back(nodes[0]);
		IntersectionData intrsData{};

		while (boxStack.size() > 0) {
			Node currentBox = boxStack.back(); boxStack.pop_back();

			if (currentBox.box.intersect(ray)) {
				// The ray intersects the box lets move on to its children! :}
				if (currentBox.isLeaf) {
					// This is a leaf node, let's intersect all the objects in it and update intrsData
					for (const Triangle& tri : currentBox.triangles) {
						if (tri.intersect(ray, intrsData)) {
							intrsData.mat = materialMap[tri.objIdx];
						}
					}
				}
				else {
					// This is not a leaf node. Let's add it's children to the stack
					boxStack.push_back(nodes[currentBox.child1Idx]);
					boxStack.push_back(nodes[currentBox.child2Idx]);
				}
			}
		}

		return intrsData;
	}

private:
	std::vector<Node> nodes;
	int32_t leafSize = 1;
	std::vector<Material> materialMap;
	int32_t maxDepth = 12;
	int32_t triangleCount = 0;

	inline void buildTree(const std::vector<Triangle>& tris, AABB& box, int component, int parentIdx, int child, int depth) {
		std::vector<Triangle> triangles;

		// Add the triangles that have intersection with the box
			for (const Triangle& tri : tris) {
				if (box.intersect(tri)) {
					triangles.push_back(tri);
				}
			}

		int idx = nodes.size();

		// Are we processing a first or second child?
		if (parentIdx > -1) {
			if (child == 1) {
				nodes[parentIdx].child1Idx = idx;
			}
			else {
				nodes[parentIdx].child2Idx = idx;
			}
		}

		// We should not continue splitting further
		if (depth > maxDepth || triangles.size() <= leafSize) {
			nodes.emplace_back(triangles, true, box.min, box.max, -1, -1);
			
			return;
		}

		// Add a new parent node
		nodes.emplace_back(std::vector<Triangle>{}, false, box.min, box.max, -1, -1);

		// Continue splitting the box
		AABB firstChildBox = box;
		firstChildBox.max[component] = firstChildBox.min[component] + ((firstChildBox.max[component] - firstChildBox.min[component]) / 2);
		buildTree(triangles, firstChildBox, (component + 1) % 3, idx, 1, depth + 1);
		AABB secondChildBox = box;
		secondChildBox.min[component] = firstChildBox.max[component];
		buildTree(triangles, secondChildBox, (component + 1) % 3, idx, 2, depth + 1);
	}
};

#endif // !AABB_TREE_H
