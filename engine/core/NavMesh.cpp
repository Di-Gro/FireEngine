#include "NavMesh.h"
#include "Game.h"
#include "Actor.h"
#include <list>

#include "Transform.h"
#include "MeshComponent.h"
#include "LineComponent.h"
#include "Render.h"
#include "Scene.h"
#include "DetourTileCacheBuilder.h"
#include "MeshAsset.h"
#include "DirectXCollision.h"



NavMesh::NavMesh(Game* game) :game(game)
{

    m_solid = NULL;
    m_chf = NULL;
    m_cset = NULL;
    m_pmesh = NULL;
    //m_cfg;   
    m_dmesh = NULL;

    m_navMesh = NULL;
    m_navQuery = NULL;
    //m_navMeshDrawFlags;
    m_ctx = NULL;

    RecastCleanup();


    m_scale = 20;





    mExtents[0] = 2 * m_scale;
    mExtents[1] = 4 * m_scale;
    mExtents[2] = 2 * m_scale;

    // Setup the default query filter
    mFilter = new dtQueryFilter();
    mFilter->setIncludeFlags(0xFFFF);    // Include all
    mFilter->setExcludeFlags(0);         // Exclude none
    // Area flags for polys to consider in search, and their cost
    mFilter->setAreaCost(SAMPLE_POLYAREA_GROUND, 1.0f);       // TODO have a way of configuring the filter
    mFilter->setAreaCost(DT_TILECACHE_WALKABLE_AREA, 1.0f);


    // Init path store. MaxVertex 0 means empty path slot
    for (int i = 0; i < MAX_PATHS; i++) {
        m_PathStore[i].MaxVertex = 0;
        m_PathStore[i].Target = 0;
    }


    // Set configuration
    Configure();
}



