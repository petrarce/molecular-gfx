/*	ParticleSystem.h
	Copyright 2015-2017 Fabian Herb

	This file is part of Molecular Engine.
*/

#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <molecular/gfx/RenderFunction.h>
#include <molecular/util/StringUtils.h>
#include <molecular/gfx/TextureUtils.h>

namespace molecular
{
namespace Gfx
{

class ParticleSystem : public RenderFunction
{
public:
	template<class TRenderManager>
	ParticleSystem(TRenderManager& manager) :
		RenderFunction(manager),
		mBillboardProgram(manager.GetRenderer().CreateProgram()),
		mEmitterProgram(manager.GetRenderer().CreateProgram()),
		mFeedbackProgram(manager.GetRenderer().CreateProgram())
	{
		auto& srv = manager.GetFileServer();
		auto billboardVertFile = srv.Open("nvparticles/billboard.vert");
		auto billboardGeomFile = srv.Open("nvparticles/billboard.geom");
		auto billboardFragFile = srv.Open("nvparticles/billboard.frag");
		auto billboardVertSrc = StringUtils::FromStorage(billboardVertFile);
		auto billboardGeomSrc = StringUtils::FromStorage(billboardGeomFile);
		auto billboardFragSrc = StringUtils::FromStorage(billboardFragFile);
		RenderCmdSink::Program::ShaderSourceItem billboardItems[3] =
		{
			{RenderCmdSink::Program::kVertexShader, billboardVertSrc.data(), billboardVertSrc.size()},
			{RenderCmdSink::Program::kGeometryShader, billboardGeomSrc.data(), billboardGeomSrc.size()},
			{RenderCmdSink::Program::kFragmentShader, billboardFragSrc.data(), billboardFragSrc.size()}
		};
		mBillboardProgram->Store(billboardItems, 3);

		auto emitterVertFile = srv.Open("nvparticles/emitter_feedback.vert");
		auto emitterGeomFile = srv.Open("nvparticles/emitter_feedback.geom");
		auto emitterVertSrc = StringUtils::FromStorage(emitterVertFile);
		auto emitterGeomSrc = StringUtils::FromStorage(emitterGeomFile);
		InitProgram(mEmitterProgram, emitterVertSrc, emitterGeomSrc);

		auto feedbackVertFile = srv.Open("nvparticles/feedback.vert");
		auto feedbackGeomFile = srv.Open("nvparticles/feedback.geom");
		auto feedbackGeomSrc = StringUtils::FromStorage(feedbackGeomFile);
		auto feedbackVertSrc = StringUtils::FromStorage(feedbackVertFile);
		InitProgram(mFeedbackProgram, feedbackVertSrc, feedbackGeomSrc);

		mFbmTexture = TextureUtils::Fbm3DTexture(mRenderer, srv, kFbmTexSize);
		mParticleTexture = TextureUtils::SpotTexture(mRenderer, kParticleTexSize, kParticleTexSize);
		mRandomTexture = TextureUtils::Random1DTexture(mRenderer, kRandomTexSize);

		InitParticleBuffers();
	}
	~ParticleSystem();

	void Execute() override;
	util::AxisAlignedBox GetBounds() const override {return util::AxisAlignedBox();}

private:
	struct Particle
	{
		uint8_t reserved = 0;
		uint8_t color[3] = {0, 0, 0};
		float age = 0.0f;
		Vector3 position = {0.0f, 0.0f, 0.0f};
		Vector3 velocity = {0.0f, 0.0f, 0.0f};
	};

	struct Parameters
	{
		uint32_t emittersCount = 4;
		float    emitPeriod = 5.f/1000.f; //5ms
		uint32_t emitCount = 10;
		float    particleLifetime = 10;
		float    billboardSize = 0.01f;
		float    velocityScale = 0.0f;
		bool     freezeEmitters = false;
		bool     useColors = true;
	};

	void Draw();
	void InitProgram(RenderCmdSink::Program* program, const std::string& vertSrc, const std::string& geomSrc);
	void InitParticleBuffers();
	void AdvanceFeedback();
	void UpdateEmitters();
	void EmitParticles();
	void ProcessParticles();
	void DrawParticles();
	void MoveEmitters();
	void Update(float deltaTime);
	void SeedEmitters(unsigned int num);

	void ApplyAttributes(RenderCmdSink::Program* program, RenderCmdSink::VertexBuffer* buffer, bool applyVelocity = true);

	RenderCmdSink::Program* mBillboardProgram = nullptr;
	RenderCmdSink::Program* mEmitterProgram = nullptr;
	RenderCmdSink::Program* mFeedbackProgram = nullptr;
	RenderCmdSink::TransformFeedback* mParticleFeedbacks[2];
	RenderCmdSink::TransformFeedback* mEmitterFeedback;
	RenderCmdSink::VertexBuffer* mParticleBuffers[2];
	RenderCmdSink::VertexBuffer* mEmitterBuffer;
	RenderCmdSink::VertexBuffer* mEmitterFeedbackBuffer;
	RenderCmdSink::Texture* mRandomTexture = nullptr;
	RenderCmdSink::Texture* mParticleTexture = nullptr;
	RenderCmdSink::Texture* mFbmTexture = nullptr;
	int mCurrentBuffers = 0; // Ring

	bool mIsStopped = false;

	float mTime = 0.0f;
	float mDeltaTime;

	Parameters mParams;
	static const unsigned int kRandomTexSize = 2048;
	static const unsigned int kFbmTexSize = 256;
	static const unsigned int kParticleTexSize = 256;
	static const unsigned int kMaxEmitters = 32;
	static constexpr int kMaxParticles = 1024 * 1024;
	Particle mEmitters[kMaxEmitters];
	uint32_t mEmittersCount = 0;
//	uint32_t mParticleCount = 0;
	static const VertexAttributeInfo mVertexAttributeInfos[];
};

}
}

#endif // PARTICLESYSTEM_H
