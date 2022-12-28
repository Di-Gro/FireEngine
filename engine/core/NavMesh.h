#pragma once

#include <vector>
#include <DirectXMath.h>
#include <utility>

class Game;


class NavMesh
{
public:
	void LoadStaticMeshes();
	void CalculateNavmesh();
	void UpdateNavmesh();
	void GetNavmesh();
	bool SaveNavmesh();
	NavMesh(Game* game) :game(game)	{};
private:

	Game* game;
	std::vector<float> StaticMeshesVert;
	std::vector <int> StaticMeshesTris;
	std::vector<float> StaticMeshesNormals;
	
	float	           rc_bmin[3];
	float	           rc_bmax[3];
};

