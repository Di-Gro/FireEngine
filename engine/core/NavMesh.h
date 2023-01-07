#pragma once
#pragma once

#include <vector>
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"

class dtNavMesh;
class dtNavMeshQuery;
class Game;
namespace DirectX
{
	namespace SimpleMath
	{
		struct Vector3;
	}
}



#define MAX_PATHS 128//максимальное количество путей, которое можем хранить
#define MAX_POLYGONS 256// максимальное количество полигонов в пути
#define MAX_VERT 512//максимальное количество вершин в пути


typedef struct
{
	float pX[MAX_VERT];
	float pY[MAX_VERT];
	float pZ[MAX_VERT];

	int MaxVertex;
	int Target;
}pathdata;


enum SamplePolyAreas
{
	SAMPLE_POLYAREA_GROUND,
	SAMPLE_POLYAREA_WATER,
	SAMPLE_POLYAREA_ROAD,
	SAMPLE_POLYAREA_DOOR,
	SAMPLE_POLYAREA_GRASS,
	SAMPLE_POLYAREA_JUMP,
};
enum SamplePolyFlags
{
	SAMPLE_POLYFLAGS_WALK = 0x01,
	SAMPLE_POLYFLAGS_SWIM = 0x02,
	SAMPLE_POLYFLAGS_DOOR = 0x04,
	SAMPLE_POLYFLAGS_JUMP = 0x08,
	SAMPLE_POLYFLAGS_ALL = 0xffff
};


class NavMesh
{
public:
	void LoadStaticMeshes();
	void CalculateNavmesh();
	void UpdateNavmesh();
	void GetNavmesh();
	bool SaveNavmesh();
	bool NavMeshBuild();
	std::vector<DirectX::SimpleMath::Vector3> GetPath(int pathSlot);
	int FindPath(DirectX::SimpleMath::Vector3 pStartPos, DirectX::SimpleMath::Vector3 pEndPos, int nPathSlot, int nTarget);
	int FindPath(float* pStartPos, float* pEndPos, int nPathSlot, int nTarget);
	NavMesh(Game* game);
	void DebugDrawPolyMesh();
protected:

	unsigned char* m_triareas;
	rcHeightfield* m_solid;
	rcCompactHeightfield* m_chf;
	rcContourSet* m_cset;
	rcPolyMesh* m_pmesh;
	rcConfig m_cfg;
	rcPolyMeshDetail* m_dmesh;
	void Configure();
	class dtNavMesh* m_navMesh;
	class dtNavMeshQuery* m_navQuery;
	unsigned char m_navMeshDrawFlags;

	rcContext* m_ctx;

	float m_cellSize;
	float m_cellHeight;
	float m_agentHeight;
	float m_agentRadius;
	float m_agentMaxClimb;
	float m_agentMaxSlope;
	float m_regionMinSize;
	float m_regionMergeSize;
	float m_edgeMaxLen;
	float m_edgeMaxError;
	int m_vertsPerPoly;
	float m_detailSampleDist;
	float m_detailSampleMaxError;
	bool m_keepInterResults;
	
	static const int MAX_OFFMESH_CONNECTIONS = 256;
	float m_offMeshConVerts[MAX_OFFMESH_CONNECTIONS * 3 * 2];
	float m_offMeshConRads[MAX_OFFMESH_CONNECTIONS];
	unsigned char m_offMeshConDirs[MAX_OFFMESH_CONNECTIONS];
	unsigned char m_offMeshConAreas[MAX_OFFMESH_CONNECTIONS];
	unsigned short m_offMeshConFlags[MAX_OFFMESH_CONNECTIONS];
	unsigned int m_offMeshConId[MAX_OFFMESH_CONNECTIONS];
	int m_offMeshConCount;

	void RecastCleanup();

	float mExtents[3];
	float m_flTestStart[3];
	float m_flTestEnd[3];
	dtQueryFilter* mFilter;

	pathdata m_PathStore[MAX_PATHS];
	float m_scale;
private:

	Game* game;
	std::vector<float> StaticMeshesVert;
	std::vector <int> StaticMeshesTris;


	float	           rc_bmin[3];
	float	           rc_bmax[3];
};
