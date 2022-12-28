#include "NavMesh.h"
#include "Game.h"
#include "Actor.h"
#include <list>

#include "CameraComponent.h"
#include "SimpleMath.h"
#include "Transform.h"
#include "MeshComponent.h"
#include "Render.h"

void NavMesh::LoadStaticMeshes()
{
	auto camera_matrix = game->render()->camera()->cameraMatrix();
	int vertsIndex = 0;
	int prevVerticiesCount = 0;
	int prevIndexCountTotal = 0;
	for (auto actor_it = game->GetNextRootActors(game->BeginActor()); actor_it != game->EndActor(); ++actor_it) {
		auto actor = *actor_it;
		if (actor->IsDestroyed())
			continue;;
		auto components = actor->GetComponentList();
		for (auto component : *components) {
			auto meshComppnent = dynamic_cast<MeshComponent*>(component);
			if (meshComppnent != nullptr) {
				auto mesh = meshComppnent->mesh();

				auto world_matrix = meshComppnent->GetWorldMatrix();
				for(auto shape_i = 0; shape_i<mesh->shapeCount();shape_i++)
				{

					auto shape = mesh->GetConstShape(shape_i);
					auto transform =  world_matrix * camera_matrix;
					
					StaticMeshesVert.resize(StaticMeshesVert.size() + shape->vertecesSize);
					StaticMeshesTris.resize(StaticMeshesTris.size() + shape->indecesSize);
					for(auto i = 0; i<shape->vertecesSize; i++)
					{

						auto pos = shape->verteces[i].position ;//*transform
						StaticMeshesVert[vertsIndex] = pos.x;
						StaticMeshesVert[vertsIndex+1] = pos.y;
						StaticMeshesVert[vertsIndex+2] = pos.z;
						vertsIndex += 3;
					}

					for (size_t j = 0; j < shape->indecesSize; j++)
					{
						StaticMeshesTris[prevIndexCountTotal + j] = shape->indeces[j] + prevVerticiesCount;
					}

					prevIndexCountTotal += shape->indecesSize;
					prevVerticiesCount += shape->vertecesSize;
				}
			}
		}
	}
	StaticMeshesNormals.resize(StaticMeshesTris.size());
	for (int i = 0; i < prevIndexCountTotal; i += 3)
	{
		const float* v0 = &StaticMeshesVert[StaticMeshesTris[i] * 3];
		const float* v1 = &StaticMeshesVert[StaticMeshesTris[i + 1] * 3];
		const float* v2 = &StaticMeshesVert[StaticMeshesTris[i + 2] * 3];
		float e0[3], e1[3];
		for (int j = 0; j < 3; ++j)
		{
			e0[j] = (v1[j] - v0[j]);
			e1[j] = (v2[j] - v0[j]);
		}
		float* n = &StaticMeshesNormals[i];
		n[0] = ((e0[1] * e1[2]) - (e0[2] * e1[1]));
		n[1] = ((e0[2] * e1[0]) - (e0[0] * e1[2]));
		n[2] = ((e0[0] * e1[1]) - (e0[1] * e1[0]));

		float d = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
		if (d > 0)
		{
			d = 1.0f / d;
			n[0] *= d;
			n[1] *= d;
			n[2] *= d;
		}
	}
}

void NavMesh::CalculateNavmesh()
{
	Vector3 vertA, vertB, vertC, vertNorm;
	int nVert = 0;

}