void NavMesh::LoadStaticMeshes()
{
    /*auto camera_matrix = game->render()->camera()->cameraMatrix();*/
    uint64_t vertsIndex = 0;
    uint64_t prevVerticiesCount = 0;
    uint64_t prevIndexCountTotal = 0;

    auto scene = game->currentScene();
    for (auto actor_it = scene->GetNextRootActors(scene->BeginActor()); actor_it != scene->EndActor(); ++actor_it) {
        auto actor = *actor_it;
        if (actor->IsDestroyed())
            continue;;
        auto components = actor->GetComponentList();
        for (auto component : *components) {
            auto meshComppnent = dynamic_cast<MeshComponent*>(component);
            if (meshComppnent != nullptr) {
                auto mesh = meshComppnent->mesh();
                if (mesh == nullptr || mesh->topology != D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
                    continue;
                std::cout << actor->name() << std::endl;
                auto world_matrix = Matrix::CreateScale(meshComppnent->meshScale) * meshComppnent->GetWorldMatrix();
                for (auto shape_i = 0; shape_i < mesh->shapeCount(); shape_i++)
                {

                    auto shape = mesh->GetConstShape(shape_i);



                    auto transform = world_matrix /** camera_matrix*/;
                    std::cout << " shape->vertecesSize =  " << shape->vertecesSize << '\n';
                    std::cout << " shape->indecesSize =  " << shape->indecesSize << '\n';
                    StaticMeshesVert.resize(StaticMeshesVert.size() + shape->vertecesSize * 3);
                    StaticMeshesTris.resize(StaticMeshesTris.size() + shape->indecesSize);
                    for (auto i = 0; i < shape->vertecesSize; i++)
                    {

                        auto pos = shape->verteces[i].position;
                        auto  PosMatrix = Matrix::CreateTranslation(pos.x, pos.y, pos.z) * transform;
                        auto NewPos = PosMatrix.Translation();
                        StaticMeshesVert[vertsIndex] = NewPos.x;
                        StaticMeshesVert[vertsIndex + 1] = NewPos.y;
                        StaticMeshesVert[vertsIndex + 2] = NewPos.z;
                        vertsIndex += 3;
                    }

                    for (size_t j = 0; j < shape->indecesSize; j++)
                    {
                        StaticMeshesTris[prevIndexCountTotal + j] = shape->indeces[j] + prevVerticiesCount;
                    }

                    prevIndexCountTotal += shape->indecesSize;
                    prevVerticiesCount += shape->vertecesSize;
                    std::cout << "prevIndexCountTotal = " << prevIndexCountTotal << '\n';
                    std::cout << "prevVerticiesCount = " << prevVerticiesCount << '\n';
                }
            }
        }
    }



}

void NavMesh::DebugDrawPolyMesh()
{
    auto& mesh = *m_pmesh;
    const int nvp = mesh.nvp;
    const float cs = mesh.cs;
    const float ch = mesh.ch;
    const float* orig = mesh.bmin;
    std::vector<Vector3> lines;

    Vector4 color = { 0.5,0.5,0.5,1 };
    DirectX::SimpleMath::Color coln = DirectX::SimpleMath::Color(0, 48 / 255.0f, 64 / 255.0f, 32 / 255.0f);
    for (int i = 0; i < mesh.npolys; ++i)
    {
        const unsigned short* p = &mesh.polys[i * nvp * 2];
        for (int j = 0; j < nvp; ++j)
        {
            if (p[j] == RC_MESH_NULL_IDX) break;
            if (p[nvp + j] & 0x8000) continue;
            const int nj = (j + 1 >= nvp || p[j + 1] == RC_MESH_NULL_IDX) ? 0 : j + 1;
            const int vi[2] = { p[j], p[nj] };

            DirectX::SimpleMath::Vector3 points[2];
            for (int k = 0; k < 2; ++k) {
                const unsigned short* v = &mesh.verts[vi[k] * 3];
                const float x = orig[0] + v[0] * cs;
                const float y = orig[1] + (v[1] + 1) * ch + 2.f;
                const float z = orig[2] + v[2] * cs;
                points[k] = DirectX::SimpleMath::Vector3(x, y, z);
            }
            lines.emplace_back(points[0]);
            lines.emplace_back(points[1]);
        }
    }

    // Draw boundary edges
    DirectX::SimpleMath::Color colb = DirectX::SimpleMath::Color(0.9f, 0.1f, 0.1f, 0.8f);
    for (int i = 0; i < mesh.npolys; ++i)
    {
        const unsigned short* p = &mesh.polys[i * nvp * 2];
        for (int j = 0; j < nvp; ++j)
        {
            if (p[j] == RC_MESH_NULL_IDX) break;
            if ((p[nvp + j] & 0x8000) == 0) continue;
            const int nj = (j + 1 >= nvp || p[j + 1] == RC_MESH_NULL_IDX) ? 0 : j + 1;
            const int vi[2] = { p[j], p[nj] };

            DirectX::SimpleMath::Color col = colb;
            if ((p[nvp + j] & 0xf) != 0xf)
                col = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 0.5f);

            DirectX::SimpleMath::Vector3 points[2];
            for (int k = 0; k < 2; ++k)
            {
                const unsigned short* v = &mesh.verts[vi[k] * 3];
                const float x = orig[0] + v[0] * cs;
                const float y = orig[1] + (v[1] + 1) * ch + 2.f;
                const float z = orig[2] + v[2] * cs;
                points[k] = DirectX::SimpleMath::Vector3(x, y, z);
            }

            lines.emplace_back(points[0]);
            lines.emplace_back(points[1]);
        }
    }
    auto actor = game->currentScene()->CreateActor("PathLine");
    auto line = actor->AddComponent<LineComponent>();
    line->SetVector(lines, color);

}

void NavMesh::RecastCleanup()
{
    if (m_triareas.data()) m_triareas.clear();


    rcFreeHeightField(m_solid);
    m_solid = 0;
    rcFreeCompactHeightfield(m_chf);
    m_chf = 0;
    rcFreeContourSet(m_cset);
    m_cset = 0;
    rcFreePolyMesh(m_pmesh);
    m_pmesh = 0;
    rcFreePolyMeshDetail(m_dmesh);
    m_dmesh = 0;
    dtFreeNavMesh(m_navMesh);
    m_navMesh = 0;

    dtFreeNavMeshQuery(m_navQuery);
    m_navQuery = 0;

    if (m_ctx) {
        delete m_ctx;
        m_ctx = 0;
    }
}

