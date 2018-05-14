#ifndef _ORK_MESH_H_
#define _ORK_MESH_H_

#include <cstring>
#include <sstream>
#include "Types.h"
#include "../as-is/Engine/Geometry.h"
#include "../as-is/Engine/SubMesh.h"
#include "../as-is/Engine/MeshManager.h"
#include "../as-is/Engine/SceneManager.h"
#include "../as-is/Engine/OpenglDriver/GLGeometry.h"

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
        position.resize(verticesCount * 3);
        color.resize(verticesCount * 4);
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
        /////
        position.resize(verticesCount * 3);
        color.resize(verticesCount * 4);
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
    as_Geometry* createGeometry(const string &name)
    {
        as_Geometry* geometry = new as_Geometry();
        geometry->name = name;
        geometry->mNumVertices = verticesCount;
        geometry->mNumFaces = indicesCount / 3;
        geometry->mNumBones = 0;

        assert(verticesCount == vertices.size());
        cout << "verticesCount=" << verticesCount << endl;
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
        cout << "indicesCount=" << indicesCount << endl;
        for (int k = 0; k < indicesCount; k++)
        {
            geometry->indices.push_back(indices[k]);
        }
       
        return geometry;
    }

    MeshPtr genMeshPtr(const string &name)
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

    MeshPtr createMeshPtr(const string& name)
    {
        auto meshMgr = MeshManager::getSingletonPtr();
        auto mesh_ = meshMgr->getMeshByName(name);
        if (mesh_ != NULL)
            return mesh_;
        MeshPtr mesh = genMeshPtr(name);
        meshMgr->registerMesh(mesh);
        return mesh;
    }

    Entity *createEntity(SceneManager* scene_manager, const string &name)
    {
        MeshPtr mesh = createMeshPtr(name);
        if (mesh == NULL) return NULL;
        Entity* entity = scene_manager->getEntity(name);
        if (entity == NULL)
            entity = scene_manager->CreateEntity(name);
        entity->setMesh(mesh);
        return entity;
    }

    //you can create a scene for a scene_manager here
    SceneNode* createSceneNode(SceneManager* scene_manager, const string &name)
    {
        Entity* entity = createEntity(scene_manager, name);
        SceneNode* snode = scene_manager->getSceneNode(name);
        if (snode == NULL) {
            snode = scene_manager->CreateSceneNode(name);
            snode->attachMovable(entity);
        }
        return snode;
    }

    //写死了
    void updateGPUVertexData(const GL_GPUVertexData &vertexData)
    {
        //as_Geometry* as_geometry = createGeometry("Temp");
        glBindBuffer(GL_ARRAY_BUFFER, vertexData.vbo[0]);
       // glBufferSubData(GL_ARRAY_BUFFER, 0, as_geometry->position.size() * sizeof(float), &as_geometry->position[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, position.size() * sizeof(float), &position[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vertexData.vbo[1]);
        //glBufferSubData(GL_ARRAY_BUFFER, 0, as_geometry->normal.size() * sizeof(float), &as_geometry->normal[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, color.size() * sizeof(float), &color[0]);

    }
public:
   /* static void loadMeshesToSceneManager(SceneManager* scene_manager, vector<Mesh<vertex, index>*> &meshes, const string &name_root)
    {
        auto sceneRoot = scene_manager->GetSceneRoot();
        auto objectNode = scene_manager->CreateSceneNode("objectNode");
        sceneRoot->attachNode(objectNode);

        int numOfMeshes = meshes.size();
        for (int k = 0; k < numOfMeshes; k++)
        {
            auto mesh = meshes[k];
            assert(mesh != NULL);
            ostringstream name;
            name << name_root << "_" << k << endl;
            cout << name.str() << endl;
            auto snode = mesh->createSceneNode(scene_manager, name.str());
            assert(snode != NULL);
            objectNode->attachNode(snode);
        }
    }*/

private:
    vector<vertex> vertices;
    int verticesCount;

    vector<index>  indices;
    int indicesCount;

public:
    vector<float> position;//#v * 3
    vector<float> color;//#v * 4
};

}

#endif
