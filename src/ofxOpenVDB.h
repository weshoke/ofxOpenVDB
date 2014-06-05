#pragma once
#include <openvdb/openvdb.h>
#include <openvdb/tools/Composite.h>
#include <openvdb/tools/GridOperators.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/ValueTransformer.h>
#include <openvdb/tools/VolumeToMesh.h>
#include "ofxOpenVDBRayIntersector64.h"

using namespace openvdb;


namespace ofxOpenVDB {

template <typename Grid>
void drawCube(const Grid& grid, const Coord &minCoord, const Coord &maxCoord) {
	Vec3d world000 = grid.indexToWorld(minCoord);
	Vec3d world100 = grid.indexToWorld(Coord(maxCoord[0]+1, minCoord[1], minCoord[2]));
	Vec3d world010 = grid.indexToWorld(Coord(minCoord[0], maxCoord[1]+1, minCoord[2]));
	Vec3d world110 = grid.indexToWorld(Coord(maxCoord[0]+1, maxCoord[1]+1, minCoord[2]));
	Vec3d world001 = grid.indexToWorld(Coord(minCoord[0], minCoord[1], maxCoord[2]+1));
	Vec3d world101 = grid.indexToWorld(Coord(maxCoord[0]+1, minCoord[1], maxCoord[2]+1));
	Vec3d world011 = grid.indexToWorld(Coord(minCoord[0], maxCoord[1]+1, maxCoord[2]+1));
	Vec3d world111 = grid.indexToWorld(maxCoord+Coord(1, 1, 1));
		
	glVertex3dv(world000.asPointer());
	glVertex3dv(world100.asPointer());
	
	glVertex3dv(world100.asPointer());
	glVertex3dv(world110.asPointer());
	
	glVertex3dv(world110.asPointer());
	glVertex3dv(world010.asPointer());
	
	glVertex3dv(world010.asPointer());
	glVertex3dv(world000.asPointer());
	
	
	glVertex3dv(world001.asPointer());
	glVertex3dv(world101.asPointer());
	
	glVertex3dv(world101.asPointer());
	glVertex3dv(world111.asPointer());
	
	glVertex3dv(world111.asPointer());
	glVertex3dv(world011.asPointer());
	
	glVertex3dv(world011.asPointer());
	glVertex3dv(world001.asPointer());
	
	
	glVertex3dv(world000.asPointer());
	glVertex3dv(world001.asPointer());
	
	glVertex3dv(world100.asPointer());
	glVertex3dv(world101.asPointer());
	
	glVertex3dv(world110.asPointer());
	glVertex3dv(world111.asPointer());
	
	glVertex3dv(world010.asPointer());
	glVertex3dv(world011.asPointer());
}

template <typename Grid>
void drawVoxel(const Grid& grid, Coord c) {
	const typename Grid::TreeType::LeafNodeType *node = grid.getConstAccessor().probeConstLeaf(c);
	if(node) {
		glBegin(GL_LINES);
			drawCube(grid, c, c);
		glEnd();
	}
}

template <typename Grid>
void drawGridHiearchy(const Grid& grid) {
	static ofVec4f colors[] = {
		ofVec4f(0., 142./255., 87./255., 0.7),
		ofVec4f(1., 144./255., 54./255., 0.7),
		ofVec4f(0., 152./255., 246./255., 0.7),
		ofVec4f(123./255., 8./255., 7./255., 0.6)
	};
	
	glBegin(GL_LINES);
	for(typename Grid::TreeType::NodeCIter iter = grid.tree().cbeginNode(); iter.test(); ++iter) {
		CoordBBox bbox;
		if(!iter.getBoundingBox(bbox)) continue;
		
		Index depth = iter.getDepth();
		glColor4fv(colors[depth].getPtr());
		
		const Coord &minCoord = bbox.min();
		const Coord &maxCoord = bbox.max();
		drawCube(grid, minCoord, maxCoord);
	}
	
	glColor4f(0, 0, 0, 1);
	for(typename Grid::TreeType::ValueOnCIter iter = grid.tree().cbeginValueOn(); iter.test(); ++iter) {
		CoordBBox bbox;
		if(!iter.getBoundingBox(bbox)) continue;
		
		const Coord &minCoord = bbox.min();
		const Coord &maxCoord = bbox.max();
		drawCube(grid, minCoord, maxCoord);
	}
	glEnd();
}

}