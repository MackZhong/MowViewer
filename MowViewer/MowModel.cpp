#include "stdafx.h"
#include "MowModel.h"

WCHAR buf[128];

MowModel::MowModel()
{
	m_BlockProcessMaps["MESH"] = MeshProcess;
	m_BlockProcessMaps["SKIN"] = SkinProcess;
	m_BlockProcessMaps["VERT"] = VertProcess;
	m_BlockProcessMaps["INDX"] = IndxProcess;
	Clear();
}


MowModel::~MowModel()
{
	Clear();
}

void MowModel::Clear()
{
	m_VertexBuffer.Destroy();
	m_IndexBuffer.Destroy();
	m_VertexBufferDepth.Destroy();
	m_IndexBufferDepth.Destroy();

	for (std::vector < Mesh * > ::iterator m; m < m_pMesh.end(); m++)
		delete (*m);
	m_pMesh.clear();

	delete[] m_pMaterial;
	m_pMaterial = nullptr;

	delete[] m_pVertexDataDepth;
	delete[] m_pIndexDataDepth;

	m_pVertexDataDepth = nullptr;
	m_pIndexDataDepth = nullptr;

	ReleaseTextures();

	m_ModelInfo.Zero();
}

void MowModel::ComputeMeshBoundingBox(unsigned int meshIndex, BoundingBox & bbox) const
{
}

void MowModel::ComputeGlobalBoundingBox(BoundingBox & bbox) const
{
}

void MowModel::ComputeAllBoundingBoxes()
{
}

void MowModel::LoadTextures()
{
}

void MowModel::ReleaseTextures()
{
}

bool MowModel::LoadAS2Ply(LPCWSTR filename)
{
	Clear();

	size_t s = sizeof(BoundingBox);
	s = sizeof(XMFLOAT3);
	s = sizeof(XMFLOAT2);

	FILE *file = nullptr;
	if (0 != _wfopen_s(&file, filename, L"rb"))
		return false;

	bool ok = false;
	char entry[5] = { '\0' };
	if (1 == fread(&m_ModelInfo.header, sizeof(EplyHeader), 1, file) && strcmp("EPLYBNDS", m_ModelInfo.header.magicK) == 0) {
		while (1 == fread(entry, 4, 1, file)) {
			BlockProcessMapsType::iterator e = m_BlockProcessMaps.find(entry);
			if (m_BlockProcessMaps.end() == e) {
				swprintf_s(buf, L"Unsupported entry type: %hS\n", entry);
				OutputDebugStringW(buf);
				throw buf;
				break;
			}
			if (!e->second(this, file))
				break;
		}
	}
	fclose(file);

	return true;
}

bool MeshProcess(MowModel* pModel, FILE * file)
{
	pModel->m_ModelInfo.meshCount++;

	uint32_t u1;
	assert(1 == fread(&u1, 4, 1, file));
	swprintf_s(buf, L"Unknown 4 bytes:0x%08X, %f, %u at 0x%08X\n", u1, (float)u1, u1, ftell(file) - 4);
	OutputDebugStringW(buf);
	uint32_t u2;
	assert(1 == fread(&u2, 4, 1, file));
	swprintf_s(buf, L"Unknown 4 bytes:0x%08X, %f, %u at 0x%08X\n", u2, (float)u2, u2, ftell(file) - 4);
	OutputDebugStringW(buf);

	uint32_t trangles;
	assert(1 == fread(&trangles, 4, 1, file));
	swprintf_s(buf, L"Number of triangles : %u at 0x%08X\n", trangles, ftell(file) - 4);
	OutputDebugStringW(buf);
	enum _MaterialType {
		MT_644 = 0x0644, MT_604 = 0x0604, MT_404 = 0x0404, MT_704 = 0x0704, MT_744 = 0x0744, MT_C14 = 0x0C14
	};
	uint32_t materialType;
	assert(1 == fread(&materialType, 4, 1, file));
	swprintf_s(buf, L"Material type: 0x%04X at 0x%08X\n", materialType, ftell(file) - 4);
	OutputDebugStringW(buf);
	uint32_t u3;
	switch (materialType) {
	case MT_404:
	case MT_C14:
		break;
	default:
		assert(1 == fread(&u3, 4, 1, file));
		swprintf_s(buf, L"Unknown 4 bytes:0x%08X, %f, %u at 0x%08X\n", u3, (float)u3, u3, ftell(file) - 4);
		OutputDebugStringW(buf);
		break;
	}
	uint8_t materialNameLength;
	assert(1 == fread(&materialNameLength, 1, 1, file));
	swprintf_s(buf, L"Material name length: %u at 0x%08X\n", materialNameLength, ftell(file) - 1);
	OutputDebugStringW(buf);
	char* materialName = new char[materialNameLength + 1]{ 0 };
	memset(materialName, 0, materialNameLength + 1);
	assert(1 == fread(materialName, materialNameLength, 1, file));
	swprintf_s(buf, L"Material file: %hS at 0x%08X\n", materialName, ftell(file) - materialNameLength);
	OutputDebugStringW(buf);
	delete[] materialName;
	pModel->m_ModelInfo.materialCount++;
	if (MT_C14 == materialType) {
		byte u4[3] = { 0 };
		assert(1 == fread(u4, 3, 1, file));
		swprintf_s(buf, L"Unknown 3 bytes:%02x,%02x,%02x at 0x%08X\n", u4[0], u4[1], u4[2], ftell(file) - 3);
		OutputDebugStringW(buf);
	}

	return true;
}

