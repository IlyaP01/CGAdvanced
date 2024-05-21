#include "Scene.h"
#include <d3dcompiler.h>


void Scene::clear()
{
	m_objects = {};
}

void Scene::update(
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr < ID3D11DeviceContext> const& pContext)
{
	if (m_pVertexShader == nullptr || m_pInputLayout == nullptr)
		initResurses(pDevice, pContext);
	else
	{
		pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);
		pContext->IASetInputLayout(m_pInputLayout.Get());
	}
}

void Scene::render(Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext, PBRPixelShader* pixelShader)
{
	if (m_pVertexShader == nullptr || m_pInputLayout == nullptr)
		initResurses(pDevice, pContext);
	for (auto& obj : m_objects)
		obj->render(pDevice, pContext, pixelShader);
}

void Scene::initResurses(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	// create vertex shader
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	THROW_IF_FAILED(DrawError, D3DReadFileToBlob(m_vsPath, &pBlob));
	THROW_IF_FAILED(DrawError, pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pVertexShader));

	// input (vertex) layout (2d position only)
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "Normal",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12u,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};
	THROW_IF_FAILED(DrawError, pDevice->CreateInputLayout(
		ied, (UINT)std::size(ied),
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		&m_pInputLayout
	));

	update(pDevice, pContext);
}

void Scene::setPBRParams(UINT idx, PBRParams params)
{
	assert(idx < m_objects.size());
	m_objects[idx]->setPBRParams(params);
}