#include "../../util/path.hpp"
#include "assimp_loader.h"
#include <assimp/cimport.h>
#include <assimp/cfileio.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/config.h>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>
#include <iostream>
#include <stdexcept>

#include "../../util/compat/os.macro.hpp"
#ifdef SS_UTIL_ANDROID_COMPATIBLE
    #include "../../util/compat/os.undef.hpp"
    #include "../../lib/native.hpp"
    #include "../../util/unreachable.macro.hpp"

    namespace ss
    {
        namespace
        {
            inline namespace assimp_io
            {
                void flush(aiFile*) { SS_UTIL_UNREACHABLE(); }
                auto write(aiFile*, char const*, size_t, size_t) -> size_t { SS_UTIL_UNREACHABLE(); }

                auto read(aiFile* file, char* buf, size_t size, size_t count) -> size_t
                {
                    auto asset = (android::Asset*)file->UserData;
                    int n = 0;
                    for (; n<int(count); n++) {
                        if (asset->read(buf, size) != size) break;
                        buf += size;
                    }
                    return n;
                }

                auto seek(aiFile* file, size_t offset, aiOrigin whence) -> aiReturn
                {
                    auto asset = (android::Asset*)file->UserData;
                    switch (whence) {
                        case aiOrigin_SET: asset->seek_front(); break;
                        case aiOrigin_END: asset->seek_back(); break;
                        case aiOrigin_CUR: break;
                        default: SS_UTIL_UNREACHABLE();
                    }
                    asset->seek(int(offset));
                    return aiReturn_SUCCESS;
                }

                auto tell(aiFile* file) -> size_t
                {
                    auto asset = (android::Asset*)file->UserData;
                    return asset->tell();
                }

                auto file_size(aiFile* file) -> size_t
                {
                    auto asset = (android::Asset*)file->UserData;
                    return asset->size();
                }

                auto open(aiFileIO*, char const* path, char const* mode) -> aiFile*
                {
                    auto asset = new android::Asset(path);
                    auto ud = (aiUserData)asset;
                    return new aiFile{
                        &read,
                        &write,
                        &tell,
                        &file_size,
                        &seek,
                        &flush,
                        ud,
                    };
                }

                void close(aiFileIO*, aiFile* file)
                {
                    auto asset = (android::Asset*)file->UserData;
                    delete asset;
                    delete file;
                }

                aiFileIO filesystem_callbacks{
                    &open,
                    &close,
                };
                auto ai_filesystem = &filesystem_callbacks;
            }
        }
    }

    #include "../../util/unreachable.undef.hpp"
#else
    #include "../../util/compat/os.undef.hpp"

    namespace ss
    {
        namespace
        {
            aiFileIO* ai_filesystem = nullptr;
        }
    }
#endif

Matrix4 convert_assimp_matrix(aiMatrix4x4 m) {
    return Matrix4{
        Vector4{m.a1, m.b1, m.c1, m.d1},
        Vector4{m.a2, m.b2, m.c2, m.d2},
        Vector4{m.a3, m.b3, m.c3, m.d3},
        Vector4{m.a4, m.b4, m.c4, m.d4},
    };
}

// assume one scene contains only one model,but allow multi mesh
// so load and combine bones to one skeleton
bool LoadSceneSkeleton(const aiScene* scene,as_Skeleton* skeleton){
    //create mesh skeleton
    map<string, aiBone*> bone_map;
    map<string, string>  parent_map;//用于记录节点的父子关系
    aiNode* rootnode = scene->mRootNode;
    aiNode* skeleton_root = nullptr;
    for (int m_i = 0; m_i < (int)scene->mNumMeshes; m_i++) {
        aiMesh* mesh = scene->mMeshes[m_i];
        if (mesh->HasBones()){
            skeleton->name = mesh->mName.C_Str();
            for (int i = 0; i < (int)mesh->mNumBones; i++){
                aiBone* bone = mesh->mBones[i];
                bone_map[bone->mName.C_Str()] = bone;
                aiNode* node = rootnode->FindNode(bone->mName);
                while (true){
                    aiNode* parent = node->mParent;
                    if (parent == rootnode){
                        skeleton_root = node;
                        break;
                    }
                    parent_map[node->mName.C_Str()] = parent->mName.C_Str();
                    node = parent;
                }
            }
        }
    }
    if (!(skeleton_root && bone_map.size() > 0))
        return false;
    assert(skeleton_root);
    assert(bone_map.size() > 0);
    as_Bone* root = new as_Bone(skeleton_root->mName.C_Str(), {}, {}, 0, nullptr);
    skeleton->root = root;
    skeleton->bones.push_back(root);
    int boneid = 0;
    skeleton->bone_map[root->name] = root;
    for (auto it : bone_map){
        boneid++;
        auto aibone = it.second;
        as_Bone* bone = new as_Bone(aibone->mName.C_Str(), convert_assimp_matrix(aibone->mOffsetMatrix), {}, boneid, nullptr);
        skeleton->bone_map[it.first] = bone;
        skeleton->bones.push_back(bone);
    }
    skeleton->num_bones = (int)skeleton->bones.size();
    //find their parent
    for (int i = 1; i < (int)skeleton->bones.size(); i++){
        auto bone = skeleton->bones[i];
        as_Bone* parent = skeleton->bone_map[parent_map[bone->name]];
        bone->parent = parent;
        parent->children.push_back(bone);
    }
    return true;
}

