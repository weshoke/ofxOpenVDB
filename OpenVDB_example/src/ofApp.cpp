#include "ofApp.h"
#include "ofxOpenVDB.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofEnableAntiAliasing();
	camera.setupPerspective(false, 60, 0.1, 10000);
	camera.setPosition(0, 0, -10);
	camera.lookAt(ofVec3f(0, 0, 0));
	camera.setAutoDistance(false);
	
	floatGrid = openvdb::tools::createLevelSetSphere<FloatGrid>(5, Vec3s(0, 0, 0), 0.05, openvdb::OPENVDB_VERSION_NAME::LEVEL_SET_HALF_WIDTH);
	FloatGrid::Ptr grid2 = openvdb::tools::createLevelSetSphere<FloatGrid>(3, Vec3s(5, 0, 0), 0.05, openvdb::OPENVDB_VERSION_NAME::LEVEL_SET_HALF_WIDTH);
	
	//Vec3IGrid::Ptr copyOfIn1 = floatGrid->deepCopy(),
	openvdb::tools::csgDifference(*floatGrid, *grid2);
	
	std::vector<Vec3s> points;
	std::vector<Vec4I> quads;
	openvdb::tools::volumeToMesh(*floatGrid, points, quads, 0.);
	
	std::vector<ofIndexType> indices;
	indices.reserve(quads.size()*6);
	
	std::vector<ofIndexType> edgeIndices;
	edgeIndices.reserve(quads.size()*8);
	for(int i=0; i < quads.size(); ++i) {
		const Vec4I& q = quads[i];
		indices.push_back(q[0]);
		indices.push_back(q[1]);
		indices.push_back(q[3]);
		
		indices.push_back(q[3]);
		indices.push_back(q[1]);
		indices.push_back(q[2]);
		
		edgeIndices.push_back(q[0]);
		edgeIndices.push_back(q[1]);
		edgeIndices.push_back(q[1]);
		edgeIndices.push_back(q[2]);
		edgeIndices.push_back(q[2]);
		edgeIndices.push_back(q[3]);
		edgeIndices.push_back(q[3]);
		edgeIndices.push_back(q[0]);
	}
	
	
	mesh.setVertexData((ofVec3f *)(points[0].asPointer()), points.size(), GL_DYNAMIC_DRAW);
	mesh.setIndexData(&indices[0], indices.size(), GL_DYNAMIC_DRAW);
	
	edgeMesh.setVertexData((ofVec3f *)(points[0].asPointer()), points.size(), GL_DYNAMIC_DRAW);
	edgeMesh.setIndexData(&edgeIndices[0], edgeIndices.size(), GL_DYNAMIC_DRAW);
}

ofVec4f colors[] = {
	ofVec4f(0., 142./255., 87./255., 0.7),
	ofVec4f(1., 144./255., 54./255., 0.7),
	ofVec4f(0., 152./255., 246./255., 0.7),
	ofVec4f(123./255., 8./255., 7./255., 0.6)
};


void drawGrid(const FloatGrid::Ptr& grid) {
	glBegin(GL_LINES);
	
	//FloatGrid::ValueOnCIter
	//Int64Grid::ValueOnCIter iter = grid.cbeginValueOn();
	//for (FloatGrid::ValueOnCIter iter = grid->cbeginValueOn(); iter.test(); ++iter) {
	for(FloatGrid::TreeType::NodeCIter iter = grid->tree().cbeginNode(); iter.test(); ++iter) {
		/*
		if(!iter.isVoxelValue()) {
			std::cout << "non voxel\n";
		}
	
		if(!iter.isVoxelValue()) continue;
		*/
		CoordBBox bbox;
		if(!iter.getBoundingBox(bbox)) continue;
		
		const Coord &minCoord = bbox.min();
		const Coord &maxCoord = bbox.max();
		Vec3d world000 = grid->indexToWorld(minCoord);
		Vec3d world100 = grid->indexToWorld(Coord(maxCoord[0], minCoord[1], minCoord[2]));
		Vec3d world010 = grid->indexToWorld(Coord(minCoord[0], maxCoord[1], minCoord[2]));
		Vec3d world110 = grid->indexToWorld(Coord(maxCoord[0], maxCoord[1], minCoord[2]));
		Vec3d world001 = grid->indexToWorld(Coord(minCoord[0], minCoord[1], maxCoord[2]));
		Vec3d world101 = grid->indexToWorld(Coord(maxCoord[0], minCoord[1], maxCoord[2]));
		Vec3d world011 = grid->indexToWorld(Coord(minCoord[0], maxCoord[1], maxCoord[2]));
		Vec3d world111 = grid->indexToWorld(maxCoord);
		
		
		Index depth = iter.getDepth();
		glColor4fv(colors[depth].getPtr());
		
	
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
	glEnd();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	camera.begin();
	//glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	
	glEnable(GL_DEPTH_TEST);
	glColor4f(0.5, 0.5, 0.5, 1.);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1,1);
	mesh.drawElements(GL_TRIANGLES, mesh.getNumIndices());
	glDisable(GL_POLYGON_OFFSET_FILL);
	
	glColor4f(0., 0., 0., 0.6);
	edgeMesh.drawElements(GL_LINES, edgeMesh.getNumIndices());
	
	drawGrid(floatGrid);
	
	camera.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
