#pragma once
#include <utility>
#include <list>
#include <map>

class MaterialResource;
class MeshComponent;
class Component;
class ILightSource;
class IShadowCaster;

namespace Pass {
	
	using MaterialPriority = size_t;
	using MaterialRef = const MaterialResource*;

	using ShapeIndex = int;
	using ShapeRef = std::pair<MeshComponent*, ShapeIndex>;
	using ShapeList = std::list<ShapeRef>;
	using ShapeIter = ShapeList::iterator;

	using ShadowCaster = std::list<IShadowCaster*>::iterator;
	using LightSource = std::list<ILightSource*>::iterator;

	struct MatShapes {
		MaterialRef material;
		ShapeList* shapes;
	};

	using SortedMaterials = std::multimap<MaterialPriority, MatShapes>;
	using MaterialIter = SortedMaterials::iterator;

}