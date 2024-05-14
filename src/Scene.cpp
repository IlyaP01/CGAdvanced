#include "Scene.h"
#include <d3dcompiler.h>


void Scene::clear()
{
	m_objects = {};
}

void Scene::render(Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext)
{
	if (m_pVertexShader == nullptr)
		initResurses(pDevice, pContext);
	for (auto& obj : m_objects)
		obj->render(pDevice, pContext);
}

void Scene::initResurses(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	// create vertex shader
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	THROW_IF_FAILED(DrawError, D3DReadFileToBlob(m_vsPath, &pBlob));
	THROW_IF_FAILED(DrawError, pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pVertexShader));

	// bind vertex shader
	pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);


	// input (vertex) layout (2d position only)
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "Color",0,DXGI_FORMAT_R8G8B8A8_UNORM,0,12u,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};
	THROW_IF_FAILED(DrawError, pDevice->CreateInputLayout(
		ied, (UINT)std::size(ied),
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		&pInputLayout
	));

	// bind vertex layout
	pContext->IASetInputLayout(pInputLayout.Get());
}