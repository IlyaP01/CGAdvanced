#include "PBRWidget.h"

#include "ImGui/imgui.h"

PBRWidget::PBRWidget()
	: m_mode(PBRMode::Full)
{}

PBRWidget::PBRWidget(PBRParams & params)
	: m_params(params)
{}

void PBRWidget::AddToRender()
{
	if (ImGui::Begin("PBR Settings"))
	{
		ImGui::RadioButton("Full", reinterpret_cast<int*>(&m_mode), static_cast<int>(PBRMode::Full));
		ImGui::RadioButton("Normal distribution", reinterpret_cast<int*>(&m_mode), static_cast<int>(PBRMode::NormalDistribution));
		ImGui::RadioButton("Geometry", reinterpret_cast<int*>(&m_mode), static_cast<int>(PBRMode::Geometry));
		ImGui::RadioButton("Fresnel", reinterpret_cast<int*>(&m_mode), static_cast<int>(PBRMode::Fresnel));
		ImGui::SliderFloat("Roughness", &m_params.roughness, 0, 1);
		ImGui::SliderFloat("Metalness ", &m_params.metalness, 0, 1);
		ImGui::Text("Albedo");
		ImGui::SetNextItemWidth(50);
		ImGui::InputFloat("##albedo0", &m_params.albedo.x, 0, 1, "%.3f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::InputFloat("##albedo1", &m_params.albedo.y, 0, 1, "%.3f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::InputFloat("##albedo2", &m_params.albedo.z, 0, 1, "%.3f");
	}
	ImGui::End();
}

PBRParams PBRWidget::GetParams() const
{
	return m_params;
}

PBRMode PBRWidget::GetMode() const
{
	return m_mode;
}