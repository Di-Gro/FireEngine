#include "OpaquePass.h"

#include "OldPass.h"
#include "Render.h"
#include "CameraComponent.h"
#include "MaterialAlias.h"
#include "Mesh.h"
#include "MeshComponent.h"


void OpaquePass::Draw() {

	m_BeginDraw();

	for (auto& pair : f_sortedMaterials) {
		auto *matShapes = &pair.second;
		auto* material = matShapes->material;
		auto* shapes = matShapes->shapes;

		for (auto shapeRef : *shapes) {
			auto *meshComponent = shapeRef.first;
			auto index = shapeRef.second;

			if (!meshComponent->IsDestroyed())
				meshComponent->OnDrawShape(index);
		}
	}

	m_EndDraw();
}