void FillBoneIdWeights(const aiScene*scene, as_Skeleton* skeleton, vector<as_Geometry*>& geo){
    //create boneId and bone Weights;
    for (int m_i = 0; m_i < (int)scene->mNumMeshes; m_i++) {
        aiMesh* mesh = scene->mMeshes[m_i];
        as_Geometry* geometry = geo[m_i];
        geometry->boneID = vector<int>(geometry->mNumVertices*MaxBonePerVertex, 0);
        geometry->boneWeight = vector<float>(geometry->mNumVertices*MaxBonePerVertex, 0);
        vector<int> offset = vector<int>(geometry->mNumVertices, 0);//代表vertex已经填充了几个weight
        if (mesh->HasBones()){
            for (int i = 0; i < (int)mesh->mNumBones; i++){
                aiBone* bone = mesh->mBones[i];
                for (int j = 0; j < (int)bone->mNumWeights; j++){
                    const aiVertexWeight & weight = bone->mWeights[j];
                    int vertexId = weight.mVertexId;
                    int offset_t = offset[vertexId];
                    offset[vertexId] += 1;
                    geometry->boneID[vertexId * 4 + offset_t] = skeleton->bone_map[bone->mName.C_Str()]->id;
                    geometry->boneWeight[vertexId * 4 + offset_t] = weight.mWeight;

                }
            }
        }
    }
}

Vector3 aiVector3D_to_Vector3(aiVector3D& v){
    return Vector3(v.x, v.y, v.z);
}
Quaternion aiQuaternion_to_quanternion(aiQuaternion& q){
    return Quaternion{q.w, q.x, q.y, q.z};
}
// load mesh position,normal,indices
void LoadMeshBasicAttributes(const aiMesh* mesh, as_Geometry* geometry,Vector3& sMax,Vector3& sMin){

    geometry->mNumVertices = mesh->mNumVertices;
    geometry->name = mesh->mName.C_Str();
    for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
        if (mesh->HasPositions()) {
            auto& vp = mesh->mVertices[v_i];
            geometry->position.push_back(vp.x);
            geometry->position.push_back(vp.y);
            geometry->position.push_back(vp.z);
            auto p = Vector3{vp.x, vp.y, vp.z};
            sMin = min(sMin, p);
            sMax = max(sMax, p);
        }
        if (mesh->HasNormals()) {
            const aiVector3D* vn = &(mesh->mNormals[v_i]);
            geometry->normal.push_back(vn->x);
            geometry->normal.push_back(vn->y);
            geometry->normal.push_back(vn->z);

        }
        if (mesh->HasTangentsAndBitangents()) {
            auto vn = &(mesh->mTangents[v_i]);
            geometry->tangent.push_back(vn->x);
            geometry->tangent.push_back(vn->y);
            geometry->tangent.push_back(vn->z);

            vn = &(mesh->mBitangents[v_i]);
            geometry->bitangent.push_back(vn->x);
            geometry->bitangent.push_back(vn->y);
            geometry->bitangent.push_back(vn->z);

        }
        if (mesh->HasTextureCoords(0)) {
            const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
            geometry->texcoord.push_back(vt->x);
            geometry->texcoord.push_back(vt->y);

        }

    }
    if (mesh->HasFaces()){
        geometry->mNumFaces = mesh->mNumFaces;
        for (int i = 0; i < (int)mesh->mNumFaces; i++){
            aiFace* face = &mesh->mFaces[i];
            geometry->indices.push_back(face->mIndices[0]);
            geometry->indices.push_back(face->mIndices[1]);
            geometry->indices.push_back(face->mIndices[2]);
        }
    }
}

