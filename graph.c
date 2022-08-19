/*
graph.c

Set of vertices and edges implementation.

Implementations for helper functions for graph construction and manipulation.

Skeleton written by Grady Fitzpatrick for COMP20007 Assignment 1 2022
*/
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "graph.h"
#include "utils.h"
#include "pq.h"

#define INITIALEDGES 32
#define DEFAULTCOST 1
#define HEARTROOMCOST 0

struct edge;

/* Definition of a graph. */
struct graph {
	int numVertices;
	int numEdges;
	int allocedEdges;
	struct edge **edgeList;
};

/* Definition of an edge. */
struct edge {
	int start;
	int end;
	int cost;
};


struct graph *newGraph(int numVertices){
	struct graph *g = (struct graph *) malloc(sizeof(struct graph));
	assert(g);
	/* Initialise edges. */
	g->numVertices = numVertices;
	g->numEdges = 0;
	g->allocedEdges = 0;
	g->edgeList = NULL;
	return g;
}

/* Adds an edge to the given graph. */
void addEdge(struct graph *g, int start, int end, int cost){
	assert(g);
	struct edge *newEdge = NULL;
	/* Check we have enough space for the new edge. */
	if((g->numEdges + 1) > g->allocedEdges){
		if(g->allocedEdges == 0){
			g->allocedEdges = INITIALEDGES;
		} else {
			(g->allocedEdges) *= 2;
		}
		g->edgeList = (struct edge **) realloc(g->edgeList,
			sizeof(struct edge *) * g->allocedEdges);
		assert(g->edgeList);
	}

	/* Create the edge */
	newEdge = (struct edge *) malloc(sizeof(struct edge));
	assert(newEdge);
	newEdge->start = start;
	newEdge->end = end;
	newEdge->cost = cost;

	/* Add the edge to the list of edges. */
	g->edgeList[g->numEdges] = newEdge;
	(g->numEdges)++;
}

/* Returns a new graph which is a deep copy of the given graph (which must be 
	freed with freeGraph when no longer used). */
struct graph *duplicateGraph(struct graph *g){
	struct graph *copyGraph = (struct graph *) malloc(sizeof(struct graph));
	assert(copyGraph);
	copyGraph->numVertices = g->numVertices;
	copyGraph->numEdges = g->numEdges;
	copyGraph->allocedEdges = g->allocedEdges;
	copyGraph->edgeList = (struct edge **) malloc(sizeof(struct edge *) * g->allocedEdges);
	assert(copyGraph->edgeList || copyGraph->numEdges == 0);
	int i;
	/* Copy edge list. */
	for(i = 0; i < g->numEdges; i++){
		struct edge *newEdge = (struct edge *) malloc(sizeof(struct edge));
		assert(newEdge);
		newEdge->start = (g->edgeList)[i]->start;
		newEdge->end = (g->edgeList)[i]->end;
		newEdge->cost = (g->edgeList)[i]->cost;
		(copyGraph->edgeList)[i] = newEdge;
	}
	return copyGraph;
}

/* Frees all memory used by graph. */
void freeGraph(struct graph *g){
	int i;
	for(i = 0; i < g->numEdges; i++){
		free((g->edgeList)[i]);
	}
	if(g->edgeList){
		free(g->edgeList);
	}
	free(g);
}

/* Returns the cost of entering the room based on whether the room
  is a heart room. */
int isHeartRoom(int room, int numHeartRooms, int *heartRooms) {
	int i;

	for (i = 0; i < numHeartRooms; i++) {
		if (heartRooms[i] == room) {
			return HEARTROOMCOST;
		}
	}
	return DEFAULTCOST;
}


