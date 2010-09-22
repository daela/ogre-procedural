/*
-----------------------------------------------------------------------------
This source file is part of ogre-procedural

For the latest info, see http://code.google.com/p/ogre-procedural/

Copyright (c) 2010 Michael Broutin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "ProceduralRoundedBoxGenerator.h"
#include "ProceduralPlaneGenerator.h"
#include "ProceduralUtils.h"

namespace Procedural
{

void RoundedBoxGenerator::_addCorner(Ogre::ManualObject* manual, int& offset, bool isXPositive, bool isYPositive, bool isZPositive)
{
	Ogre::Vector3 offsetPosition((isXPositive?1:-1)*sizeX, (isYPositive?1:-1)*sizeY, (isZPositive?1:-1)*sizeZ);
	Ogre::Real deltaRingAngle = (Ogre::Math::HALF_PI / chamferNumSeg);
	Ogre::Real deltaSegAngle = (Ogre::Math::HALF_PI / chamferNumSeg);
	Ogre::Real offsetRingAngle = isYPositive?0:-Ogre::Math::HALF_PI;
	Ogre::Real offsetSegAngle = isXPositive?0:Ogre::Math::PI + isZPositive?0:Ogre::Math::HALF_PI;

	// Generate the group of rings for the sphere
	for(unsigned int ring = 0; ring <= chamferNumSeg; ring++ ) {
		Ogre::Real r0 = chamferSize * sinf (ring * deltaRingAngle + offsetRingAngle);
		Ogre::Real y0 = chamferSize * cosf (ring * deltaRingAngle + offsetRingAngle);

		// Generate the group of segments for the current ring
		for(unsigned int seg = 0; seg <= chamferNumSeg; seg++) {
			Ogre::Real x0 = r0 * sinf(seg * deltaSegAngle + offsetSegAngle);
			Ogre::Real z0 = r0 * cosf(seg * deltaSegAngle + offsetSegAngle);

			// Add one vertex to the strip which makes up the sphere
			manual->position( x0 + offsetPosition.x, y0 + offsetPosition.y, z0 + offsetPosition.z);
			if (enableNormals)
				manual->normal(Ogre::Vector3(x0, y0, z0).normalisedCopy());
			for (unsigned int tc=0;tc<numTexCoordSet;tc++)
				manual->textureCoord((Ogre::Real) seg / (Ogre::Real) chamferNumSeg * uTile, (Ogre::Real) ring / (Ogre::Real) chamferNumSeg * vTile);

			if (ring != chamferNumSeg) {
				// each vertex (except the last) has six indices pointing to it
				manual->index(offset + chamferNumSeg + 1);
				manual->index(offset);
				manual->index(offset + chamferNumSeg);
				manual->index(offset + chamferNumSeg + 1);
				manual->index(offset + 1);
				manual->index(offset);
				offset ++;
				}
		}; // end for seg
	} // end for ring
}

/**
 * xPos,yPos,zPos : 1 => positive
                    -1 => negative
					0 => undefined
 */
void RoundedBoxGenerator::_addEdge(Ogre::ManualObject* manual, int& offset, short xPos, short yPos, short zPos)
{
	Ogre::Vector3 offsetPosition = xPos * sizeX * Ogre::Vector3::UNIT_X + yPos * sizeY * Ogre::Vector3::UNIT_Y + zPos * sizeZ * Ogre::Vector3::UNIT_Z;
	Ogre::Vector3 vy0 = (1-abs(xPos)) * Ogre::Vector3::UNIT_X + (1-abs(yPos)) * Ogre::Vector3::UNIT_Y + (1-abs(zPos)) * Ogre::Vector3::UNIT_Z;//extrusion direction
	Ogre::Vector3 vx0 = vy0.perpendicular();
	Ogre::Vector3 vz0 = vx0.crossProduct(vy0);

	float height=1.0;//TODO
	int numSegHeight=1;//TODO

	Ogre::Real deltaAngle = (Ogre::Math::HALF_PI / chamferNumSeg);
	Ogre::Real deltaHeight = height/(Ogre::Real)numSegHeight;

	if (xPos==0)
		numSegHeight = numSegX;
	else if (yPos==0)
		numSegHeight = numSegY;
	else if (zPos==0)
		numSegHeight = numSegZ;
	
	for (int i = 0; i <=numSegHeight; i++)
		for (int j = 0; j<=chamferNumSeg; j++)
		{
			Ogre::Real x0 = chamferSize * cosf(j*deltaAngle);
			Ogre::Real z0 = chamferSize * sinf(j*deltaAngle);
			manual->position(x0 * vx0 + i*deltaHeight * vy0 + z0 * vz0);
			manual->normal((x0*vx0+z0*vz0).normalisedCopy());
			manual->textureCoord(j/(Ogre::Real)chamferNumSeg*uTile, i/(Ogre::Real)numSegHeight*vTile);

			if (i != numSegHeight) {
				manual->index(offset + chamferNumSeg + 1);
				manual->index(offset);
				manual->index(offset + chamferNumSeg);
				manual->index(offset + chamferNumSeg + 1);
				manual->index(offset + 1);
				manual->index(offset);
				}
					offset ++;
		}
}