//
void readAnimation(aiAnimation* ai_anim, as_SkeletonAnimation* my_anim){
    my_anim->name = ai_anim->mName.C_Str();
    my_anim->duration = (float)ai_anim->mDuration;
    my_anim->tickPerSecond = (float)ai_anim->mTicksPerSecond;
    my_anim->numChannels = ai_anim->mNumChannels;
    for (int i = 0; i < (int)ai_anim->mNumChannels; i++){
        auto node = ai_anim->mChannels[i];
        as_NodeAnimation* mynode = new as_NodeAnimation;

        mynode->name = node->mNodeName.C_Str();
        mynode->positionKeys = vector<vectorKey>(node->mNumPositionKeys);
        mynode->rotateKeys = vector<quaterionKey>(node->mNumRotationKeys);
        mynode->scaleKeys = vector<vectorKey>(node->mNumScalingKeys);
        mynode->numPositionkeys = node->mNumPositionKeys;
        mynode->numRotatekeys = node->mNumRotationKeys;
        mynode->numScalekeys = node->mNumScalingKeys;
        for (int i = 0; i < (int)node->mNumPositionKeys; i++){
            mynode->positionKeys[i].time = (float)node->mPositionKeys[i].mTime;
            mynode->positionKeys[i].value = aiVector3D_to_Vector3(node->mPositionKeys[i].mValue);
        }
        for (int i = 0; i < (int)node->mNumRotationKeys; i++){
            mynode->rotateKeys[i].time = (float)node->mRotationKeys[i].mTime;
            mynode->rotateKeys[i].quat = aiQuaternion_to_quanternion(node->mRotationKeys[i].mValue);
        }
        for (int i = 0; i < (int)node->mNumScalingKeys; i++){
            mynode->scaleKeys[i].time = (float)node->mScalingKeys[i].mTime;
            mynode->scaleKeys[i].value = aiVector3D_to_Vector3(node->mScalingKeys[i].mValue);
        }
        my_anim->channels.push_back(mynode);
    }
}
Vector3 aiColortoVector3(aiColor3D& c){
    return Vector3(c.r, c.g, c.b);
}
void readMaterial(aiMaterial* mat,as_Material* my_mat){
    aiColor3D color(0.f, 0.f, 0.f);
    aiString name;
    mat->Get(AI_MATKEY_NAME, name);
    my_mat->name = name.C_Str();
    mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    my_mat->diffuse = aiColortoVector3(color);
    mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
    my_mat->specular = aiColortoVector3(color);
    mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
    my_mat->ambient = aiColortoVector3(color);
    mat->Get(AI_MATKEY_COLOR_TRANSPARENT, color);
    my_mat->transparent = aiColortoVector3(color);
    mat->Get(AI_MATKEY_COLOR_EMISSIVE, color);
    my_mat->emissive = aiColortoVector3(color);

    mat->Get(AI_MATKEY_SHININESS, my_mat->shininess);
    mat->Get(AI_MATKEY_OPACITY, my_mat->opacity);

    aiString path;
    vector<aiTextureType> textype = { aiTextureType_DIFFUSE ,aiTextureType_SPECULAR,aiTextureType_AMBIENT,aiTextureType_OPACITY,
        aiTextureType_HEIGHT ,aiTextureType_EMISSIVE ,aiTextureType_NORMALS,aiTextureType_SHININESS,aiTextureType_DISPLACEMENT,
        aiTextureType_REFLECTION ,aiTextureType_LIGHTMAP };

    vector<string>  textype2 = { "diffuse" ,"specular" ,"ambient","opacity","height","emissive" ,"normal" ,"shininess" ,"displacement"
    ,"reflection" ,"lightmap" };
    for (int i = 0; i < (int)textype.size(); i++) {
        if (aiReturn_SUCCESS == mat->GetTexture(textype[i], 0, &path)) {

            my_mat->textures[textype2[i]] = as_TexturePath(string(name.C_Str()) + textype2[i]);
            my_mat->textures[textype2[i]].path = path.C_Str();
        }
    }
}

