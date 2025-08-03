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
		int pos[2]; 
		int fromStart;
		int fromEnd;
		int prev; // index, not pos
	};

	ulong id = 0;
	int graphLen;
	int headsLen;
	searchCell graph[1000];
	searchCell heads[1000];

	int getPathRoad(int* start, int* end, int path[][2])
	{
		// short circuit
		if (start[0] == end[0] && start[1] == end[1])
		{
			path[0][0] = end[0];
			path[0][1] = end[1];
			return 1;
		}

		id += 1;
		graphLen = 0;

		heads[0].fromStart = 0;
		heads[0].fromEnd = abs(start[0] - end[0]) + abs(start[1] - end[1]);
		heads[0].pos[0] = start[0];
		heads[0].pos[1] = start[1];
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
			cell* current = map.at(shortestHead->pos[0], shortestHead->pos[1]);
			current->visited = id;
			memcpy(graph + graphLen, shortestHead, sizeof(searchCell));
			graphLen += 1;
			// check for end
			if (shortestHead->pos[0] == end[0] && shortestHead->pos[1] == end[1])
				break;

			// get neighbors
			cell* left = map.at(shortestHead->pos[0] - 1, shortestHead->pos[1]);
			if (left && left->road && left->visited != id)
			{
				searchCell* h = heads + headsLen;
				h->fromStart = shortestHead->fromStart + 1;
				h->fromEnd = abs(shortestHead->pos[0] - 1 - end[0]) + abs(shortestHead->pos[1] - end[1]);
				h->pos[0] = shortestHead->pos[0] - 1;
				h->pos[1] = shortestHead->pos[1];
				h->prev = graphLen - 1;
				headsLen += 1;
			}
			cell* top = map.at(shortestHead->pos[0], shortestHead->pos[1] + 1);
			if (top && top->road && top->visited != id)
			{
				searchCell* h = heads + headsLen;
				h->fromStart = shortestHead->fromStart + 1;
				h->fromEnd = abs(shortestHead->pos[0] - end[0]) + abs(shortestHead->pos[1] - end[1] + 1);
				h->pos[0] = shortestHead->pos[0];
				h->pos[1] = shortestHead->pos[1] + 1;
				h->prev = graphLen - 1;
				headsLen += 1;
			}
			cell* right = map.at(shortestHead->pos[0] + 1, shortestHead->pos[1]);
			if (right && right->road && right->visited != id)
			{
				searchCell* h = heads + headsLen;
				h->fromStart = shortestHead->fromStart + 1;
				h->fromEnd = abs(shortestHead->pos[0] + 1 - end[0]) + abs(shortestHead->pos[1] - end[1]);
				h->pos[0] = shortestHead->pos[0] + 1;
				h->pos[1] = shortestHead->pos[1];
				h->prev = graphLen - 1;
				headsLen += 1;
			}
			cell* bottom = map.at(shortestHead->pos[0], shortestHead->pos[1] - 1);
			if (bottom && bottom->road && bottom->visited != id)
			{
				searchCell* h = heads + headsLen;
				h->fromStart = shortestHead->fromStart + 1;
				h->fromEnd = abs(shortestHead->pos[0] + 1 - end[0]) + abs(shortestHead->pos[1] - end[1]);
				h->pos[0] = shortestHead->pos[0];
				h->pos[1] = shortestHead->pos[1] - 1;
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
		path[0][0] = end[0];
		path[0][1] = end[1];
		uint it2 = 1;
		while (it != graph)
		{
			prevprev = prev;
			prev = it;
			it = graph + it->prev;
			// record only turns
			if (prevprev && prevprev->pos[0] != it->pos[0] && prevprev->pos[1] != it->pos[1])
			{
				path[it2][0] = prev->pos[0];
				path[it2][1] = prev->pos[1];
				it2 += 1;
			}
			massert(it2 < MAX_PATH, "takes too long");
		}

		return it2;
	}
}