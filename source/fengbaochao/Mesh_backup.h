#ifndef _ORK_MESH_H_
#define _ORK_MESH_H_

#include <cstring> // for memcpy
#include "Types.h"
#include "../as-is/Engine/Geometry.h"
#include "../as-is/Engine/SubMesh.h"
#include "../as-is/Engine/SceneManager.h"
//#include "ork/render/CPUBuffer.h"
//#include "ork/render/GPUBuffer.h"
//#include "ork/render/MeshBuffers.h"

using namespace std;

namespace VirtualGlobeRender
{

    struct P3_C4
    {
        vec3f _p;
        vec4f _c;
        P3_C4() {}
        P3_C4(const vec3f& p, const vec4f& c) : _p(p), _c(c) {}
    };

/**
 * A MeshBuffers wrapper that provides a convenient API to define the mesh content.
 * @ingroup render
 *
 * @tparam vertex the type of the vertices of this mesh.
 * @tparam index the type of the indices of this mesh.
 */
template <class vertex, class index> class Mesh 
{
public:
    /**
     * Creates a new mesh.
     *
     * @param m how the list of vertices of this mesh must be interpreted.
     * @param usage how the data should be handled.
     * @param vertexCount the initial capacity of the vertex array.
     * @param indiceCount the initial capacity of the indice array.
     */
    Mesh(MeshMode m, MeshUsage usage, int vertexCount = 4, int indiceCount = 4);

    /**
     * Creates a new mesh.
     *
     * @param target the mesh buffers wrapped by this mesh.
     * @param m how the list of vertices of this mesh must be interpreted.
     * @param usage how the data should be handled.
     * @param vertexCount the initial capacity of the vertex array.
     * @param indiceCount the initial capacity of the indice array.
     */
    //Mesh(ptr<MeshBuffers> target, MeshMode m, MeshUsage usage, int vertexCount = 4, int indiceCount = 4);

    /**
     * Deletes this mesh.
     */
    virtual ~Mesh();

    /**
     * Returns the interpretation mode of the vertices of this mesh.
     */
    inline MeshMode getMode() const;

    /**
     * Returns the number of vertices in this mesh.
     */
    inline int getVertexCount() const;

    /**
     * Returns a vertex of this mesh.
     *
     * @param i a vertex index.
     */
    inline vertex getVertex(int i) const;

    /**
     * Returns the number of indices of this mesh.
     */
    inline int getIndiceCount() const;

    /**
     * Returns the vertex index used for primitive restart. -1 means no restart.
     */
    inline GLint getPrimitiveRestart() const;

    /**
     * Returns the number of vertices per patch in this mesh, if #getMode() is PATCHES.
     */
    inline GLint getPatchVertices() const;

    /**
     * Returns the MeshBuffers wrapped by this Mesh instance.
     */
    //inline ptr<MeshBuffers> getBuffers() const;

    /**
     * Declares an attribute of the vertices of this mesh.
     *
     * @param id a vertex attribute index.
     * @param size the number of components in attributes of this kind.
     * @param type the type of each component in attributes of this kind.
     * @param norm if the attribute components must be normalized to 0..1.
     */
    inline void addAttributeType(int id, int size, AttributeType type, bool norm);
	inline void addAttributeType(int id, int size, AttributeType type, bool norm, int divisor);
    /**
     * Sets the capacity of the vertex and indice array of this mesh. Does
     * nothing if the provided sizes are smaller than the current ones.
     *
     * @param vertexCount the new vertex array capacity.
     * @param indiceCount the new indice array capacity.
     */
    inline void setCapacity(int vertexCount, int indiceCount);

    /**
     * Adds a vertex to this mesh.
     *
     * @param v a vertex.
     */
    inline void addVertex(const vertex &v);

    /**
     * Adds vertices this mesh.
     *
     * @param v a pointer to a vertex array.
     * @param count number of vertices
     */
    inline void addVertices(const vertex *v, int count);

    /**
     * Adds an indice to this mesh.
     *
     * @param i a vertex index.
     */
    inline void addIndice(index i);

    /**
     * Sets the interpretation mode of the vertices of this mesh.
     */
    inline void setMode(MeshMode mode);

    /**
     * Changes a vertex of this mesh.
     */
    inline void setVertex(int i, const vertex &v);

    /**
     * Changes an indice of this mesh.
     */
    inline void setIndice(int i, index ind);

    /**
     * Sets the vertex index used for primitive restart. -1 means no restart.
     */
    inline void setPrimitiveRestart(GLint restart);

    /**
     * Sets the number of vertices per patch in this mesh, if #getMode() is PATCHES.
     */
    inline void setPatchVertices(GLint vertices);

    /**
     * Removes all the vertices and indices of this mesh.
     */
    inline void clear();

    /**
     * Clears the MeshBuffers.
     */
    inline void clearBuffers();


	/**
     * 设置是否开启CPU段的顶点信息自动在传往GPU后就自动删除机制，主要为大范围动态场景内存高效管理服务
     */
    inline void setCPUDataClearMode(bool whether);

	/**
     * 查询是否开启CPU段的顶点信息自动在传往GPU后就自动删除机制
     */
    inline bool isCPUDataClearMode();

    //below four functions used for creating meshes
    inline as_Geometry* createGeometry(const string &name)
    {
        as_Geometry* geometry = new as_Geometry();
        geometry->name = name;
        geometry->mNumVertices = verticesCount;
        geometry->mNumFaces = indicesCount / 3;
        geometry->mNumBones = 0;

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
    /**
     * The usage of this mesh.
     */
    MeshUsage usage;

    /**
     * The Buffer containing the vertices data.
     */
    //mutable ptr<Buffer> vertexBuffer;

    /**
     * The Buffer containing the indices data.
     */
    //mutable ptr<Buffer> indexBuffer;

    /**
     * True if the vertex data has changed since last call to #uploadVertexDataToGPU.
     */
    mutable bool vertexDataHasChanged;

    /**
     * True if the index data has changed since last call to #uploadIndexDataToGPU.
     */
    mutable bool indexDataHasChanged;

    /**
     * True if the CPU or GPU mesh buffers have been created.
     */
    mutable bool created;

    /**
     * How the list of vertices of this mesh must be interpreted.
     */
    MeshMode m;

    /**
     * The vertices of this mesh.
     */
    mutable vertex *vertices;

    /**
     * The capacity of the vertex array.
     */
    int verticesLength;

    /**
     * The actual number of vertices.
     */
    int verticesCount;

    /**
     * The indices of this mesh.
     */
    mutable index *indices;

    /**
     * The capacity of the indice array.
     */
    int indicesLength;

    /**
     * The actual number of indices.
     */
    int indicesCount;

    /**
     * The vertex index used for primitive restart. -1 means no restart.
     */
    GLint primitiveRestart;

    /**
     * The number of vertices per patch in this mesh, if #getMode() is PATCHES.
     */
    GLint patchVertices;

    /**
     * The MeshBuffers wrapped by this Mesh.
     */
    //mutable ptr<MeshBuffers> buffers;

    /**
     * Resizes the vertex array to expand its capacity.
     */
    void resizeVertices(int newSize);

    /**
     * Resizes the indice array to expand its capacity.
     */
    void resizeIndices(int newSize);

    /**
     * Creates the CPU of GPU buffers based on the current content of the
     * vertex and indice arrays.
     */
    void createBuffers() const;

    /**
     * Send the vertices to the GPU.
     */
    void uploadVertexDataToGPU(BufferUsage u) const;

    /**
     * Send the indices to the GPU.
     */
    void uploadIndexDataToGPU(BufferUsage u) const;

	bool cpu_data_clear;

    //friend class FrameBuffer;
};

template<class vertex, class index>
Mesh<vertex, index>::Mesh(MeshMode m, MeshUsage usage, int vertexCount, int indiceCount)
{
    vertices = new vertex[vertexCount];
    verticesLength = vertexCount;
    verticesCount = 0;
    indices = new index[indiceCount];
    indicesLength = indiceCount;
    indicesCount = 0;
    primitiveRestart = -1;
    patchVertices = 0;
    vertexDataHasChanged = true;
    indexDataHasChanged = true;
	cpu_data_clear = false;
}
/*
template<class vertex, class index>
Mesh<vertex, index>::Mesh(ptr<MeshBuffers> target, MeshMode m, MeshUsage usage, int vertexCount, int indiceCount) :
    Object("Mesh"), usage(usage), created(false), m(m)//, buffers(target)
{
    vertices = new vertex[vertexCount];
    verticesLength = vertexCount;
    verticesCount = 0;
    indices = new index[indiceCount];
    indicesLength = indiceCount;
    indicesCount = 0;
    primitiveRestart = -1;
    patchVertices = 0;
    vertexDataHasChanged = true;
    indexDataHasChanged = true;
	cpu_data_clear = false;
}
*/
template<class vertex, class index>
Mesh<vertex, index>::~Mesh()
{
	if(vertices)
    delete[] vertices;
	if(indices)
    delete[] indices;
}


template<class vertex, class index>
MeshMode Mesh<vertex, index>::getMode() const
{
    return m;
}

template<class vertex, class index>
int Mesh<vertex, index>::getVertexCount() const
{
    return verticesCount;
}

template<class vertex, class index>
vertex Mesh<vertex, index>::getVertex(int i) const
{
    return vertices[i];
}

template<class vertex, class index>
int Mesh<vertex, index>::getIndiceCount() const
{
    return indicesCount;
}

template<class vertex, class index>
GLint Mesh<vertex, index>::getPrimitiveRestart() const
{
    return primitiveRestart;
}

template<class vertex, class index>
GLint Mesh<vertex, index>::getPatchVertices() const
{
    return patchVertices;
}

template<class vertex, class index>
void Mesh<vertex, index>::uploadVertexDataToGPU(BufferUsage u) const
{
    //ptr<GPUBuffer> vb = vertexBuffer.cast<GPUBuffer>();
    //assert(vb != NULL); // check it's a GPU mesh
    //vb->setData(verticesCount * sizeof(vertex), vertices, u);
    vertexDataHasChanged = false;
}

template<class vertex, class index>
void Mesh<vertex, index>::uploadIndexDataToGPU(BufferUsage u) const
{
    //ptr<GPUBuffer> ib = indexBuffer.cast<GPUBuffer>();
    //assert(ib != NULL);
    //ib->setData(indicesCount * sizeof(index), indices, u);
    indexDataHasChanged = false;
}

//template<class vertex, class index>
//ptr<MeshBuffers> Mesh<vertex, index>::getBuffers() const
//{
//    if (!created) {
//        createBuffers();
//    }
//
//    if ((usage == GPU_DYNAMIC) || (usage == GPU_STREAM)) { // upload data to GPU if needed
//        BufferUsage u = usage == GPU_DYNAMIC ? DYNAMIC_DRAW : STREAM_DRAW;
//        if (vertexDataHasChanged) {
//            uploadVertexDataToGPU(u);
//        }
//        if ((indicesCount != 0) && indexDataHasChanged) {
//            uploadIndexDataToGPU(u);
//        }
//    }
//
//    buffers->primitiveRestart = primitiveRestart;
//    buffers->patchVertices = patchVertices;
//
//    return buffers;
//}

template<class vertex, class index>
void Mesh<vertex, index>::addAttributeType(int id, int size, AttributeType type, bool norm)
{
  //  buffers->addAttributeBuffer(id, size, sizeof(vertex), type, norm);
}

//////////////////////////////////////////////////////////////////////////
// xwt
//////////////////////////////////////////////////////////////////////////
template<class vertex, class index>
void Mesh<vertex, index>::addAttributeType(int id, int size, AttributeType type, bool norm, int divisor)
{
	//buffers->addAttributeBuffer(id, size, sizeof(vertex), type, norm, divisor);
}
template<class vertex, class index>
void Mesh<vertex, index>::setCapacity(int vertexCount, int indiceCount)
{
    if (verticesCount < vertexCount) {
        resizeVertices(vertexCount);
		verticesCount = vertexCount;//sz add 12-4-21
    }
    if (indicesCount < indiceCount) {
        resizeIndices(indiceCount);
		indicesCount = indiceCount;
    }
}

template<class vertex, class index>
void Mesh<vertex, index>::addVertex(const vertex &v)
{
    if (verticesCount == verticesLength) {
        resizeVertices(2 * verticesLength);
    }
    vertices[verticesCount++] = v;
    vertexDataHasChanged = true;
}

template<class vertex, class index>
void Mesh<vertex, index>::addVertices(const vertex *v, int count)
{
    for (int i = 0; i < count; ++i) {
        addVertex(v[i]);
    }
}

template<class vertex, class index>
void Mesh<vertex, index>::addIndice(index i)
{
    if (indicesCount == indicesLength) {
        resizeIndices(2 * indicesLength);
    }
    indices[indicesCount++] = i;
    indexDataHasChanged = true;
}

template<class vertex, class index>
void Mesh<vertex, index>::setMode(MeshMode mode)
{
    m = mode;
}

template<class vertex, class index>
void Mesh<vertex, index>::setVertex(int i, const vertex &v)
{
    vertices[i] = v;
    vertexDataHasChanged = true;
}

template<class vertex, class index>
void Mesh<vertex, index>::setIndice(int i, index ind)
{
    indices[i] = ind;
    indexDataHasChanged = true;
}

template<class vertex, class index>
void Mesh<vertex, index>::setPrimitiveRestart(GLint restart)
{
    primitiveRestart = restart;
}

template<class vertex, class index>
void Mesh<vertex, index>::setPatchVertices(GLint vertices)
{
    patchVertices = vertices;
}

template<class vertex, class index>
void Mesh<vertex, index>::resizeVertices(int newSize)
{
    vertex *newVertices = new vertex[newSize];
    memcpy(newVertices, vertices, verticesLength * sizeof(vertex));
    delete[] vertices;
    vertices = newVertices;
    verticesLength = newSize;
	
    if (created) {
        //buffers->reset();
        created = false;
    }
}

template<class vertex, class index>
void Mesh<vertex, index>::resizeIndices(int newSize)
{
    index *newIndices = new index[newSize];
    memcpy(newIndices, indices, indicesLength * sizeof(index));
    delete[] indices;
    indices = newIndices;
    indicesLength = newSize;
    if (created) {
       // buffers->reset();
        created = false;
    }
}

template<class vertex, class index>
void Mesh<vertex, index>::clear()
{
    verticesCount = 0;
    indicesCount = 0;
    vertexDataHasChanged = true;
    indexDataHasChanged = true;
    if (created) {
      //  buffers->reset();
       // buffers->setIndicesBuffer(NULL);
        created = false;
    }
}

template<class vertex, class index>
bool Mesh<vertex, index>::isCPUDataClearMode()
{
	return cpu_data_clear;
}

template<class vertex, class index>
void Mesh<vertex, index>::setCPUDataClearMode(bool whether)
{
	cpu_data_clear = whether;
}

template<class vertex, class index>
void Mesh<vertex, index>::clearBuffers()
{
    /*if (created) {
        buffers->reset();
        created = false;
    }*/
}

template<class vertex, class index>
void Mesh<vertex, index>::createBuffers() const
{
    /*if (usage == GPU_STATIC || usage == GPU_DYNAMIC || usage ==  GPU_STREAM) {
        GPUBuffer *gpub = new GPUBuffer();
        vertexBuffer = ptr<Buffer>(gpub);
        if (usage == GPU_STATIC) {
            uploadVertexDataToGPU(STATIC_DRAW);
			if(cpu_data_clear)
			{
				delete[] vertices;
				vertices = NULL;
			}
        }
    } else if (usage == CPU) {
        CPUBuffer *cpub = new CPUBuffer(vertices);
        vertexBuffer = ptr<Buffer>((Buffer*)cpub);
    }

    assert(buffers->getAttributeCount() > 0);
    for (int i = 0; i < buffers->getAttributeCount(); ++i) {
        buffers->getAttributeBuffer(i)->setBuffer(vertexBuffer);
    }

    if (indicesCount != 0) {
        if (usage == GPU_STATIC || usage == GPU_DYNAMIC || usage == GPU_STREAM) {
            GPUBuffer *gpub = new GPUBuffer();
            indexBuffer = ptr<Buffer>(gpub);
            if (usage == GPU_STATIC) {
                uploadIndexDataToGPU(STATIC_DRAW);
				if(cpu_data_clear)
				{
					delete[] indices;
					indices = NULL;
				}
            }
        } else if (usage == CPU) {
            CPUBuffer *cpub = new CPUBuffer(indices);
            indexBuffer = ptr<Buffer>(cpub);
        }

        AttributeType type;
        switch (sizeof(index)) {
        case 1:
            type = A8UI;
            break;
        case 2:
            type = A16UI;
            break;
        default:
            type = A32UI;
            break;
        }
        buffers->setIndicesBuffer(new AttributeBuffer(0, 1, type, false, indexBuffer));
    }
    buffers->mode = m;
    buffers->nvertices = verticesCount;
    buffers->nindices = indicesCount;
    created = true;*/
}

}

#endif
