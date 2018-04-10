#pragma once
#include "VectorMath.h"
#include "TextureManager.h"
#include "GpuBuffer.h"

using namespace Math;

typedef bool BlockProcessFunc(class MowModel* pModel, FILE* file);

bool MeshProcess(MowModel* pModel, FILE * file);
bool SkinProcess(MowModel* pModel, FILE * file);
bool VertProcess(MowModel* pModel, FILE * file);
bool IndxProcess(MowModel* pModel, FILE * file);

class MowModel
{
public:
	typedef struct _BoundingBox
	{
		XMFLOAT3 min;
		XMFLOAT3 max;
	}BoundingBox;

	typedef struct _EplyHeader
	{
		char magicK[8];
		BoundingBox boundingBox;
	} EplyHeader;

	typedef struct _ModelInfo
	{
		EplyHeader header;
		uint32_t meshCount;
		uint32_t materialCount;
		uint32_t vertexDataByteSize;
		uint32_t indexDataByteSize;
		uint32_t vertexDataByteSizeDepth;
		void Zero() {
			meshCount = 0;
			materialCount = 0;
			vertexDataByteSize = 0;
			indexDataByteSize = 0;
			vertexDataByteSizeDepth = 0;
			header.boundingBox.min = { 0.0f, 0.0f, 0.0f };// Vector3(0.0f);
			header.boundingBox.max = { 0.0f, 0.0f, 0.0f };// Vector3(0.0f);

		}
	}ModelInfo;

	typedef struct _Mesh
	{
		BoundingBox boundingBox;

		unsigned int materialIndex;

		unsigned int attribsEnabled;
		unsigned int attribsEnabledDepth;
		unsigned int vertexStride;
		unsigned int vertexStrideDepth;

		unsigned int vertexDataByteOffset;
		unsigned int vertexCount;
		unsigned int indexDataByteOffset;
		unsigned int indexCount;

		unsigned int vertexDataByteOffsetDepth;
		unsigned int vertexCountDepth;
	}Mesh;

	typedef struct _Material
	{
		Vector3 diffuse;
		Vector3 specular;
		Vector3 ambient;
		Vector3 emissive;
		Vector3 transparent; // light passing through a transparent surface is multiplied by this filter color
		float opacity;
		float shininess; // specular exponent
		float specularStrength; // multiplier on top of specular color

		enum { maxTexPath = 128 };
		enum { texCount = 6 };
		char texDiffusePath[maxTexPath];
		char texSpecularPath[maxTexPath];
		char texEmissivePath[maxTexPath];
		char texNormalPath[maxTexPath];
		char texLightmapPath[maxTexPath];
		char texReflectionPath[maxTexPath];

		enum { maxMaterialName = 128 };
		char name[maxMaterialName];
	}Material;

	const Mesh& Meshs(uint32_t index) const { return *m_pMesh.at(index); }
	const Material& Materials(uint32_t index) const { return m_pMaterial[index]; }
	const StructuredBuffer& VertexBuffer() const { return m_VertexBuffer; }
	const ByteAddressBuffer& IndexBuffer() const { return m_IndexBuffer; }
	const uint16_t& VertexStride() const { return m_VertexStride; }

protected:
	ModelInfo m_ModelInfo;
	std::vector<Mesh*> m_pMesh;
	Material *m_pMaterial;
	StructuredBuffer m_VertexBuffer;
	ByteAddressBuffer m_IndexBuffer;
	uint16_t m_VertexStride;

	// optimized for depth-only rendering
	unsigned char *m_pVertexDataDepth;
	unsigned char *m_pIndexDataDepth;
	StructuredBuffer m_VertexBufferDepth;
	ByteAddressBuffer m_IndexBufferDepth;
	uint32_t m_VertexStrideDepth;
	D3D12_CPU_DESCRIPTOR_HANDLE * m_SRVs;

public:
	MowModel();
	~MowModel();

	void Clear();

protected:
	void ComputeMeshBoundingBox(unsigned int meshIndex, BoundingBox &bbox) const;
	void ComputeGlobalBoundingBox(BoundingBox &bbox) const;
	void ComputeAllBoundingBoxes();

	void LoadTextures();
	void ReleaseTextures();
	bool LoadAS2Ply(LPCWSTR filename);
	typedef std::map <std::string, BlockProcessFunc* > BlockProcessMapsType;
	BlockProcessMapsType m_BlockProcessMaps;
	friend BlockProcessFunc MeshProcess, SkinProcess, VertProcess, IndxProcess;

public:
	virtual bool Load(LPCWSTR filename) { return LoadAS2Ply(filename); }
	const BoundingBox& GetBoundingBox() const
	{
		return m_ModelInfo.header.boundingBox;
	}
	const uint32_t MeshesCount() const { return m_ModelInfo.meshCount; }
	const uint32_t MaterialsCount() const { return m_ModelInfo.materialCount; }
	D3D12_CPU_DESCRIPTOR_HANDLE* GetSRVs(uint32_t materialIdx) const
	{
		return m_SRVs + materialIdx * 6;
	}
};
