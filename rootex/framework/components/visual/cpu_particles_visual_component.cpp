#include "cpu_particles_visual_component.h"

#include "random.h"
#include "systems/render_system.h"
#include "timer.h"

Component* CPUParticlesVisualComponent::Create(const JSON::json& componentData)
{
	ParticleTemplate particalTemplate {
		{ 
			componentData["velocity"]["x"], 
			componentData["velocity"]["y"], 
			componentData["velocity"]["z"] 
		},
		{ 
			componentData["angularVelocity"]["x"], 
			componentData["angularVelocity"]["y"], 
			componentData["angularVelocity"]["z"], 
			componentData["angularVelocity"]["w"] 
		},
		{ 
			componentData["colorBegin"]["r"], 
			componentData["colorBegin"]["g"], 
			componentData["colorBegin"]["b"], 
			componentData["colorBegin"]["a"] 
		},
		{ 
			componentData["colorEnd"]["r"], 
			componentData["colorEnd"]["g"], 
			componentData["colorEnd"]["b"], 
			componentData["colorEnd"]["a"] 
		},
		componentData["velocityVariation"],
		componentData["sizeBegin"],
		componentData["sizeEnd"],
		componentData["sizeVariation"],
		componentData["lifeTime"]
	};

	CPUParticlesVisualComponent* particles = new CPUParticlesVisualComponent(componentData["poolSize"], particalTemplate);
	return particles;
}

Component* CPUParticlesVisualComponent::CreateDefault()
{
	CPUParticlesVisualComponent* particles = new CPUParticlesVisualComponent(1000, ParticleTemplate());
	return particles;
}

CPUParticlesVisualComponent::CPUParticlesVisualComponent(size_t poolSize, const ParticleTemplate& particleTemplate)
    : VisualComponent(RenderPassMain, Ref<CPUParticlesMaterial>(new CPUParticlesMaterial()), nullptr)
    , m_QuadVertices({ 
		{ { -0.5f, -0.5f, 0.0f } }, // 0
          { { +0.5f, -0.5f, 0.0f } }, // 1
          { { +0.5f, +0.5f, 0.0f } }, // 2
          { { -0.5f, +0.5f, 0.0f } } }) // 3
    , m_QuadIndices({ 0, 1, 2, 2, 3, 0 })
    , m_ParticleTemplate(particleTemplate)
    , m_TransformComponent(nullptr)
{
	m_ParticlePool.resize(poolSize);
	m_PoolIndex = poolSize - 1;
	m_LastRenderTimePoint = std::chrono::high_resolution_clock::now();
	m_EmitRate = 0;
}

CPUParticlesVisualComponent::~CPUParticlesVisualComponent()
{
}

bool CPUParticlesVisualComponent::setup()
{
	m_TransformComponent = m_Owner->getComponent<TransformComponent>().get();
	if (!m_TransformComponent)
	{
		ERR("Transform Component not found on entity with CPU Particles Component: " + m_Owner->getFullName());
		return false;
	}
	return true;
}

bool CPUParticlesVisualComponent::preRender()
{
	VisualComponent::preRender();

	int i = m_EmitRate;
	while (i >= 0)
	{
		emit(m_ParticleTemplate);
		i--;
	}

	for (auto& particle : m_ParticlePool)
	{
		if (particle.m_LifeRemaining <= 0.0f)
		{
			particle.m_IsActive = false;
			continue;
		}
		if (!particle.m_IsActive)
		{
			continue;
		}

		float delta = (std::chrono::high_resolution_clock::now() - m_LastRenderTimePoint).count() * (NS_TO_MS * MS_TO_S);
		particle.m_LifeRemaining -= delta;
		particle.m_Position += particle.m_Velocity * delta;
		particle.m_Rotation += 0.5f * particle.m_AngularVelocity * delta * particle.m_Rotation; // https://gamedev.stackexchange.com/a/157018/106158
	}

	return true;
}

void CPUParticlesVisualComponent::render()
{
	for (auto& particle : m_ParticlePool)
	{
		if (!particle.m_IsActive)
		{
			continue;
		}

		float life = particle.m_LifeRemaining / particle.m_LifeTime;
		float size = particle.m_SizeBegin * (life) + particle.m_SizeEnd * (1.0f - life);

		Color color = Color::Lerp(particle.m_ColorEnd, particle.m_ColorBegin, life);

		RenderSystem::GetSingleton()->pushMatrix(Matrix::CreateScale(size) * Matrix::CreateFromQuaternion(particle.m_Rotation) * Matrix::CreateTranslation(particle.m_Position) * Matrix::Identity);
		m_Attributes.getMaterial()->setShaderConstantBuffer(Shader::VertexConstantBufferType::Model, RenderSystem::GetSingleton()->getTopMatrix());
		m_Attributes.getMaterial()->setShaderConstantBuffer(PSSolidConstantBuffer({ color }));
		RenderSystem::GetSingleton()->getRenderer()->draw(&m_QuadVertices, &m_QuadIndices, m_Attributes.getMaterial());
		RenderSystem::GetSingleton()->popMatrix();
	}
}

