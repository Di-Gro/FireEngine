#include "NavMesh.h"
#include "Game.h"
#include "Actor.h"
#include <list>

#include "Transform.h"
#include "MeshComponent.h"
#include "Render.h"
#include "Scene.h"

void NavMesh::LoadStaticMeshes()
{
    /*auto camera_matrix = game->render()->camera()->cameraMatrix();*/
    uint64_t vertsIndex = 0;
    uint64_t prevVerticiesCount = 0;
    uint64_t prevIndexCountTotal = 0;

    rc_bmax[2] = FLT_MAX; rc_bmax[1] = FLT_MAX; rc_bmax[0] = FLT_MAX;
    rc_bmin[2] = FLT_MIN; rc_bmin[1] = FLT_MIN; rc_bmin[0] = FLT_MIN;
    auto scene = game->scene();
    for (auto actor_it = scene->GetNextRootActors(scene->BeginActor()); actor_it != scene->EndActor(); ++actor_it) {
        auto actor = *actor_it;
        if (actor->IsDestroyed())
            continue;;
        auto components = actor->GetComponentList();
        for (auto component : *components) {
            auto meshComppnent = dynamic_cast<MeshComponent*>(component);
            if (meshComppnent != nullptr) {
                auto mesh = meshComppnent->mesh();

                auto world_matrix = Matrix::CreateScale(meshComppnent->meshScale) * meshComppnent->GetWorldMatrix();
                for (auto shape_i = 0; shape_i < mesh->shapeCount(); shape_i++)
                {

                    auto shape = mesh->GetConstShape(shape_i);
                    auto transform = world_matrix /** camera_matrix*/;

                    StaticMeshesVert.resize(StaticMeshesVert.size() + shape->vertecesSize * 3);
                    StaticMeshesTris.resize(StaticMeshesTris.size() + shape->indecesSize);
                    for (auto i = 0; i < shape->vertecesSize * 3; i++)
                    {

                        auto pos = shape->verteces[i].position;
                        auto  PosMatrix = Matrix::CreateTranslation(pos.x, pos.y, pos.z) * transform;
                        auto NewPos = PosMatrix.Translation();
                        StaticMeshesVert[vertsIndex] = NewPos.x;
                        StaticMeshesVert[vertsIndex + 1] = NewPos.y;
                        StaticMeshesVert[vertsIndex + 2] = NewPos.z;
                        vertsIndex += 3;

                        if (NewPos.x < rc_bmin[0]) rc_bmin[0] = NewPos.x;
                        if (NewPos.y < rc_bmin[1]) rc_bmin[1] = NewPos.y;
                        if (NewPos.z < rc_bmin[2]) rc_bmin[2] = NewPos.z;

                        if (NewPos.x > rc_bmax[0]) rc_bmax[0] = NewPos.x;
                        if (NewPos.y > rc_bmax[1]) rc_bmax[1] = NewPos.y;
                        if (NewPos.z > rc_bmax[2]) rc_bmax[2] = NewPos.z;


                    }

                    for (size_t j = 0; j < shape->indecesSize; j++)
                    {
                        StaticMeshesTris[prevIndexCountTotal + j] = shape->indeces[j] + prevVerticiesCount;
                    }

                    prevIndexCountTotal += shape->indecesSize;
                    prevVerticiesCount += shape->vertecesSize * 3;
                }
            }
        }
    }

}

void NavMesh::RecastCleanup()
{
    if (m_triareas) delete[] m_triareas;
    m_triareas = 0;

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
    m_cellSize = 9.0;//0.3;
    m_cellHeight = 6.0;//0.2;
    m_agentMaxSlope = 45;
    m_agentHeight = 64.0;
    m_agentMaxClimb = 16;
    m_agentRadius = 16;
    m_edgeMaxLen = 512;
    m_edgeMaxError = 1.3;
    m_regionMinSize = 50;
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


    m_ctx->resetTimers();
    m_ctx->startTimer(RC_TIMER_TOTAL);


    LoadStaticMeshes();
    Configure();



    rcVcopy(m_cfg.bmin, rc_bmin);
    rcVcopy(m_cfg.bmax, rc_bmax);
    rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

    int nverts = StaticMeshesVert.size();
    int ntris = StaticMeshesTris.size();


    m_solid = rcAllocHeightfield();
    if (!m_solid)
    {

        return false;
    }
    if (!rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
    {

        return false;
    }

    m_triareas = new unsigned char[ntris];
    if (!m_triareas)
    {

        return false;
    }

    memset(m_triareas, 0, ntris * sizeof(unsigned char));
    rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle, StaticMeshesVert.data(), StaticMeshesVert.size(), StaticMeshesTris.data(), StaticMeshesTris.size(), m_triareas);
    rcRasterizeTriangles(m_ctx, StaticMeshesVert.data(), StaticMeshesVert.size(), StaticMeshesTris.data(), m_triareas, StaticMeshesTris.size(), *m_solid, m_cfg.walkableClimb);

    if (!m_keepInterResults)
    {
        delete[] m_triareas;
        m_triareas = 0;
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

    m_ctx->stopTimer(RC_TIMER_TOTAL);

    return true;

}


int NavMesh::FindPath(float* pStartPos, float* pEndPos, int nPathSlot, int nTarget)
{
    dtStatus status;
    dtPolyRef StartPoly;
    float StartNearest[3];
    dtPolyRef EndPoly;
    float EndNearest[3];
    dtPolyRef PolyPath[MAX_POLYGONS];
    int nPathCount = 0;
    float StraightPath[MAX_VERT * 3];
    int nVertCount = 0;


    // find the start polygon
    status = m_navQuery->findNearestPoly(pStartPos, mExtents, mFilter, &StartPoly, StartNearest);
    if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -1; // couldn't find a polygon

    // find the end polygon
    status = m_navQuery->findNearestPoly(pEndPos, mExtents, mFilter, &EndPoly, EndNearest);
    if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -2; // couldn't find a polygon

    status = m_navQuery->findPath(StartPoly, EndPoly, StartNearest, EndNearest, mFilter, PolyPath, &nPathCount, MAX_POLYGONS);
    if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -3; // couldn't create a path
    if (nPathCount == 0) return -4; // couldn't find a path

    status = m_navQuery->findStraightPath(StartNearest, EndNearest, PolyPath, nPathCount, StraightPath, NULL, NULL, &nVertCount, MAX_VERT);
    if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -5; // couldn't create a path
    if (nVertCount == 0) return -6; // couldn't find a path

    int nIndex = 0;
    for (int nVert = 0; nVert < nVertCount; nVert++)
    {
        m_PathStore[nPathSlot].pX[nVert] = StraightPath[nIndex++];
        m_PathStore[nPathSlot].pY[nVert] = StraightPath[nIndex++];
        m_PathStore[nPathSlot].pZ[nVert] = StraightPath[nIndex++];

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