void NavMesh::Configure()
{
    if (m_ctx) {
        delete m_ctx;
        m_ctx = 0;
    }
    m_ctx = new rcContext(true);
    m_cellSize = 0.3 * m_scale;//0.3;
    m_cellHeight = 0.2 * m_scale;//0.2;
    m_agentMaxSlope = 45;
    m_agentHeight = 0.2 * m_scale;
    m_agentMaxClimb = 0.9 * m_scale;
    m_agentRadius = 0.6 * m_scale;
    m_edgeMaxLen = 12 * m_scale;
    m_edgeMaxError = 1.3;
    m_regionMinSize = 8 * m_scale;
    m_regionMergeSize = 20;
    m_vertsPerPoly = 6;
    m_detailSampleDist = 6;
    m_detailSampleMaxError = 1;
    m_keepInterResults = false;

    memset(&m_cfg, 0, sizeof(m_cfg));
    m_cfg.cs = m_cellSize;
    m_cfg.ch = m_cellHeight;
    m_cfg.walkableSlopeAngle = m_agentMaxSlope;
    m_cfg.walkableHeight = (int)ceilf(m_agentHeight / m_cfg.ch);
    m_cfg.walkableClimb = (int)floorf(m_agentMaxClimb / m_cfg.ch);
    m_cfg.walkableRadius = (int)ceilf(m_agentRadius / m_cfg.cs);
    m_cfg.maxEdgeLen = (int)(m_edgeMaxLen / m_cellSize);
    m_cfg.maxSimplificationError = m_edgeMaxError;
    m_cfg.minRegionArea = (int)rcSqr(m_regionMinSize);
    m_cfg.mergeRegionArea = (int)rcSqr(m_regionMergeSize);
    m_cfg.maxVertsPerPoly = (int)m_vertsPerPoly;
    m_cfg.detailSampleDist = m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
    m_cfg.detailSampleMaxError = m_cellHeight * m_detailSampleMaxError;
}


