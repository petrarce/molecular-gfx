/*	DrawMeshData.cpp
	Copyright 2012-2017 Fabian Herb

	This file is part of Molecular Engine.
*/

#include "DrawMeshData.h"
#include <molecular/gfx/DefaultProgramData.h>
#include <molecular/gfx/MeshDataSource.h>
#include <molecular/gfx/Material.h>
#include <molecular/meshfile/MeshFile.h>

namespace molecular
{
namespace Gfx
{

DrawMeshData::~DrawMeshData()
{
	Unload();
	for(auto buffer: mVertexBuffers)
		mRenderer.DestroyVertexBuffer(buffer);
	for(auto buffer: mIndexBuffers)
		mRenderer.DestroyIndexBuffer(buffer);
}

void DrawMeshData::Execute()
{
	if(mMeshes.empty())
		return; // No meshes loaded, do nothing

	for(auto& mesh: mMeshes)
	{
		if(mesh.material)
			mesh.material->Bind();

		auto& attributes = mVertexDataSets.at(mesh.info.vertexDataSet).attributes;
		BindAttributesAndDraw(mesh, attributes.begin(), attributes.end());

		if(mesh.material)
			mesh.material->Unbind();
	}
}

void DrawMeshData::Load(MeshDataSource& source)
{
	unsigned int numVertexBuffers = source.PrepareVertexData();
	unsigned int numIndexBuffers = source.PrepareIndexData();
	unsigned int numVertexDataSets = source.GetNumVertexDataSets();

	mVertexDataSets.resize(numVertexDataSets);
	for(unsigned int i = 0; i < numVertexDataSets; ++i)
	{
		mVertexDataSets[i].attributes = source.GetVertexBufferInfos(i);
	}

	for(auto& info: source.GetIndexBufferInfos())
	{
		Mesh mesh;
		mesh.info = info;
		if(info.material[0] == 0)
			mesh.material = nullptr;
		else
			mesh.material = mMaterialManager.GetMaterial(info.material);
		mMeshes.push_back(mesh);
	}

	mVertexBuffers.resize(numVertexBuffers);
	for(unsigned int i = 0; i < numVertexBuffers; ++i)
	{
		mVertexBuffers[i] = mRenderer.CreateVertexBuffer();
		mVertexBuffers[i]->Store(source.VertexBufferData(i), source.VertexBufferSize(i));
	}

	mIndexBuffers.resize(numIndexBuffers);
	for(unsigned int i = 0; i < numIndexBuffers; ++i)
	{
		mIndexBuffers[i] = mRenderer.CreateIndexBuffer();
		mIndexBuffers[i]->Store(source.IndexBufferData(i), source.IndexBufferSize(i));
	}
	mBounds = source.GetBounds();
}

void DrawMeshData::Load(const MeshFile& file)
{
	if(file.magic != MeshFile::kMagic)
		throw std::runtime_error("Mesh file magic number does not match");
	if(file.version != MeshFile::kVersion)
		throw std::runtime_error("Wrong mesh file version");
	if(file.numVertexDataSets > 100)
		throw std::runtime_error("Implausible number of vertex datasets");
	if(file.numIndexSpecs > 100)
		throw std::runtime_error("Implausible number of index specifications");
	if(file.numBuffers > 100)
		throw std::runtime_error("Implausible number of buffers");

	mVertexDataSets.resize(file.numVertexDataSets);
	for(unsigned int i = 0; i < file.numVertexDataSets; ++i)
	{
		const MeshFile::VertexDataSet& vSet = file.GetVertexDataSet(i);
		mVertexDataSets[i].attributes.resize(vSet.numVertexSpecs);
		for(unsigned int vSpec = 0; vSpec < vSet.numVertexSpecs; ++vSpec)
			mVertexDataSets[i].attributes[vSpec] = file.GetVertexSpec(i, vSpec);
	}

	mMeshes.resize(file.numIndexSpecs);
	for(unsigned int i = 0; i < file.numIndexSpecs; ++i)
	{
		const IndexBufferInfo& info = file.GetIndexSpec(i);
		mMeshes[i].info = info;
		if(info.material[0] == 0)
			mMeshes[i].material = nullptr;
		else
			mMeshes[i].material = mMaterialManager.GetMaterial(info.material);
	}

	mVertexBuffers.resize(file.numBuffers);
	mIndexBuffers.resize(file.numBuffers);
	for(unsigned int i = 0; i < file.numBuffers; ++i)
	{
		/* TODO: mVertexBuffers and mIndexBuffers combined contain twice as
			many buffers as necessary, but they are referenced correctly. */
		if(file.buffers[i].type == MeshFile::Buffer::Type::kVertex)
		{
			mVertexBuffers[i] = mRenderer.CreateVertexBuffer();
			mVertexBuffers[i]->Store(file.GetBufferData(i), file.buffers[i].size);
		}
		else if(file.buffers[i].type == MeshFile::Buffer::Type::kIndex)
		{
			mIndexBuffers[i] = mRenderer.CreateIndexBuffer();
			mIndexBuffers[i]->Store(file.GetBufferData(i), file.buffers[i].size);
		}
	}
	mBounds = util::AxisAlignedBox(file.boundsMin, file.boundsMax);
}

void DrawMeshData::Unload()
{
	mMeshes.clear();
	mIndexBuffers.clear();
	mVertexBuffers.clear();

	// Reset bounding box?
}

void DrawMeshData::Draw(Mesh& mesh)
{
	Binding<Uniform<Hash>> blendMode("blendMode"_H, this);
	bool add = (**blendMode == "add"_H);
	bool mix = (**blendMode == "mix"_H);

	if(add || mix)
	{
		mRenderer.SetDepthState(true, false);
		if(add)
			mRenderer.SetBlending(true, RenderCmdSink::kOne, RenderCmdSink::kOne);
		else if(mix)
			mRenderer.SetBlending(true, RenderCmdSink::kSrcAlpha, RenderCmdSink::kOneMinusSrcAlpha);
	}

	PrepareProgram();
	mRenderer.Draw(mIndexBuffers.at(mesh.info.buffer), mesh.info);

	if(add || mix)
	{
		mRenderer.SetBlending(false);
		mRenderer.SetDepthState(true, true);
	}
}

void DrawMeshData::BindAttributesAndDraw(Mesh& mesh, std::vector<VertexAttributeInfo>::iterator begin, std::vector<VertexAttributeInfo>::iterator end)
{
	if(begin == end)
		Draw(mesh);
	else
	{
		Binding<Attribute> attrBinding(begin->semantic, this);
		*attrBinding = Attribute(mVertexBuffers.at(begin->buffer), *begin);
		BindAttributesAndDraw(mesh, begin + 1, end);
	}
}

}
}
