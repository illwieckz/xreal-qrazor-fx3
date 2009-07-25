#ifndef SPEAKER_H_
#define SPEAKER_H_

#include "ieclass.h"
#include "Bounded.h"
#include "cullable.h"
#include "editable.h"

#include "math/Vector3.h"
#include "entitylib.h"
#include "generic/callback.h"

#include "../origin.h"
#include "../namedentity.h"
#include "../keyobservers.h"
#include "../Doom3Entity.h"
#include "../EntitySettings.h"

#include "SpeakerRenderables.h"

namespace entity {

class SpeakerNode;

class Speaker :
	public Cullable,
	public Bounded,
	public Snappable
{
	Doom3Entity& m_entity;
	KeyObserverMap m_keyObservers;
	MatrixTransform m_transform;

	OriginKey m_originKey;
	Vector3 m_origin;

	NamedEntity m_named;

	// The current speaker radii (min / max)
	SoundRadii _radii;
	// The "working set" which is used during resize operations
	SoundRadii _radiiTransformed;

	// The default radii as defined on the currently active sound shader
	SoundRadii _defaultRadii;

    // Renderable speaker radii
	RenderableSpeakerRadii _renderableRadii;

	bool m_useSpeakerRadii;
	bool m_minIsSet;
	bool m_maxIsSet;

	AABB m_aabb_local;

	// the AABB that determines the rendering area
	AABB m_aabb_border;

	RenderableSolidAABB m_aabb_solid;
	RenderableWireframeAABB m_aabb_wire;
	RenderableNamedEntity m_renderName;

	Callback m_transformChanged;
	Callback m_boundsChanged;
	Callback m_evaluateTransform;
public:
	// Constructor
	Speaker(entity::SpeakerNode& node,
				  const Callback& transformChanged, 
				  const Callback& boundsChanged,
				  const Callback& evaluateTransform);
	
	// Copy constructor
	Speaker(const Speaker& other, 
				  SpeakerNode& node, 
				  const Callback& transformChanged, 
				  const Callback& boundsChanged,
				  const Callback& evaluateTransform);

	InstanceCounter m_instanceCounter;
	void instanceAttach(const scene::Path& path);
	void instanceDetach(const scene::Path& path);

	Doom3Entity& getEntity();
	const Doom3Entity& getEntity() const;

	//Namespaced& getNamespaced();
	NamedEntity& getNameable();
	const NamedEntity& getNameable() const;
	TransformNode& getTransformNode();
	const TransformNode& getTransformNode() const;

	const AABB& localAABB() const;

	VolumeIntersectionValue intersectVolume(const VolumeTest& volume, const Matrix4& localToWorld) const;

    // Render functions (invoked by SpeakerNode)
	void renderSolid(RenderableCollector& collector,
                     const VolumeTest& volume,
                     const Matrix4& localToWorld,
                     bool isSelected) const;
	void renderWireframe(RenderableCollector& collector,
                         const VolumeTest& volume,
                         const Matrix4& localToWorld,
                         bool isSelected) const;

	void testSelect(Selector& selector, SelectionTest& test, const Matrix4& localToWorld);

	void translate(const Vector3& translation);
	void rotate(const Quaternion& rotation);
	
	void snapto(float snap);
	
	void revertTransform();
	void freezeTransform();
	void transformChanged();
	typedef MemberCaller<Speaker, &Speaker::transformChanged> TransformChangedCaller;

	// greebo: Modifies the speaker radii according to the passed bounding box
	// this is called during drag-resize operations
	void setRadiusFromAABB(const AABB& aabb);

public:

	void construct();

	// updates the AABB according to the SpeakerRadii
	void updateAABB();

	void updateTransform();
	typedef MemberCaller<Speaker, &Speaker::updateTransform> UpdateTransformCaller;

	void originChanged();
	typedef MemberCaller<Speaker, &Speaker::originChanged> OriginChangedCaller;

	void sSoundChanged(const std::string& value);
	typedef MemberCaller1<Speaker, const std::string&, &Speaker::sSoundChanged> sSoundChangedCaller;

	void sMinChanged(const std::string& value);
	typedef MemberCaller1<Speaker, const std::string&, &Speaker::sMinChanged> sMinChangedCaller;

	void sMaxChanged(const std::string& value);
	typedef MemberCaller1<Speaker, const std::string&, &Speaker::sMaxChanged> sMaxChangedCaller;
};

} // namespace entity

#endif /*SPEAKER_H_*/
