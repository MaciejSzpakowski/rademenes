#include "types.h"
#include "building.h"
#include "body.h"
#include "map.h"
#include "gl.h"
#include <cstdio>
#include <cstring>
#include "stream.h"
#include <cmath>

namespace ph
{
	struct searchCell
	{
		int x, y;
		int fromStart;
		int fromEnd;
		int prev; // index, not pos
	};

	ulong id = 0;
	int graphLen;
	int headsLen;
	searchCell graph[1000];
	searchCell heads[1000];

	int getPathRoad(vec2i* start, vec2i* end, vec2i* path)
	{
		// short circuit
		if (start->x == end->x && start->y == end->y)
		{
			path[0].x = end->x;
			path[0].y = end->y;
			return 1;
		}

		id += 1;
		graphLen = 0;

		heads[0].fromStart = 0;
		heads[0].fromEnd = abs(start->x - end->x) + abs(start->y - end->y);
		heads[0].x = start->x;
		heads[0].y = start->y;
		headsLen = 1;

		while (true)
		{
			// no heads means we cannot reach
			if (headsLen == 0) return 0;

			// find shortest head
			searchCell* shortestHead = heads;
			uint shortestHeadIndex = 0;
			for (uint i = 1; i < headsLen; i++)
			{
				if (heads[i].fromStart + heads[i].fromEnd < shortestHead->fromStart + shortestHead->fromEnd)
				{
					shortestHead = heads + i;
					shortestHeadIndex = i;
				}
			}

			// add to graph
			cell* current = map.at(shortestHead->x, shortestHead->y);
			current->visited = id;
			memcpy(graph + graphLen, shortestHead, sizeof(searchCell));
			graphLen += 1;
			// check for end
			if (shortestHead->x == end->x && shortestHead->y == end->y)
				break;

			// get neighbors
			cell* left = map.at(shortestHead->x - 1, shortestHead->y);
			if (left && left->road && left->visited != id)
			{
				searchCell* h = heads + headsLen;
				h->fromStart = shortestHead->fromStart + 1;
				h->fromEnd = abs(shortestHead->x - 1 - end->x) + abs(shortestHead->y - end->y);
				h->x = shortestHead->x - 1;
				h->y = shortestHead->y;
				h->prev = graphLen - 1;
				headsLen += 1;
			}
			cell* top = map.at(shortestHead->x, shortestHead->y + 1);
			if (top && top->road && top->visited != id)
			{
				searchCell* h = heads + headsLen;
				h->fromStart = shortestHead->fromStart + 1;
				h->fromEnd = abs(shortestHead->x - end->x) + abs(shortestHead->y - end->y + 1);
				h->x = shortestHead->x;
				h->y = shortestHead->y + 1;
				h->prev = graphLen - 1;
				headsLen += 1;
			}
			cell* right = map.at(shortestHead->x + 1, shortestHead->y);
			if (right && right->road && right->visited != id)
			{
				searchCell* h = heads + headsLen;
				h->fromStart = shortestHead->fromStart + 1;
				h->fromEnd = abs(shortestHead->x + 1 - end->x) + abs(shortestHead->y - end->y);
				h->x = shortestHead->x + 1;
				h->y = shortestHead->y;
				h->prev = graphLen - 1;
				headsLen += 1;
			}
			cell* bottom = map.at(shortestHead->x, shortestHead->y - 1);
			if (bottom && bottom->road && bottom->visited != id)
			{
				searchCell* h = heads + headsLen;
				h->fromStart = shortestHead->fromStart + 1;
				h->fromEnd = abs(shortestHead->x + 1 - end->x) + abs(shortestHead->y - end->y);
				h->x = shortestHead->x;
				h->y = shortestHead->y - 1;
				h->prev = graphLen - 1;
				headsLen += 1;
			}

			// remove from heads
			if (shortestHeadIndex != headsLen - 1)
			{
				// fast remove
				memcpy(shortestHead, heads + headsLen - 1, sizeof(searchCell));
			}
			headsLen -= 1;
		}

		searchCell* it = graph + graphLen - 1;
		searchCell* prev = nullptr;
		searchCell* prevprev = nullptr;
		path[0].x = end->x;
		path[0].y = end->y;
		uint it2 = 1;
		while (it != graph)
		{
			prevprev = prev;
			prev = it;
			it = graph + it->prev;
			// record only turns
			if (prevprev && prevprev->x != it->x && prevprev->y != it->y)
			{
				path[it2].x = prev->x;
				path[it2].y = prev->y;
				it2 += 1;
			}
			massert(it2 < MAX_PATH, "takes too long");
		}

		return it2;
	}
}