void CPUParticlesVisualComponent::postRender()
{
	VisualComponent::postRender();
	m_LastRenderTimePoint = std::chrono::high_resolution_clock::now();
}

void CPUParticlesVisualComponent::emit(const ParticleTemplate& particleTemplate)
{
	Particle& particle = m_ParticlePool[m_PoolIndex];

	particle.m_IsActive = true;
	particle.m_Position = m_TransformComponent->getPosition();
	particle.m_Rotation = m_TransformComponent->getRotation();

	particle.m_Velocity = particleTemplate.m_Velocity;
	particle.m_Velocity.x += particleTemplate.m_VelocityVariation * (Random::Float() - 0.5f);
	particle.m_Velocity.y += particleTemplate.m_VelocityVariation * (Random::Float() - 0.5f);
	particle.m_Velocity.z += particleTemplate.m_VelocityVariation * (Random::Float() - 0.5f);

	particle.m_AngularVelocity = particleTemplate.m_AngularVelocity;

	particle.m_ColorBegin = particleTemplate.m_ColorBegin;
	particle.m_ColorEnd = particleTemplate.m_ColorEnd;

	particle.m_LifeTime = particleTemplate.m_LifeTime;
	particle.m_LifeRemaining = particleTemplate.m_LifeTime;
	particle.m_SizeBegin = particleTemplate.m_SizeBegin + particleTemplate.m_SizeVariation * (Random::Float() - 0.5f);
	particle.m_SizeEnd = particleTemplate.m_SizeEnd;

	m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
}

JSON::json CPUParticlesVisualComponent::getJSON() const
{
	JSON::json j;

	j["poolSize"] = m_ParticlePool.size();
	j["velocity"]["x"] = m_ParticleTemplate.m_Velocity.x;
	j["velocity"]["y"] = m_ParticleTemplate.m_Velocity.y;
	j["velocity"]["z"] = m_ParticleTemplate.m_Velocity.z;
	j["velocityVariation"] = m_ParticleTemplate.m_VelocityVariation;
	j["angularVelocity"]["x"] = m_ParticleTemplate.m_AngularVelocity.x;
	j["angularVelocity"]["y"] = m_ParticleTemplate.m_AngularVelocity.y;
	j["angularVelocity"]["z"] = m_ParticleTemplate.m_AngularVelocity.z;
	j["angularVelocity"]["w"] = m_ParticleTemplate.m_AngularVelocity.w;
	j["colorBegin"]["r"] = m_ParticleTemplate.m_ColorBegin.x;
	j["colorBegin"]["g"] = m_ParticleTemplate.m_ColorBegin.y;
	j["colorBegin"]["b"] = m_ParticleTemplate.m_ColorBegin.z;
	j["colorBegin"]["a"] = m_ParticleTemplate.m_ColorBegin.w;
	j["colorEnd"]["r"] = m_ParticleTemplate.m_ColorEnd.x;
	j["colorEnd"]["g"] = m_ParticleTemplate.m_ColorEnd.y;
	j["colorEnd"]["b"] = m_ParticleTemplate.m_ColorEnd.z;
	j["colorEnd"]["a"] = m_ParticleTemplate.m_ColorEnd.w;
	j["sizeBegin"] = m_ParticleTemplate.m_SizeBegin;
	j["sizeEnd"] = m_ParticleTemplate.m_SizeEnd;
	j["sizeVariation"] = m_ParticleTemplate.m_SizeVariation;
	j["lifeTime"] = m_ParticleTemplate.m_LifeTime;

	return j;
}

#ifdef ROOTEX_EDITOR
#include "imgui.h"
void CPUParticlesVisualComponent::draw()
{
	ImGui::DragInt("Emit Rate", &m_EmitRate);
	ImGui::Separator();
	ImGui::Text("Particle", ImGuiTreeNodeFlags_CollapsingHeader);
	ImGui::DragFloat3("Velocity", &m_ParticleTemplate.m_Velocity.x);
	ImGui::DragFloat("Velocity Variation", &m_ParticleTemplate.m_VelocityVariation);
	ImGui::DragFloat4("Angular Velocity", &m_ParticleTemplate.m_AngularVelocity.x);
	ImGui::ColorEdit4("Color Begin", &m_ParticleTemplate.m_ColorBegin.x);
	ImGui::ColorEdit4("Color End", &m_ParticleTemplate.m_ColorEnd.x);
	ImGui::DragFloat("Size Begin", &m_ParticleTemplate.m_SizeBegin, 0.01f);
	ImGui::DragFloat("Size End", &m_ParticleTemplate.m_SizeEnd, 0.01f);
	ImGui::DragFloat("Size Variation", &m_ParticleTemplate.m_SizeVariation, 0.01f);
	ImGui::DragFloat("Lifetime", &m_ParticleTemplate.m_LifeTime, 0.01f);
}
#endif // ROOTEX_EDITOR
