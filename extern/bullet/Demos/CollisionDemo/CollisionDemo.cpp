/*
 * Copyright (c) 2005 Erwin Coumans <www.erwincoumans.com>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies.
 * Erwin Coumans makes no representations about the suitability 
 * of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 */


///
/// Collision Demo shows a degenerate case, where the Simplex solver has to deal with near-affine dependent cases
/// See the define CATCH_DEGENERATE_TETRAHEDRON in Bullet's VoronoiSimplexSolver.cpp
///

#include "GL_Simplex1to4.h"
#include "SimdQuaternion.h"
#include "SimdTransform.h"
#include "NarrowPhaseCollision/VoronoiSimplexSolver.h"
#include "CollisionShapes/BoxShape.h"

#include "NarrowPhaseCollision/GjkPairDetector.h"
#include "NarrowPhaseCollision/PointCollector.h"
#include "NarrowPhaseCollision/VoronoiSimplexSolver.h"
#include "NarrowPhaseCollision/ConvexPenetrationDepthSolver.h"

#include "GL_ShapeDrawer.h"
#include <GL/glut.h>
#include "GlutStuff.h"


float yaw=0.f,pitch=0.f,roll=0.f;
const int maxNumObjects = 4;
const int numObjects = 2;

GL_Simplex1to4 simplex;

PolyhedralConvexShape*	shapePtr[maxNumObjects];

SimdTransform tr[numObjects];
int screenWidth = 640.f;
int screenHeight = 480.f;

void DrawRasterizerLine(float const* , float const*, int)
{

}

int main(int argc,char** argv)
{
	setCameraDistance(20.f);

	tr[0].setOrigin(SimdVector3(0.0013328250f,8.1363249f,7.0390840f));
	tr[1].setOrigin(SimdVector3(0.00000000f,9.1262732f,2.0343180f));

	//tr[0].setOrigin(SimdVector3(0,0,0));
	//tr[1].setOrigin(SimdVector3(0,10,0));

	SimdMatrix3x3 basisA;
	basisA.setValue(0.99999958f,0.00022980258f,0.00090992288f,
		-0.00029313788f,0.99753088f,0.070228584f,
		-0.00089153741f,-0.070228823f,0.99753052f);

	SimdMatrix3x3 basisB;
	basisB.setValue(1.0000000f,4.4865553e-018f,-4.4410586e-017f,
		4.4865495e-018f,0.97979438f,0.20000751f,
		4.4410586e-017f,-0.20000751f,0.97979438f);

	tr[0].setBasis(basisA);
	tr[1].setBasis(basisB);



	SimdVector3 boxHalfExtentsA(1.0000004768371582f,1.0000004768371582f,1.0000001192092896f);
	SimdVector3 boxHalfExtentsB(3.2836332321166992f,3.2836332321166992f,3.2836320400238037f);

	BoxShape	boxA(boxHalfExtentsA);
	BoxShape	boxB(boxHalfExtentsB);
	shapePtr[0] = &boxA;
	shapePtr[1] = &boxB;
	

	SimdTransform tr;
	tr.setIdentity();


	return glutmain(argc, argv,screenWidth,screenHeight,"Collision Demo");
}

//to be implemented by the demo

void clientMoveAndDisplay()
{
	
	clientDisplay();
}


static VoronoiSimplexSolver sGjkSimplexSolver;
SimplexSolverInterface& gGjkSimplexSolver = sGjkSimplexSolver;



void clientDisplay(void) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glDisable(GL_LIGHTING);

	//GL_ShapeDrawer::DrawCoordSystem();

	float m[16];
	int i;

	GjkPairDetector	convexConvex(shapePtr[0],shapePtr[1],&sGjkSimplexSolver,0);

	SimdVector3 seperatingAxis(0.00000000f,0.059727669f,0.29259586f);
	convexConvex.SetCachedSeperatingAxis(seperatingAxis);

	PointCollector gjkOutput;
	GjkPairDetector::ClosestPointInput input;
	input.m_transformA = tr[0];
	input.m_transformB = tr[1];

	convexConvex.GetClosestPoints(input ,gjkOutput);

	if (gjkOutput.m_hasResult)
	{
		SimdVector3 endPt = gjkOutput.m_pointInWorld +
			gjkOutput.m_normalOnBInWorld*gjkOutput.m_distance;

		 glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3d(gjkOutput.m_pointInWorld.x(), gjkOutput.m_pointInWorld.y(),gjkOutput.m_pointInWorld.z());
		glVertex3d(endPt.x(),endPt.y(),endPt.z());
		//glVertex3d(gjkOutputm_pointInWorld.x(), gjkOutputm_pointInWorld.y(),gjkOutputm_pointInWorld.z());
		//glVertex3d(gjkOutputm_pointInWorld.x(), gjkOutputm_pointInWorld.y(),gjkOutputm_pointInWorld.z());
		glEnd();

	}

	for (i=0;i<numObjects;i++)
	{
		
		tr[i].getOpenGLMatrix( m );

		GL_ShapeDrawer::DrawOpenGL(m,shapePtr[i],SimdVector3(1,1,1));


	}

	simplex.SetSimplexSolver(&sGjkSimplexSolver);
	SimdPoint3 ybuf[4],pbuf[4],qbuf[4];
	int numpoints = sGjkSimplexSolver.getSimplex(pbuf,qbuf,ybuf);
	simplex.Reset();
	
	for (i=0;i<numpoints;i++)
		simplex.AddVertex(ybuf[i]);

	SimdTransform ident;
	ident.setIdentity();
	ident.getOpenGLMatrix(m);
	GL_ShapeDrawer::DrawOpenGL(m,&simplex,SimdVector3(1,1,1));


	SimdQuaternion orn;
	orn.setEuler(yaw,pitch,roll);
	tr[0].setRotation(orn);

//	pitch += 0.005f;
//	yaw += 0.01f;

	glFlush();
    glutSwapBuffers();
}





