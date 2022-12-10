#pragma once
#include <utility>
#include <list>
#include <map>

class Material;
class MeshComponent;
class Component;
class ILightSource;

namespace Pass {
	
	using MaterialPriority = size_t;
	using MaterialRef = const Material*;

	using ShapeIndex = int;
	using ShapeRef = std::pair<MeshComponent*, ShapeIndex>;
	using ShapeList = std::list<ShapeRef>;
	using ShapeIter = ShapeList::iterator;

	using ShadowCaster = std::list<Component*>::iterator;
	using LightSource = std::list<ILightSource*>::iterator;

	struct MatShapes {
		MaterialRef material;
		ShapeList* shapes;
	};

	using SortedMaterials = std::multimap<MaterialPriority, MatShapes>;
	using MaterialIter = SortedMaterials::iterator;

}