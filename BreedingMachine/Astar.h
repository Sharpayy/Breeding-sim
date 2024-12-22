#pragma once
#include <unordered_set>
#include <queue>
#include <unordered_map>
#include <vector>

#define SQRT2 sqrt(2)

class Astar {
public:

	struct point {
		int x, y;

		bool operator==(const point& other) const;

		bool operator<(const point& other) const;

	};

	struct border {
		int x, y, width, height;

	};

private:

	struct queueData {
		float f;
		point p;
	};

	struct compareQueueData {
		bool operator()(queueData const& p1, queueData const& p2);
		
	};

	struct tileData {
		float x, y, z;
		point parrent;
	};

public:
	Astar(border* border = nullptr);
	~Astar();
	
	void addBlockade(point p);

	std::vector<point> findPath(point s, point e, int offset = 1);

	void clearCollisionBlocks();

private:

	double ManhattanDistance(point s, point e);

	float DiagonalDistance(point s, point e, float D1);

	float EuclideanDistance(point s, point e);

	bool tileIsBlocked(point p);

	bool tileExist(point p);

	void calculateTilesWeight(point p, point s, point e, std::priority_queue<queueData, std::vector<queueData>, compareQueueData>& lCostQueue, int offset = 1);

	point findNewLowestPoint(std::priority_queue<queueData, std::vector<queueData>, compareQueueData>& lCostQueue);

	struct pointHash {
		std::size_t operator()(const point& p) const;
	};

	border* b;
	std::unordered_map<point, tileData, pointHash> tiles;
	std::unordered_set<point, pointHash> collisionBlocks;

};