bool Import3DFromFile(
    ss::util::Sanitized_Runtime_Path const& filepath,
    std::vector<as_Mesh*>& meshs,
    std::vector<as_Geometry*>& geo,
    std::vector<as_Skeleton*>& skeletons,
    std::vector<as_SkeletonAnimation*>& ani,
    std::vector<as_Material*>& materials,
    Vector3& sMax,
    Vector3& sMin)
{
    unsigned int post_process_flags =
        aiProcess_CalcTangentSpace |            // calculate tangents and bitangents if possible
        aiProcess_JoinIdenticalVertices |       // join identical vertices/ optimize indexing
        aiProcess_ValidateDataStructure |       // perform a full validation of the loader's output
        aiProcess_ImproveCacheLocality |        // improve the cache locality of the output vertices
        aiProcess_RemoveRedundantMaterials |    // remove redundant materials
        aiProcess_FindInvalidData |             // detect invalid model data, such as invalid normal vectors
        aiProcess_TransformUVCoords |           // preprocess UV transformations (scaling, translation ...)
        aiProcess_LimitBoneWeights |            // limit bone weights to 4 per vertex
        aiProcess_OptimizeMeshes |              // join small meshes, if possible;
        aiProcess_GenSmoothNormals |            // generate smooth normal vectors if not existing
        aiProcess_Triangulate |                 // triangulate polygons with more than 3 edges
        0;
    float smoothAngle = 80.0f;
    bool no_points_lines = false;

    auto props = aiCreatePropertyStore();
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_TER_MAKE_UVS, 1);
    aiSetImportPropertyFloat(props, AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, smoothAngle);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_SBP_REMOVE,
        no_points_lines ? aiPrimitiveType_LINE | aiPrimitiveType_POINT : 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_GLOB_MEASURE_TIME, 1);

    auto scene = aiImportFileExWithProperties(
        filepath.unsafe_get(),
        post_process_flags,
        ::ss::ai_filesystem,
        props);

    if (scene == nullptr) {
        throw std::runtime_error{
            "Cannot read mesh " + filepath.unsafe_get_as_str() + ": " + aiGetErrorString()
        };
    }

    std::cerr << "Imported scene " << filepath.unsafe_get() << "\n";
    std::cerr << "  " << scene->mNumAnimations << " animations\n";
    std::cerr << "  " << scene->mNumCameras    << " cameras\n";
    std::cerr << "  " << scene->mNumLights     << " lights\n";
    std::cerr << "  " << scene->mNumMaterials  << " materials\n";
    std::cerr << "  " << scene->mNumMeshes     << " meshes\n";
    std::cerr << "  " << scene->mNumTextures   << " textures\n";

    sMax = Vector3(-1E15f, -1E15f, -1E15f);
    sMin = Vector3(1E15f, 1E15f, 1E15f);

    for (int m_i = 0; m_i < (int)scene->mNumMeshes; m_i++) {
        const aiMesh* aimesh = scene->mMeshes[m_i];
        std::cerr << "  " << aimesh->mNumVertices << " vertices\n";
        as_Geometry* geometry =new as_Geometry;;
        LoadMeshBasicAttributes(aimesh, geometry, sMax, sMin);

        {
            geometry->name = ss::util::basename(filepath.unsafe_get());
            if (m_i > 0) {
                geometry->name += "_";
                geometry->name += std::to_string(m_i);
            }
        }
        geo.push_back(geometry);
        as_Mesh* mymesh = new as_Mesh;
        mymesh->name = geometry->name;
        mymesh->geometry = geometry;
        mymesh->materialID = aimesh->mMaterialIndex;
        meshs.push_back(mymesh);
    }
    if (scene->HasAnimations()){
        as_Skeleton* skeleton = new as_Skeleton;
        bool ok=LoadSceneSkeleton(scene, skeleton);
        if (!ok)
            goto READ_MATERIAL;
        skeletons.push_back(skeleton);
        FillBoneIdWeights(scene, skeleton, geo);

        for (auto& g : geo){
            g->mSkeleton = skeleton;
            g->has_skeleton = 1;
        }

        for (int i = 0; i < (int)scene->mNumAnimations; i++){
            as_SkeletonAnimation* animation = new as_SkeletonAnimation;
            readAnimation(scene->mAnimations[i], animation);
            ani.push_back(animation);
        }
    }
    READ_MATERIAL:
    if (scene->HasMaterials()){
        for (int i = 0; i < (int)scene->mNumMaterials; i++){
            as_Material* mat = new as_Material;
            readMaterial(scene->mMaterials[i], mat);
            materials.push_back(mat);
        }
        for (int i = 0; i < (int)scene->mNumMeshes; i++) {
            meshs[i]->material = materials[meshs[i]->materialID];
        }
    }

    return true;
}

