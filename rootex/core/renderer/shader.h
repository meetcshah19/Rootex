#pragma once

#include "buffer_format.h"
#include "common/common.h"
#include "constant_buffer.h"
#include "rendering_device.h"
#include "texture.h"

class Shader
{
protected:
	LPCWSTR m_VertexPath;
	LPCWSTR m_PixelPath;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;

	Shader(const LPCWSTR& vertexPath, const LPCWSTR& pixelPath, const BufferFormat& vertexBufferFormat);

	friend class ShaderLibrary;

public:
	virtual ~Shader();

	enum class Type
	{
		Vertex,
		Pixel
	};

	enum class VertexConstantBufferType
	{
		Model,
		ModelInverse,
		View,
		Projection
	};

	virtual void bind() const;
	void unbind() const;

	void set(const VertexConstantBufferType& type, const Matrix& constantBuffer);
	void set(const PSDiffuseConstantBuffer& constantBuffer) const;
	void set(const PSSolidConstantBuffer& constantBuffer);
};

class DiffuseShader : public Shader
{
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerState;

	DiffuseShader(const LPCWSTR& vertexPath, const LPCWSTR& pixelPath, const BufferFormat& vertexBufferFormat);
	DiffuseShader(DiffuseShader&) = delete;
	~DiffuseShader() = default;

	friend class ShaderLibrary;

public:
	virtual void bind() const override;
	
	void set(const Texture* texture);
};

class CPUParticlesShader : public Shader
{
	CPUParticlesShader(const LPCWSTR& vertexPath, const LPCWSTR& pixelPath, const BufferFormat& vertexBufferFormat);
	CPUParticlesShader(CPUParticlesShader&) = delete;
	~CPUParticlesShader() = default;

	friend class ShaderLibrary;

public:
	virtual void bind() const override;
};
