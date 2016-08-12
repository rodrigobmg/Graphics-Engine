﻿#pragma once

#include "D3DBase.h"
#include "Technique.h"
#include "Timer.h"
#include "MathHelper.h"

namespace GraphicsEngine
{
	class Graphics
	{
	public:
		explicit Graphics(HWND outputWindow, uint32_t clientWidth, uint32_t clientHeight);

		void OnResize(uint32_t clientWidth, uint32_t clientHeight);
		void Update(const Timer& timer);
		void Render(const Timer& timer);
		
	private:
		void InitializeGeometry(const D3DBase& d3dBase);
		void UpdateProjectionMatrix();

	private:
		D3DBase m_d3d;
		Technique m_technique;
		
		std::unique_ptr<MeshGeometry> m_boxGeometry;
		
		DirectX::XMFLOAT4X4 m_modelMatrix = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 m_viewMatrix = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 m_projectionMatrix = MathHelper::Identity4x4();

		float m_theta = 1.5f * DirectX::XM_PI;
		float m_phi = DirectX::XM_PIDIV4;
		float m_radius = 5.0f;
	};
}
