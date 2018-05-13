#ifndef _ORK_MESH_H_
#define _ORK_MESH_H_

#include <cstring>
#include "Types.h"
#include "../as-is/Engine/Geometry.h"
#include "../as-is/Engine/SubMesh.h"
#include "../as-is/Engine/MeshManager.h"
#include "../as-is/Engine/SceneManager.h"

using namespace std;

namespace VirtualGlobeRender
{

template <class vertex, class index> class Mesh 
{
public:
    Mesh(MeshMode m, MeshUsage usage, int verticesCount = 0, int indicesCount = 0):verticesCount(verticesCount), indicesCount(indicesCount)
    {
        vertices.resize(verticesCount);
        indices.resize(indicesCount);
    }

    virtual ~Mesh() {}

    int getVertexCount() const
    {
        return verticesCount;
    }

    vertex getVertex(int i) const
    {
        return vertices[i];
    }

    int getIndiceCount() const
    {
        return indicesCount;
    }

    void setCapacity(int verticesCount_, int indicesCount_)
    {
        verticesCount = verticesCount_;
        vertices.resize(verticesCount);
        indicesCount = indicesCount_;
        indices.resize(indicesCount);
    }

    void addVertex(const vertex &v)
    {
        vertices.add(v);
        verticesCount++;
    }

    void addVertices(const vertex *v, int count)
    {
        for (int i = 0; i < count; ++i)
        {
            addVertex(v[i]);
        }
    }

    void addIndice(index i)
    {
        indices.add(v);
        indicesCount++;
    }

    void setVertex(int i, const vertex &v)
    {
        vertices[i] = v;
    }

    void setIndice(int i, index ind)
    {
        indices[i] = ind;
    }

    void clear()
    {
        vertices.clear();
        verticesCount = 0;
        indices.clear();
        indicesCount = 0;
    }

    //below four functions used for creating meshes
    inline as_Geometry* createGeometry(const string &name)
    {
        as_Geometry* geometry = new as_Geometry();
        geometry->name = name;
        geometry->mNumVertices = verticesCount;
        geometry->mNumFaces = indicesCount / 3;
        geometry->mNumBones = 0;

        assert(verticesCount == vertices.size());
        //very slow(need refine)
        for (int k = 0; k < verticesCount; k++)
        {
            geometry->position.push_back(vertices[k]._p.x);
            geometry->position.push_back(vertices[k]._p.y);
            geometry->position.push_back(vertices[k]._p.z);
            geometry->normal.push_back(vertices[k]._c.x);
            geometry->normal.push_back(vertices[k]._c.y);
            geometry->normal.push_back(vertices[k]._c.z);
            geometry->normal.push_back(vertices[k]._c.w);
        }
        assert(indicesCount == indices.size());
        for (int k = 0; k < indicesCount; k++)
        {
            geometry->indices.push_back(indices[k]);
        }
       
        return geometry;
    }

    inline MeshPtr genMeshPtr(const string &name)
    {
        //only geometries and meshes
        vector<as_Geometry*> geos;
        vector<as_Mesh*> meshs;

        as_Geometry *geo = createGeometry(name);
        geos.push_back(geo);
        as_Mesh *mesh_ = new as_Mesh();
        mesh_->name = name;
        mesh_->geometry = geo;
        mesh_->material = NULL;
        mesh_->materialID = 0;
        meshs.push_back(mesh_);

        OMesh *mesh = new SubMesh(name);
        auto& g = GlobalResourceManager::getInstance();
        for (auto& x : geos)
        {
            g.as_geometryManager.add(x);
        }
        for (auto& x : meshs)
        {
            mesh->addSubMesh_as(x);
            g.as_meshManager.add(x);
        }

        return MeshPtr(mesh);
    }

    inline MeshPtr createMeshPtr(const string& name)
    {
        auto meshMgr = MeshManager::getSingletonPtr();
        auto mesh_ = meshMgr->getMeshByName(name);
        if (mesh_ != NULL)
            return mesh_;
        MeshPtr mesh = genMeshPtr(name);
        meshMgr->registerMesh(mesh);
        return mesh;
    }

    //you can create a scene for a scene_manager here
    inline SceneNode* createSceneNode(SceneManager* scene_manager, const string &name)
    {
        MeshPtr mesh = createMeshPtr(name);
        if (mesh == NULL) return NULL;

        Entity* entity = scene_manager->getEntity(name);
        if (entity == NULL)
            entity = scene_manager->CreateEntity(name);
        entity->setMesh(mesh);
        SceneNode* snode = scene_manager->getSceneNode(name);
        if (snode == NULL) {
            snode = scene_manager->CreateSceneNode(name);
            snode->attachMovable(entity);
        }
        return snode;
    }

private:
    vector<vertex> vertices;
    int verticesCount;

    vector<index>  indices;
    int indicesCount;
};

}

#endif
