#include "SpeakerNode.h"

#include "math/frustum.h"

namespace entity {

SpeakerNode::SpeakerNode(const IEntityClassConstPtr& eclass) :
	EntityNode(eclass),
	TransformModifier(Speaker::TransformChangedCaller(_speaker), ApplyTransformCaller(*this)),
	TargetableNode(_entity, *this),
	_speaker(*this,
		Node::TransformChangedCaller(*this),
		Node::BoundsChangedCaller(*this),
		EvaluateTransformCaller(*this))
{
	TargetableNode::construct();
}

SpeakerNode::SpeakerNode(const SpeakerNode& other) :
	EntityNode(other),
	SelectableNode(other),
	scene::Cloneable(other),
	Nameable(other),
	Snappable(other),
	TransformNode(other),
	SelectionTestable(other),
	Renderable(other),
	Cullable(other),
	Bounded(other),
	TransformModifier(Speaker::TransformChangedCaller(_speaker), ApplyTransformCaller(*this)),
	TargetableNode(_entity, *this),
	_speaker(other._speaker,
		*this,
		Node::TransformChangedCaller(*this),
		Node::BoundsChangedCaller(*this),
		EvaluateTransformCaller(*this))
{
	TargetableNode::construct();
}

SpeakerNode::~SpeakerNode() {
	TargetableNode::destruct();
}

// Snappable implementation
void SpeakerNode::snapto(float snap) {
	_speaker.snapto(snap);
}

// Bounded implementation
const AABB& SpeakerNode::localAABB() const {
	return _speaker.localAABB();
}

// TransformNode implementation
const Matrix4& SpeakerNode::localToParent() const {
	return _speaker.getTransformNode().localToParent();
}

// Cullable implementation
VolumeIntersectionValue SpeakerNode::intersectVolume(
    const VolumeTest& test, const Matrix4& localToWorld) const
{
	return _speaker.intersectVolume(test, localToWorld);
}

// EntityNode implementation
Entity& SpeakerNode::getEntity() {
	return _speaker.getEntity();
}

void SpeakerNode::refreshModel() {
	// Nothing to do
}


void SpeakerNode::testSelect(Selector& selector, SelectionTest& test) {
	_speaker.testSelect(selector, test, localToWorld());
}

scene::INodePtr SpeakerNode::clone() const {
	scene::INodePtr clone(new SpeakerNode(*this));
	clone->setSelf(clone);
	return clone;
}

void SpeakerNode::instantiate(const scene::Path& path) {
	_speaker.instanceAttach(path);
	Node::instantiate(path);
}

void SpeakerNode::uninstantiate(const scene::Path& path) {
	_speaker.instanceDetach(path);
	Node::uninstantiate(path);
}

// Nameable implementation
std::string SpeakerNode::name() const {
	return _speaker.getNameable().name();
}

void SpeakerNode::attach(const NameCallback& callback) {
	_speaker.getNameable().attach(callback);
}

void SpeakerNode::detach(const NameCallback& callback) {
	_speaker.getNameable().detach(callback);
}

/* Renderable implementation */

void SpeakerNode::renderSolid(RenderableCollector& collector, const VolumeTest& volume) const
{
	_speaker.renderSolid(collector, volume, localToWorld(), isSelected());
}
void SpeakerNode::renderWireframe(RenderableCollector& collector, const VolumeTest& volume) const
{
	_speaker.renderWireframe(collector, volume, localToWorld(), isSelected());
}

void SpeakerNode::evaluateTransform()
{
	if (getType() == TRANSFORM_PRIMITIVE)
	{
		_speaker.translate(getTranslation());
		_speaker.rotate(getRotation());
	}
	/*
	else
	{
		// This seems to be a drag operation
		_dragPlanes.m_bounds = _speaker.localAABB();

		// Let the dragplanes helper resize our local AABB
		AABB resizedAABB = _dragPlanes.evaluateResize(getTranslation(), Matrix4::getIdentity());

		// Let the speaker do the rest of the math
		_speaker.setRadiusFromAABB(resizedAABB);
	}
*/
}

void SpeakerNode::applyTransform()
{
	_speaker.revertTransform();
	evaluateTransform();
	_speaker.freezeTransform();
}

} // namespace entity
