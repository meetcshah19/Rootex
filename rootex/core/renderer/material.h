#pragma once

#include "constant_buffer.h"
#include "shader.h"

class Material
{
protected:
	Shader* m_Shader;
	Vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> m_PSConstantBuffer;
	Vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> m_VSConstantBuffer;
	String m_FileName;
	String m_TypeName;

	Material(Shader* shader, const String& typeName);

public:
	template <typename T>
	static void setPSConstantBuffer(const T& constantBuffer, Microsoft::WRL::ComPtr<ID3D11Buffer>& pointer, UINT slot);
	template <typename T>
	static void SetVSConstantBuffer(const T& constantBuffer, Microsoft::WRL::ComPtr<ID3D11Buffer>& pointer, UINT slot);

	Material() = delete;
	virtual ~Material() = default;

	virtual void bind();
	
	String getFileName() { return m_FileName; };
	String getTypeName() { return m_TypeName; };
	String getFullName() { return m_FileName + " - " + m_TypeName; };
	virtual JSON::json getJSON() const;
	
	void setFileName(const String& fileName) { m_FileName = fileName; };
	
#ifdef ROOTEX_EDITOR
	virtual void draw(const String& id);
#endif // ROOTEX_EDITOR
};

template <typename T>
void Material::setPSConstantBuffer(const T& constantBuffer, Microsoft::WRL::ComPtr<ID3D11Buffer>& bufferPointer, UINT slot)
{
	if (bufferPointer == nullptr)
	{
		D3D11_BUFFER_DESC cbd = { 0 };
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(T);
		cbd.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA csd = { 0 };
		csd.pSysMem = &constantBuffer;

		bufferPointer = RenderingDevice::GetSingleton()->createPSConstantBuffer(&cbd, &csd);
		RenderingDevice::GetSingleton()->setPSConstantBuffer(bufferPointer.Get(), slot);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE subresource;
		RenderingDevice::GetSingleton()->mapBuffer(bufferPointer.Get(), subresource);
		memcpy(subresource.pData, &constantBuffer, sizeof(constantBuffer));
		RenderingDevice::GetSingleton()->unmapBuffer(bufferPointer.Get());

		RenderingDevice::GetSingleton()->setPSConstantBuffer(bufferPointer.Get(), slot);
	}
}

template <typename T>
void Material::SetVSConstantBuffer(const T& constantBuffer, Microsoft::WRL::ComPtr<ID3D11Buffer>& bufferPointer, UINT slot)
{
	if (bufferPointer == nullptr)
	{
		D3D11_BUFFER_DESC cbd = { 0 };
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(T);
		cbd.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA csd = { 0 };
		csd.pSysMem = &constantBuffer;

		bufferPointer = RenderingDevice::GetSingleton()->createVSConstantBuffer(&cbd, &csd);
		RenderingDevice::GetSingleton()->setVSConstantBuffer(bufferPointer.Get(), slot);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE subresource;
		RenderingDevice::GetSingleton()->mapBuffer(bufferPointer.Get(), subresource);
		memcpy(subresource.pData, &constantBuffer, sizeof(constantBuffer));
		RenderingDevice::GetSingleton()->unmapBuffer(bufferPointer.Get());

		RenderingDevice::GetSingleton()->setVSConstantBuffer(bufferPointer.Get(), slot);
	}
}
