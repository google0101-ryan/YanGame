#include <components/RenderModel.h>


CRenderModel::CRenderModel(const char *modelName)
{
    // Full path will be Assets/models/'modelName'.gltf
    m_name = modelName;

    m_Geometry = {};
}

void CRenderModel::Init(IGameObject *pParent)
{
    m_Geometry.pParent = pParent;

    // Load the 3D model

    auto& filesystem = g_pEngine->GetFileSystem();

    auto file = filesystem.OpenFileRead((std::string("models/") + m_name + ".gltf").c_str());

    if (!file)
    {
        MakeDefault();
        return;
    }

    // TODO: Loading GLTF models (not hard, just tired)
    LOG_FATAL("TODO: Actually load the damn thing\n");
}

void CRenderModel::Update()
{
    g_pEngine->GetRenderSystem().SubmitGeometry(&m_Geometry);
}

void AddCubeFace(Geometry &geo, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4)
{
    geo.pVerts[geo.iNumVerts+0] = {};
    geo.pVerts[geo.iNumVerts+0].pos = v1 * 8.0f;
    geo.pVerts[geo.iNumVerts+0].uv = glm::vec2(0, 0);

    geo.pVerts[geo.iNumVerts+1] = {};
    geo.pVerts[geo.iNumVerts+1].pos = v2 * 8.0f;
    geo.pVerts[geo.iNumVerts+1].uv = glm::vec2(1, 0);

    geo.pVerts[geo.iNumVerts+2] = {};
    geo.pVerts[geo.iNumVerts+2].pos = v3 * 8.0f;
    geo.pVerts[geo.iNumVerts+2].uv = glm::vec2(1, 1);

    geo.pVerts[geo.iNumVerts+3] = {};
    geo.pVerts[geo.iNumVerts+3].pos = v4 * 8.0f;
    geo.pVerts[geo.iNumVerts+3].uv = glm::vec2(0, 1);

    geo.pIndices[geo.iNumIndices+0] = geo.iNumVerts+0;
    geo.pIndices[geo.iNumIndices+1] = geo.iNumVerts+1;
    geo.pIndices[geo.iNumIndices+2] = geo.iNumVerts+2;
    geo.pIndices[geo.iNumIndices+3] = geo.iNumVerts+0;
    geo.pIndices[geo.iNumIndices+4] = geo.iNumVerts+2;
    geo.pIndices[geo.iNumIndices+5] = geo.iNumVerts+3;

    geo.iNumIndices += 6;
    geo.iNumVerts += 4;
}

void CRenderModel::MakeDefault()
{
    PurgeModel();

    m_Geometry.pIndices = new uint16_t[36];
    m_Geometry.pVerts = new DrawVert[24];
    
    AddCubeFace(m_Geometry, glm::vec3(-1, 1, 1), glm::vec3(1, 1, 1), glm::vec3(1, -1, 1), glm::vec3(-1, -1, 1));
	AddCubeFace(m_Geometry, glm::vec3(-1, 1, -1), glm::vec3(-1, -1, -1), glm::vec3(1, -1, -1), glm::vec3(1, 1, -1));

	AddCubeFace(m_Geometry, glm::vec3(1, -1, 1), glm::vec3(1, 1, 1), glm::vec3(1, 1, -1), glm::vec3(1, -1, -1));
	AddCubeFace(m_Geometry, glm::vec3(-1, -1, 1), glm::vec3(-1, -1, -1), glm::vec3(-1, 1, -1), glm::vec3(-1, 1, 1));

	AddCubeFace(m_Geometry, glm::vec3(-1, -1, 1), glm::vec3(1, -1, 1), glm::vec3(1, -1, -1), glm::vec3(-1, -1, -1));
	AddCubeFace(m_Geometry, glm::vec3(-1, 1, 1), glm::vec3(-1, 1, -1), glm::vec3(1, 1, -1), glm::vec3(1, 1, 1));
}

void CRenderModel::PurgeModel()
{
    if (m_Geometry.pIndices != nullptr)
        delete m_Geometry.pIndices;

    if (m_Geometry.pVerts != nullptr)
        delete m_Geometry.pVerts;

    m_Geometry.iNumIndices = 0;
    m_Geometry.iNumVerts = 0;
}