bool NavMesh::NavMeshBuild()
{
    LoadStaticMeshes();


    rcCalcBounds(&StaticMeshesVert[0], StaticMeshesVert.size() / 3, rc_bmin, rc_bmax);
    rcVcopy(m_cfg.bmin, rc_bmin);
    rcVcopy(m_cfg.bmax, rc_bmax);
    rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

    int ntris = StaticMeshesTris.size() / 3;


    m_solid = rcAllocHeightfield();
    if (!m_solid)
    {

        return false;
    }
    if (!rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
    {

        return false;
    }

    m_triareas.resize(ntris);
    if (!m_triareas.data())
    {
        return false;
    }

    memset(m_triareas.data(), 0, ntris * sizeof(unsigned char));
    rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle, StaticMeshesVert.data(), StaticMeshesVert.size(), StaticMeshesTris.data(), StaticMeshesTris.size() / 3, m_triareas.data());
    rcRasterizeTriangles(m_ctx, StaticMeshesVert.data(), StaticMeshesVert.size(), StaticMeshesTris.data(), m_triareas.data(), StaticMeshesTris.size() / 3, *m_solid, m_cfg.walkableClimb);

    if (!m_keepInterResults)
    {
        m_triareas.clear();

    }

    rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *m_solid);
    rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
    rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *m_solid);


    m_chf = rcAllocCompactHeightfield();
    if (!m_chf)
    {
        return false;
    }
    if (!rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
    {

        return false;
    }

    if (!m_keepInterResults)
    {
        rcFreeHeightField(m_solid);
        m_solid = 0;
    }



    if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *m_chf))
    {
        m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
        return false;
    }

    if (!rcBuildDistanceField(m_ctx, *m_chf))
    {

        return false;
    }


    if (!rcBuildRegions(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
    {

        return false;
    }

    m_cset = rcAllocContourSet();
    if (!m_cset)
    {

        return false;
    }
    if (!rcBuildContours(m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
    {

        return false;
    }

    if (m_cset->nconts == 0)
    {

    }


    m_pmesh = rcAllocPolyMesh();
    if (!m_pmesh)
    {

        return false;
    }
    if (!rcBuildPolyMesh(m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
    {

        return false;
    }


    m_dmesh = rcAllocPolyMeshDetail();
    if (!m_dmesh)
    {

        return false;
    }

    if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
    {

        return false;
    }

    if (!m_keepInterResults)
    {
        rcFreeCompactHeightfield(m_chf);
        m_chf = 0;
        rcFreeContourSet(m_cset);
        m_cset = 0;
    }


    if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
    {


        unsigned char* navData = 0;
        int navDataSize = 0;



        for (int i = 0; i < m_pmesh->npolys; ++i)
        {
            if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
            {
                m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;
                m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
            }
        }


        // Set navmesh params
        dtNavMeshCreateParams params;
        memset(&params, 0, sizeof(params));
        params.verts = m_pmesh->verts;
        params.vertCount = m_pmesh->nverts;
        params.polys = m_pmesh->polys;
        params.polyAreas = m_pmesh->areas;
        params.polyFlags = m_pmesh->flags;
        params.polyCount = m_pmesh->npolys;
        params.nvp = m_pmesh->nvp;
        params.detailMeshes = m_dmesh->meshes;
        params.detailVerts = m_dmesh->verts;
        params.detailVertsCount = m_dmesh->nverts;
        params.detailTris = m_dmesh->tris;
        params.detailTriCount = m_dmesh->ntris;

        // no off mesh connections yet
        m_offMeshConCount = 0;
        params.offMeshConVerts = m_offMeshConVerts;
        params.offMeshConRad = m_offMeshConRads;
        params.offMeshConDir = m_offMeshConDirs;
        params.offMeshConAreas = m_offMeshConAreas;
        params.offMeshConFlags = m_offMeshConFlags;
        params.offMeshConUserID = m_offMeshConId;
        params.offMeshConCount = m_offMeshConCount;

        params.walkableHeight = m_agentHeight;
        params.walkableRadius = m_agentRadius;
        params.walkableClimb = m_agentMaxClimb;
        rcVcopy(params.bmin, m_pmesh->bmin);
        rcVcopy(params.bmax, m_pmesh->bmax);
        params.cs = m_cfg.cs;
        params.ch = m_cfg.ch;

        if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
        {

            return false;
        }



        m_navMesh = dtAllocNavMesh();
        if (!m_navMesh)
        {
            dtFree(navData);

            return false;
        }



        dtStatus status;

        status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
        if (dtStatusFailed(status))
        {
            dtFree(navData);

            return false;
        }


        m_navQuery = dtAllocNavMeshQuery();
        status = m_navQuery->init(m_navMesh, 2048);



        if (dtStatusFailed(status))
        {

            return false;
        }

    }
    DebugDrawPolyMesh();
    m_ctx->stopTimer(RC_TIMER_TOTAL);

    return true;

}

int NavMesh::FindPath(Vector3 pStartpos, Vector3 pEndPos, int npathSlot, int nTarget)
{
    float start[3];
    float end[3];
    start[0] = pStartpos.x; start[1] = pStartpos.y; start[2] = pStartpos.z;
    end[0] = pEndPos.x; end[1] = pEndPos.y; end[2] = pEndPos.z;
    return FindPath(start, end, npathSlot, nTarget);
}


int NavMesh::FindPath(float* pStartPos, float* pEndPos, int nPathSlot, int nTarget)
{
    dtStatus status;
    float pExtents[3] = { 1.0f, 2.0f, 1.0f }; // size of box around start/end points to look for nav polygons
    dtPolyRef StartPoly;
    float StartNearest[3];
    dtPolyRef EndPoly;
    float EndNearest[3];
    dtPolyRef PolyPath[MAX_POLYGONS];
    int nPathCount = 0;
    float StraightPath[MAX_VERT * 3];
    int nVertCount = 0;


    // setup the filter
    dtQueryFilter Filter;
    Filter.setIncludeFlags(0xFFFF);
    Filter.setExcludeFlags(0);
    Filter.setAreaCost(SAMPLE_POLYAREA_GROUND, 1.0f);

    // find the start polygon
    status = m_navQuery->findNearestPoly(pStartPos, mExtents, &Filter, &StartPoly, StartNearest);
    if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -1; // couldn't find a polygon

    // find the end polygon
    status = m_navQuery->findNearestPoly(pEndPos, mExtents, &Filter, &EndPoly, EndNearest);
    if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -2; // couldn't find a polygon

    status = m_navQuery->findPath(StartPoly, EndPoly, StartNearest, EndNearest, &Filter, PolyPath, &nPathCount, MAX_POLYGONS);
    if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -3; // couldn't create a path
    if (nPathCount == 0) return -4; // couldn't find a path

    status = m_navQuery->findStraightPath(StartNearest, EndNearest, PolyPath, nPathCount, StraightPath, NULL, NULL, &nVertCount, MAX_VERT);
    if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -5; // couldn't create a path
    if (nVertCount == 0) return -6; // couldn't find a path

    // At this point we have our path.  Copy it to the path store
    int nIndex = 0;
    for (int nVert = 0; nVert < nVertCount; nVert++)
    {
        m_PathStore[nPathSlot].pX[nVert] = StraightPath[nIndex++];
        m_PathStore[nPathSlot].pY[nVert] = StraightPath[nIndex++];
        m_PathStore[nPathSlot].pZ[nVert] = StraightPath[nIndex++];

        //sprintf(m_chBug, "Path Vert %i, %f %f %f", nVert, m_PathStore[nPathSlot].PosX[nVert], m_PathStore[nPathSlot].PosY[nVert], m_PathStore[nPathSlot].PosZ[nVert]) ;
        //m_pLog->logMessage(m_chBug);
    }
    m_PathStore[nPathSlot].MaxVertex = nVertCount;
    m_PathStore[nPathSlot].Target = nTarget;

    return nVertCount;

}


std::vector<Vector3>NavMesh::GetPath(int pathSlot)
{
    std::vector<Vector3> result;
    if (pathSlot < 0 || pathSlot >= MAX_PATHS || m_PathStore[pathSlot].MaxVertex <= 0)
        return result;

    pathdata* path = &(m_PathStore[pathSlot]);
    result.reserve(path->MaxVertex);
    for (int i = 0; i < path->MaxVertex; i++) {
        result.push_back(Vector3(path->pX[i], path->pY[i], path->pZ[i]));
    }
    return result;
}

void NavMesh::GethPath(size_t* vertexes, int* count, int pathSlot) {
    if (pathSlot < 0 || pathSlot >= MAX_PATHS || m_PathStore[pathSlot].MaxVertex <= 0)
        return;
    pathdata*path = &(m_PathStore[pathSlot]);
    for (auto i = 0; i < path->MaxVertex; i++) {
       
    }
}

int NavMesh::PointsCount(int pathSlot) {
    if (pathSlot < 0 || pathSlot >= MAX_PATHS || m_PathStore[pathSlot].MaxVertex <= 0)
        return 0;
    pathdata* path = &(m_PathStore[pathSlot]);
    return path->MaxVertex;
}

static float frand()
{
    return (float)rand() / (float)RAND_MAX;
}


Vector3 NavMesh::getRandomNavMeshPoint() {
    float resultPoint[3];
    dtPolyRef resultPoly;
    m_navQuery->findRandomPoint(mFilter, frand, &resultPoly, resultPoint);

    return Vector3(resultPoint[0], resultPoint[1], resultPoint[2]);
}

