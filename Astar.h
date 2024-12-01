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
		float x, y;
		point parrent;
	};

public:

	Astar();
	~Astar();
	
	void addBlockade(point p);

	std::vector<point> findPath(point s, point e, int offset = 1);

	void clearCollisionBlocks();


	struct rect {
		int x, y, width, height;

		bool intersects(const point& p) const {
			int left = std::min(x, x + width);
			int right = std::max(x, x + width);
			int top = std::min(y, y + height);
			int bottom = std::max(y, y + height);

			return (p.x >= left && p.x <= right && p.y >= top && p.y <= bottom);
		}
	};
private:

	double ManhattanDistance(point s, point e);

	float DiagonalDistance(point s, point e, float D1);

	float EuclideanDistance(point s, point e);

	bool tileIsBlocked(point p);

	bool tileExist(point p);

	void calculateTilesWeight(point p, point e, std::priority_queue<queueData, std::vector<queueData>, compareQueueData>& lCostQueue, int offset = 1);

	point findNewLowestPoint(std::priority_queue<queueData, std::vector<queueData>, compareQueueData>& lCostQueue);

	struct pointHash {
		std::size_t operator()(const point& p) const;
	};

	std::unordered_map<point, tileData, pointHash> tiles;
	std::unordered_set<point, pointHash> collisionBlocks;

};