void RoundedBoxGenerator::addToManualObject(Ogre::ManualObject* manual, int& offset, float& boundingRadius, Ogre::AxisAlignedBox& aabb)
{
	// Generate the pseudo-box shape
	PlaneGenerator pg;
	pg.setUTile(uTile).setVTile(vTile);
	pg.setNumSegX(numSegY).setNumSegY(numSegX).setSizeX(sizeY).setSizeY(sizeX)
	  .setNormal(Ogre::Vector3::NEGATIVE_UNIT_Z)
	  .setPosition((.5*sizeZ+chamferSize)*Ogre::Vector3::NEGATIVE_UNIT_Z)
	  .addToManualObject(manual, offset, boundingRadius, aabb);
    pg.setNumSegX(numSegY).setNumSegY(numSegX).setSizeX(sizeY).setSizeY(sizeX)
	  .setNormal(Ogre::Vector3::UNIT_Z)
	  .setPosition((.5*sizeZ+chamferSize)*Ogre::Vector3::UNIT_Z)
	  .addToManualObject(manual, offset, boundingRadius, aabb);
    pg.setNumSegX(numSegZ).setNumSegY(numSegX).setSizeX(sizeZ).setSizeY(sizeX)
	  .setNormal(Ogre::Vector3::NEGATIVE_UNIT_Y)
	  .setPosition((.5*sizeY+chamferSize)*Ogre::Vector3::NEGATIVE_UNIT_Y)
	  .addToManualObject(manual, offset, boundingRadius, aabb);
    pg.setNumSegX(numSegZ).setNumSegY(numSegX).setSizeX(sizeZ).setSizeY(sizeX)
	  .setNormal(Ogre::Vector3::UNIT_Y)
	  .setPosition((.5*sizeY+chamferSize)*Ogre::Vector3::UNIT_Y)
	  .addToManualObject(manual, offset, boundingRadius, aabb);
    pg.setNumSegX(numSegZ).setNumSegY(numSegY).setSizeX(sizeZ).setSizeY(sizeY)
	  .setNormal(Ogre::Vector3::NEGATIVE_UNIT_X)
	  .setPosition((.5*sizeX+chamferSize)*Ogre::Vector3::NEGATIVE_UNIT_X)
	  .addToManualObject(manual, offset, boundingRadius, aabb);
    pg.setNumSegX(numSegZ).setNumSegY(numSegY).setSizeX(sizeZ).setSizeY(sizeY)
	  .setNormal(Ogre::Vector3::UNIT_X)
	  .setPosition((.5*sizeX+chamferSize)*Ogre::Vector3::UNIT_X)
	  .addToManualObject(manual, offset, boundingRadius, aabb);
	  
	// Generate the corners
	_addCorner(manual, offset, true,  true,  true);
	_addCorner(manual, offset, true,  true,  false);
	_addCorner(manual, offset, true,  false, true);
	_addCorner(manual, offset, true,  false, false);
	_addCorner(manual, offset, false, true,  true);
	_addCorner(manual, offset, false, true,  false);
	_addCorner(manual, offset, false, false, true);
	_addCorner(manual, offset, false, false, false);
			
	// Generate the edges
	
	

    aabb.setExtents(-.5*sizeX, -.5*sizeY, -.5*sizeZ,.5*sizeX, .5*sizeY, .5*sizeZ);
    boundingRadius = Ogre::Math::Sqrt(sizeX*sizeX + sizeY*sizeY + sizeZ*sizeZ);
}
}