bool SkinProcess(MowModel * pModel, FILE * file)
{
	uint32_t skinsCount;
	assert(1 == fread(&skinsCount, 4, 1, file));
	swprintf_s(buf, L"Number of skins: %u at 0x%08X\n", skinsCount, ftell(file) - 4);
	OutputDebugStringW(buf);
	for (uint32_t s = 0; s < skinsCount; s++) {
		uint8_t skinNameLength;
		assert(1 == fread(&skinNameLength, 1, 1, file));
		swprintf_s(buf, L"Skin name length: %u at 0x%08X\n", skinNameLength, ftell(file) - 1);
		OutputDebugStringW(buf);
		char* skinName = new char[skinNameLength + 1]{ 0 };
		memset(skinName, 0, skinNameLength + 1);
		assert(1 == fread(skinName, skinNameLength, 1, file));
		swprintf_s(buf, L"Material file: %hS at 0x%08X\n", skinName, ftell(file) - skinNameLength);
		OutputDebugStringW(buf);
		delete[] skinName;
	}
	return true;
}

bool VertProcess(MowModel * pModel, FILE * file)
{
	//	for i in range(0, verts) :
	//		if vertex_description == 0x00010024 :
	//			vx, vy, vz, nx, ny, nz, U, V = struct.unpack("ffffff4xff", f.read(36))
	//		elif vertex_description == 0x00070020 :
	//			vx, vy, vz, nx, ny, nz, U, V = struct.unpack("ffffffff", f.read(32))
	//		elif vertex_description == 0x00070028 :
	//			vx, vy, vz, nx, ny, nz, U, V = struct.unpack("ffffffff8x", f.read(40))
	//		elif vertex_description == 0x00070030 :
	//			vx, vy, vz, nx, ny, nz, U, V = struct.unpack("ffffffff16x", f.read(48))
	//		else:
	//			raise Exception("Unknown format: %s" % hex(vertex_description))
	//		if verbose :
	//			print("Vertex %i: " % i, vx, vy, vz)
	//		self.positions.append((vx, vy, vz))
	//		self.normals.append((nx, ny, nz))
	//		if not self.translate_uv_y :
	//			self.UVs.append((U, V))
	//		else:
	//			self.UVs.append((U, V + 1.0))
	//	print("Vertex info ends at:", hex(f.tell()))
	uint32_t vertsCount;
	assert(1 == fread(&vertsCount, 4, 1, file));
	swprintf_s(buf, L"Number of vertices: %u at 0x%08X\n", vertsCount, ftell(file) - 4);
	OutputDebugStringW(buf);

	assert(1 == fread(&pModel->m_VertexStride, 2, 1, file));
	swprintf_s(buf, L"Vertex stride: 0x%08X at 0x%08X\n", pModel->m_VertexStride, ftell(file) - 2);
	OutputDebugStringW(buf);

	uint16_t u1;
	assert(1 == fread(&u1, 2, 1, file));
	swprintf_s(buf, L"Unknown 2 bytes:0x%04X, %f, %u at 0x%08X\n", u1, (float)u1, u1, ftell(file) - 2);
	OutputDebugStringW(buf);

	pModel->m_ModelInfo.vertexDataByteSize = vertsCount * pModel->m_VertexStride;
	byte* pBuff = new byte[pModel->m_ModelInfo.vertexDataByteSize];
	assert(1 == fread(pBuff, pModel->m_ModelInfo.vertexDataByteSize, 1, file));
	pModel->m_VertexBuffer.Create(L"VertexBuffer", vertsCount, pModel->m_VertexStride, pBuff);
	delete[] pBuff;
	assert(nullptr != pModel->m_VertexBuffer.GetResource());
	swprintf_s(buf, L"Vertex data ends at: 0x%08X\n", ftell(file) - 1);
	OutputDebugStringW(buf);

	return true;
}

bool IndxProcess(MowModel * pModel, FILE * file)
{
	//	idx_count, = struct.unpack("<I", f.read(4))
	//	print("Indeces:", idx_count)
	//	for i in range(0, idx_count / 3) :
	//		i0, i1, i2 = struct.unpack("<HHH", f.read(6))
	//		if verbose :
	//			print("Face %i:" % i, i0, i1, i2)
	//		if self.material_info == 0x0744 :
	//			self.indeces.append((i2, i1, i0))
	//		else :
	//			self.indeces.append((i0, i1, i2))
	//	print("Indces end at", hex(f.tell() - 1))
	uint32_t indxCount;
	assert(1 == fread(&indxCount, 4, 1, file));
	swprintf_s(buf, L"Number of indices: %u at 0x%08X\n", indxCount, ftell(file) - 4);
	OutputDebugStringW(buf);

	pModel->m_ModelInfo.indexDataByteSize = indxCount * sizeof(uint16_t);
	byte* pBuff = new byte[pModel->m_ModelInfo.indexDataByteSize];
	assert(1 == fread(pBuff, pModel->m_ModelInfo.indexDataByteSize, 1, file));
	pModel->m_IndexBuffer.Create(L"IndexBuffer", indxCount, sizeof(uint16_t), pBuff);
	delete[] pBuff;
	assert(nullptr != pModel->m_IndexBuffer.GetResource());
	swprintf_s(buf, L"Index data ends at: 0x%08X\n", ftell(file) - 1);
	OutputDebugStringW(buf);

	return true;
}