/* Finds the shortest path and returns the net hearts lost */
int findBestPath(struct graph *g, enum problemPart part,
	int numRooms, int startingRoom, int bossRoom, 
	int numHeartRooms, int *heartRooms) {

	int i, final_cost, edge_cost = DEFAULTCOST;
	int *dist, *u, *w;
	int **rooms;
	struct pq *new_pq = newPQ();
	
	/* allocate memory accordingly */
	rooms = (int **)malloc(sizeof(int *) * numRooms);
	dist = (int *)malloc(sizeof(int) * numRooms);

	/* initialise rooms, dist and the priority queue*/
	for (i = 0; i < numRooms; i++) {
		/* create a new room which holds the room number */
		int *new_room = (int *)malloc(sizeof(*new_room));
		*new_room = i;

		dist[i] = INT_MAX;
		rooms[i] = new_room;
		enqueue(new_pq, rooms[i], dist[i]);

	}
	
	/* update the priority of starting node to zero */
	dist[startingRoom] = 0;
	update(new_pq, rooms[startingRoom], dist[startingRoom]);

	/* implement dijikstra's algorithm for finding the shortest path*/
	while (!empty(new_pq)) {
		u = deletemin(new_pq);
		
		/* find the set of neighbours of the deleted node */
		for (i = 0; i < g->numEdges; i++) {
			if (g->edgeList[i]->start == *u) {
				w = rooms[g->edgeList[i]->end];
			}
			else if (g->edgeList[i]->end == *u) {
				w = rooms[g->edgeList[i]->start];
			}
			
			/* If the room we are heading to is a heart room, then the edge cost
			   becomes zero */
			if (part == PART_C) {
				edge_cost = isHeartRoom(*w, numHeartRooms, heartRooms);
			}
			
			/* checks if w is in the priority queue */
			if (isElement(new_pq, w) && 
				dist[*u] + edge_cost < dist[*w]) {
				
				dist[*w] = dist[*u] + edge_cost;
				update(new_pq, w, dist[*w]);
			}
		}
	}

	final_cost = dist[bossRoom];

	/* free all allocated memory */
	freePQ(new_pq);
	for (i=0; i < numRooms; i++) {
		free(rooms[i]);
	}
	free(rooms);
	free(dist);

	return final_cost;
}


struct solution *graphSolve(struct graph *g, enum problemPart part,
	int numRooms, int startingRoom, int bossRoom, int numShortcuts, 
	int *shortcutStarts, int *shortcutEnds, int numHeartRooms, int *heartRooms){
	struct solution *solution = (struct solution *)
		malloc(sizeof(struct solution));
	assert(solution);

	int i, curr_shortest, overall_shortest = INT_MAX;

	if(part == PART_A){
		/* IMPLEMENT 2A SOLUTION HERE */
		solution->heartsLost = 
			findBestPath(g, part, numRooms, startingRoom, bossRoom, 
			numHeartRooms, heartRooms);

	} else if(part == PART_B) {
		/* IMPLEMENT 2B SOLUTION HERE */

		/* Try findBestPath function each time, with a different shortcut
		   incorporated into the edge list. */
		for (i = 0; i < numShortcuts; i++) {
			if (i == 0) {
				/* Change the struct to accomadate an extra shortcut
				   for the first time adding an edge. */
				addEdge(g, shortcutStarts[0], shortcutEnds[0], DEFAULTCOST);
			}
			else {
				g->edgeList[g->numEdges - 1]->start = shortcutStarts[i];
				g->edgeList[g->numEdges - 1]->end = shortcutEnds[i];

			}
			curr_shortest = 
				findBestPath(g, part, numRooms, startingRoom, bossRoom, 
				numHeartRooms, heartRooms);

			if (curr_shortest < overall_shortest) {
				overall_shortest = curr_shortest;
			}
				
		}

		/* Return the shortest path which factors in the different
		   shortcuts. */
		solution->heartsLost = overall_shortest;

	} else {
		/* IMPLEMENT 2C SOLUTION HERE */

		/* NOTE: PART C is incorporated in the if statement in the
		   findBestPath function. */
		solution->heartsLost = 
			findBestPath(g, part, numRooms, startingRoom, bossRoom, 
			numHeartRooms, heartRooms);;
	}

	return solution;
}

