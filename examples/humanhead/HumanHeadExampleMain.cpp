/*	HumanHeadExampleMain.cpp

MIT License

Copyright (c) 2019 Fabian Herb

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <molecular/gfx/glfw/GlfwContext.h>
#include <molecular/gfx/glfw/GlfwFileLoader.h>
#include <molecular/gfx/glfw/GlfwRenderManager.h>
#include <molecular/gfx/glfw/GlfwWindow.h>
#include <molecular/gfx/DefaultProgramData.h>
#include <molecular/gfx/SkinProgramData.h>
#include <molecular/gfx/functions/DrawMesh.h>
#include <molecular/gfx/functions/Transform.h>
#include <molecular/gfx/functions/ShadowMapping.h>
#include <molecular/gfx/functions/SceneSetup.h>
#include <molecular/gfx/functions/DrawTerrain.h>
#include <molecular/gfx/functions/ApplyTextures.h>
#include <molecular/gfx/functions/HumanSkin.h>
#include <molecular/gfx/functions/ViewSetup.h>
#include <molecular/gfx/RenderManager.h>

#include <molecular/util/CommandLineParser.h>
#include <molecular/util/FileServer.h>
#include <molecular/util/Matrix4.h>
#include <molecular/util/Matrix3.h>
#include <molecular/util/Math.h>
#include <molecular/util/TaskDispatcher.h>
#include <molecular/util/Vector.h>

#include <iostream>

using namespace molecular;
using namespace molecular::gfx;
using RenderManager = GlfwRenderManager;

void Run(int argc, char** argv)
{
	CommandLineParser cmd;
	CommandLineParser::Option<std::string> assetDir(cmd, "asset-dir", "Asset directory", "assets");
	cmd.Parse(argc, argv);

	// Boilerplate:
	TaskDispatcher dispatcher;
	GlfwFileLoader fileLoader(dispatcher);
	FileServer<GlfwFileLoader> fileServer(fileLoader, *assetDir, dispatcher);
	GlfwWindow window;
	GlfwContext context(window.GetWindow());
	RenderCmdSink commandSink;
	RenderManager renderManager(context, fileServer, dispatcher, commandSink);

	Blob shaderDataFile = fileServer.ReadFileSync("shaderdata/default.txt"_H, dispatcher);
	renderManager.LoadProgramFile(shaderDataFile);
	shaderDataFile = fileServer.ReadFileSync("shaderdata/skin.txt"_H, dispatcher);
	renderManager.LoadProgramFile(shaderDataFile);

	// Render graph construction:
	ViewSetup viewSetup(renderManager);
	SceneSetup sceneSetup(renderManager);
	ShadowMapping shadowMapping(renderManager);
	Transform transform(renderManager);
	ApplyTextures applyTextures(renderManager);
	HumanSkin humanSkin(renderManager);
	DrawMesh<RenderManager> drawMesh(renderManager);

	viewSetup.SetCallee(&sceneSetup);
	sceneSetup.SetCallee(&shadowMapping);
	shadowMapping.SetCallee(&transform);
	transform.SetCallee(&applyTextures);
	applyTextures.SetCallee(&humanSkin);
	humanSkin.SetCallee(&drawMesh);

	// Parameters:
	viewSetup.SetCamera(Vector3(0, 0, 0.4f), Quaternion::kIdentity);
	sceneSetup.SetDirectionalLight(true, Vector3(1,-1,-1));
	viewSetup.SetProjectionPerspective(0.3f * Math::kPi_f, 1, 0.1f);

	drawMesh.SetMeshFile("nv/geo_Skin1.nmb"_H);

	applyTextures.SetTexture("diffuseTexture"_H, "nv/AdrianAlbedo.tga"_H);
	applyTextures.SetTexture("normalMapObjectSpaceTexture"_H, "nv/AdrianFinalNormal.tga"_H);
	applyTextures.SetTexture("specularTexture"_H, "nv/AdrianSpec.tga"_H);
	applyTextures.SetTexture("rhoDTexture"_H, "nv/rho_d.tga"_H);
	applyTextures.SetParameter("rhoDTexture"_H, RenderCmdSink::Texture::kWrapS, RenderCmdSink::Texture::kClampToEdge);
	applyTextures.SetParameter("rhoDTexture"_H, RenderCmdSink::Texture::kWrapT, RenderCmdSink::Texture::kClampToEdge);

	humanSkin.SetStretchTexture("nv/FaceStretchXY.tga"_H);

	Scoping& scoping = renderManager.GetScoping();
//	Scoping::Binding<Uniform<float> > shininess(DefaultProgramData::kShininess, scoping);
	Scoping::Binding<Uniform<Vector4>> specularColor("specularColor"_H, scoping);

//	**shininess = 16;
	**specularColor = Vector4(0.719795f, 0.575374f, 0.245268f, 1.0f);

	// Render:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::cerr << "Error: " << glGetError() << std::endl;

	float angle = 0;
	while(!window.ShouldClose())
	{
		angle += 0.005f;
		transform.SetTransform(Matrix4::Scale(0.001f) * Matrix4::Translation(0,-0.65f, 0.0) * Matrix4::RotationY(angle));
		glfwMakeContextCurrent(window.GetWindow());
		renderManager.DrawOneFrame(sceneSetup);
		glfwSwapBuffers(window.GetWindow());

		GLenum error = glGetError();
		if(error)
		{
			std::cerr << "Error: " << error << std::endl;
			break;
		}
	}
}

int main(int argc, char** argv)
{
	try
	{
		Run(argc, argv);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}