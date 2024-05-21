#include "PBRWidget.h"

#include "ImGui/imgui.h"

PBRWidget::PBRWidget() = default;

PBRWidget::PBRWidget(PBRParams & params)
	: m_params(params)
{}

void PBRWidget::AddToRender()
{
	if (ImGui::Begin("PBR Settings"))
	{
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