#include "Renderables.h"

void light_draw_box_lines(const Vector3& origin, const Vector3 points[8]) {
	//draw lines from the center of the bbox to the corners
	glBegin(GL_LINES);

	glVertex3dv(origin);
	glVertex3dv(points[1]);

	glVertex3dv(origin);
	glVertex3dv(points[5]);

	glVertex3dv(origin);
	glVertex3dv(points[2]);

	glVertex3dv(origin);
	glVertex3dv(points[6]);

	glVertex3dv(origin);
	glVertex3dv(points[0]);

	glVertex3dv(origin);
	glVertex3dv(points[4]);

	glVertex3dv(origin);
	glVertex3dv(points[3]);

	glVertex3dv(origin);
	glVertex3dv(points[7]);

	glEnd();
}

namespace entity
{

void RenderLightRadiiBox::render(RenderStateFlags state) const {
	//draw the bounding box of light based on light_radius key
	aabb_draw_wire(m_points);

  #if 1    //disable if you dont want lines going from the center of the light bbox to the corners
	light_draw_box_lines(m_origin, m_points);
  #endif
}

RenderLightProjection::RenderLightProjection(const Vector3& origin, const Matrix4& projection, const Frustum& frustum)
  :	_origin(origin),
  	_projection(projection),
	_frustum(frustum)
{
}

void RenderLightProjection::render(RenderStateFlags state) const {
#if 0
	// greebo: These four define the base area and are always needed to draw the light
	// Note the minus sign before intersectPlanes (the points have to be mirrored against the origin)
	Vector3 bottomUpRight = -Plane3::intersect(_frustum.left, _frustum.top, _frustum.back);
	Vector3 bottomDownRight = -Plane3::intersect(_frustum.left, _frustum.bottom, _frustum.back);
	Vector3 bottomUpLeft = -Plane3::intersect(_frustum.right, _frustum.top, _frustum.back);
	Vector3 bottomDownLeft = -Plane3::intersect(_frustum.right, _frustum.bottom, _frustum.back);

	// The planes of the frustum are measured at world 0,0,0 so we have to position the intersection points relative to the light origin
	bottomUpRight += _origin;
	bottomDownRight += _origin;
	bottomUpLeft += _origin;
	bottomDownLeft += _origin;

	if (_start != Vector3(0,0,0)) {
		// Calculate the vertices defining the top area
		// Again, note the minus sign
		Vector3 topUpRight = -Plane3::intersect(_frustum.left, _frustum.top, _frustum.front);
		Vector3 topDownRight = -Plane3::intersect(_frustum.left, _frustum.bottom, _frustum.front);
		Vector3 topUpLeft = -Plane3::intersect(_frustum.right, _frustum.top, _frustum.front);
		Vector3 topDownLeft = -Plane3::intersect(_frustum.right, _frustum.bottom, _frustum.front);

		topUpRight += _origin;
		topDownRight += _origin;
		topUpLeft += _origin;
		topDownLeft += _origin;

		Vector3 frustum[8] = { topUpRight, topDownRight, topDownLeft, topUpLeft,
							   bottomUpRight, bottomDownRight, bottomDownLeft, bottomUpLeft };
		drawFrustum(frustum);
	}
	else {
		// no light_start, just use the top vertex (doesn't need to be mirrored)
		Vector3 top = Plane3::intersect(_frustum.left, _frustum.right, _frustum.top);
		top += _origin;

		Vector3 pyramid[5] = { top, bottomUpRight, bottomDownRight, bottomDownLeft, bottomUpLeft };
		drawPyramid(pyramid);
	}
#else
	Matrix4 unproject(matrix4_full_inverse(_projection));
	Vector3 points[8];
	aabb_corners(AABB(Vector3(0.5f, 0.5f, 0.5f), Vector3(0.5f, 0.5f, 0.5f)), points);
	points[0] = matrix4_transformed_vector4(unproject, Vector4(points[0], 1)).getProjected();
	points[1] = matrix4_transformed_vector4(unproject, Vector4(points[1], 1)).getProjected();
	points[2] = matrix4_transformed_vector4(unproject, Vector4(points[2], 1)).getProjected();
	points[3] = matrix4_transformed_vector4(unproject, Vector4(points[3], 1)).getProjected();
	points[4] = matrix4_transformed_vector4(unproject, Vector4(points[4], 1)).getProjected();
	points[5] = matrix4_transformed_vector4(unproject, Vector4(points[5], 1)).getProjected();
	points[6] = matrix4_transformed_vector4(unproject, Vector4(points[6], 1)).getProjected();
	points[7] = matrix4_transformed_vector4(unproject, Vector4(points[7], 1)).getProjected();
	//Vector4 test1 = matrix4_transformed_vector4(unproject, Vector4(0.5f, 0.5f, 0.5f, 1));
	//Vector3 test2 = vector4_projected(test1);
	aabb_draw_wire(points);
#endif
}

} // namespace entity
