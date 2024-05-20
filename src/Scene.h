#pragma once
#include <memory>
#include <vector>
#include "Drawable.h"


class Scene : public Drawable
{
public:
	template <typename DrawT, typename... argsT>
	void addDrawable(argsT... params);
	//void addLight();
	void clear();
	void update(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr < ID3D11DeviceContext> const& pContext);
	void render(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr < ID3D11DeviceContext> const&  pContext);

private:

	void initResurses(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr < ID3D11DeviceContext> const& pContext);

	std::vector<std::shared_ptr<Drawable>> m_objects;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;
	static constexpr wchar_t const * const m_vsPath = L"VertexShader.cso";
};

template <typename DrawT, typename... argsT>
void Scene::addDrawable(argsT... params)
{
	m_objects.push_back(std::shared_ptr<Drawable>(new DrawT(params...)));
}