#include "GameObject.h"
#include "Scene.h"
#include "CompValues.h"

int GameObject::idCounter = 0;


void GameObject::SetFlags(Flags val) {
	this->flags = val;
}

bool GameObject::HasFlag(unsigned state) const {
	return flags.HasState(state);
}

void GameObject::SetFlag(unsigned state) {
	flags.SetState(state);
}

void GameObject::ResetFlag(unsigned state) {
	flags.ResetState(state);
}

void GameObject::SwitchFlag(unsigned state1, unsigned state2) {
	flags.SwitchState(state1, state2);
}


GameObject* GameObject::GetRoot() const {
	if (parent == nullptr) return const_cast<GameObject*>(this);
	if (parent->parent == nullptr) return parent;

	auto currentNode = parent;

	while (currentNode->parent != nullptr) {
		currentNode = currentNode->parent;
	}

	return currentNode;
}

void GameObject::Remove() {
	GetParent()->RemoveChild(this);
}

void GameObject::AddComponent(Component* component) {
	this->components.push_back(component);
	component->SetOwner(this);
	component->Init(); // initialize component
}

bool GameObject::RemoveComponent(Component* component) {
	auto found = std::find(components.begin(), components.end(), component);

	if (found != components.end()) {
		component->SetOwner(nullptr);
		scene->RemoveSubscriber(component);
		components.erase(found);
		return true;
	}

	return false;
}

void GameObject::RemoveAllComponents() {
	for (auto comp : components) {
		RemoveComponent(comp);
	}

	components.clear();
}

bool GameObject::DestroyComponent(Component* component) {
	bool result = RemoveComponent(component);
	if (result) { delete component; }
	return result;
}

void GameObject::DestroyAllComponents() {
	for (auto comp : components) {
		scene->RemoveSubscriber(comp);
		delete comp;
	}

	components.clear();
}

void GameObject::AddChild(GameObject* child) {
	child->SetParent(this);

	if (isUpdating) {
		childrenToAdd.push_back(child);
	}
	else {
		this->children.push_back(child);
		scene->AddGameObjectInternal(child);
	}
}

bool GameObject::RemoveChild(GameObject* child) {

	auto found = std::find(children.begin(), children.end(), child);

	if (found != children.end()) {
		if (isUpdating) {
			childrenToRemove.push_back(child);
		}
		else {
			child->SetParent(nullptr);
			children.erase(found);
			scene->RemoveGameObjectInternal(child);
		}
		return true;
	}

	return false;
}

bool GameObject::DestroyChild(GameObject* child) {
	bool result = RemoveChild(child);
	if (result) { delete child; }
	return result;
}

void GameObject::RemoveAllChildren() {
	for (auto child : children) {
		RemoveChild(child);
	}
}

void GameObject::DestroyAllChildren() {
	for (auto child : children) {
		scene->RemoveGameObjectInternal(child);
		delete child;
	}
	children.clear();
}

void GameObject::Update(uint64_t delta, uint64_t absolute) {
	isUpdating = true;
	// update components
	for (auto comp : components) {
		comp->Update(delta, absolute);
	}

	// update children
	for (auto child : children) {
		child->Update(delta, absolute);
	}
	isUpdating = false;

	for(auto child : childrenToAdd) {
		AddChild(child);
	}

	for(auto child : childrenToRemove) {
		RemoveChild(child);
	}

	childrenToAdd.clear();
	childrenToRemove.clear();
}

void GameObject::UpdateTransformations() {

	if (parent != nullptr) {
		GetTransform().CalcAbsTransform(parent->GetTransform());
	}else {
		GetTransform().SetAbsAsLocal();
	}

	this->mesh->UpdateBoundingBox();

	for (auto child : children) {
		child->UpdateTransformations();
	}
}


bool GameObject::RemoveAttr(StrId key, bool destroy) {
	auto it = attributes.find(key);

	if (it != attributes.end()) {
		BaseAttribute* attr = it->second;
		attributes.erase(it);
		if (destroy) delete attr;
		return true;
	}
	return false;
}

void GameObject::RemoveAllAttributes() {
	attributes.clear();
}

void GameObject::DestroyAllAttributes() {
	for (auto attr : attributes) {
		delete attr.second;
	}

	attributes.clear();
}

void GameObject::AddAttrString(StrId key, string val) {
	AddAttr(key, val);
}

void GameObject::AddAttrInt(StrId key, int val) {
	AddAttr(key, val);
}

void GameObject::AddAttrFloat(StrId key, float val) {
	AddAttr(key, val);
}

void GameObject::AddAttrVector2f(StrId key, ofVec2f val) {
	AddAttr(key, val);
}

void GameObject::AddAttrVector3f(StrId key, ofVec3f val) {
	AddAttr(key, val);
}

void GameObject::AddAttrVec2i(StrId key, Vec2i val) {
	AddAttr(key, val);
}

void GameObject::AddAttrPtr(StrId key, void* pointer) {
	AddAttr(key, pointer);
}

string GameObject::GetAttrString(StrId key) {
	return GetAttr<string>(key);
}

int GameObject::GetAttrInt(StrId key) {
	return GetAttr<int>(key);
}

float GameObject::GetAttrFloat(StrId key) {
	return GetAttr<float>(key);
}

ofVec2f GameObject::GetAttrVector2f(StrId key) {
	return GetAttr<ofVec2f>(key);
}

ofVec3f GameObject::GetAttrVector3f(StrId key) {
	return GetAttr<ofVec3f>(key);
}

Vec2i GameObject::GetAttrVec2i(StrId key) {
	return GetAttr<Vec2i>(key);
}

void* GameObject::GetAttrPtr(StrId key) {
	auto it = attributes.find(key);
	if(it != attributes.end()) {
		if(it->second->IsPointer()) {
			// address of a pointer
			return *(void**)(it->second->RawVal());
		}else {
			// address of a value
			return it->second->RawVal();
		}
	}

	return nullptr;
}

