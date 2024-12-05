#include "Astar.h"

#define SQRT2 sqrt(2)

bool Astar::point::operator==(const point& other) const {
	return x == other.x && y == other.y;
}

bool Astar::compareQueueData::operator()(queueData const& p1, queueData const& p2) {
	return p1.f > p2.f;
}

Astar::Astar(border* border) {
	b = border;
	tiles = {};
	collisionBlocks = {};
}

Astar::~Astar() {
	clearCollisionBlocks();
}

void Astar::addBlockade(point p) {
	collisionBlocks.insert(p);
}

std::vector<Astar::point> Astar::findPath(point s, point e, int offset) {
	if (tileIsBlocked(s) || tileIsBlocked(e)) return {};
	std::vector<point> result;

	point c = s;
	std::priority_queue<queueData, std::vector<queueData>, compareQueueData> lCostQueue;
	tiles[c] = { 0, (float)DiagonalDistance(c, e, 1) };
	lCostQueue.push({ tiles[c].y, c});

	while (!(c == e)) {
		c = findNewLowestPoint(lCostQueue);
		calculateTilesWeight(c, s, e, lCostQueue, offset);
	}

	result.push_back(e);
	while (!(c == s)) {
		result.push_back(tiles[c].parrent);
		c = tiles[c].parrent;
	}

	tiles.clear();
	std::reverse(result.begin(), result.end());	
	return result;

}
void Astar::clearCollisionBlocks() {
	collisionBlocks.clear();
}

double Astar::ManhattanDistance(point s, point e) {
	return std::abs(s.x - e.x) + std::abs(s.y - e.y);
}

float Astar::DiagonalDistance(point s, point e, float D1 = 1) {
	float dx, dy;
	dx = std::fabs(s.x - e.x);
	dy = std::fabs(s.y - e.y);
	return D1 * (dx + dy) + (SQRT2 - 2 * D1) * std::min(dx, dy);
}

float Astar::EuclideanDistance(point s, point e) {
	return std::sqrtf(powf((s.x - e.x), 2) + powf((s.y - e.y), 2));
}

bool Astar::tileIsBlocked(point p) {
	if (b) {;
		if ((p.x <= b->x || p.x >= b->x + b->width) 
			&& (p.y <= b->y || p.y >= b->height - b->y)) {
			return true;
		}
	}
	return collisionBlocks.find(p) != collisionBlocks.end();
}

bool Astar::tileExist(point p) {
	return tiles.find(p) != tiles.end();
}

//, float (*operation)(point, point) = EuclideanDistance
void Astar::calculateTilesWeight(point p, point s, point e, std::priority_queue<queueData, std::vector<queueData>, compareQueueData>& lCostQueue, int offset) {
	int y, x;
	float cs, ce, cc, f;
	point c;
	for (y = p.y - offset ; y <= p.y + offset; y+=offset) {
		for (x = p.x - offset; x <= p.x + offset; x+=offset) {
			c = point{ x, y };
			if (c == p || tileIsBlocked(c)) continue;
			cs = tiles[p].x;
			cc = DiagonalDistance(c, p);
			ce = DiagonalDistance(c, e);

			if (!tileExist(c)) {
				tiles[c].x = cs + cc;//cs + tiles[p].x;
				tiles[c].y = tiles[c].x + ce;
				tiles[c].parrent = p;
				lCostQueue.push(queueData{ tiles[c].x + ce , c });
			}

			//ADD THIS FOR BETTER PATHS
			else if (tiles.at(p).x + cc < tiles.at(c).x) {
				tiles.at(c).x = tiles.at(p).x + cc;
				tiles.at(c).y = tiles.at(c).x + ce;
				tiles.at(c).parrent = p;

				lCostQueue.push({ tiles.at(c).y , c });
			}
		}
	}
}

Astar::point Astar::findNewLowestPoint(std::priority_queue<queueData, std::vector<queueData>, compareQueueData>& lCostQueue) {
	point lp;
	lp = lCostQueue.top().p;
	lCostQueue.pop();
	return lp;
}

std::size_t Astar::pointHash::operator()(const point& p) const {
	int A = (int)(p.x >= 0 ? 2 * p.x : -2 * p.x - 1);
	int B = (int)(p.y >= 0 ? 2 * p.y : -2 * p.y - 1);
	int C = (int)((A >= B ? A * A + A + B : A + B * B) / 2);
	return p.x < 0 && p.y < 0 || p.x >= 0 && p.y >= 0 ? C : -C - 1;
	//return std::hash<int>()(p.x) << 2 ^ (std::hash<int>()(p.y) << 1);
}


