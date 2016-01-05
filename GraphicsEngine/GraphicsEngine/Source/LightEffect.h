﻿#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include "DirectXMath.h"
#include "Material.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Technique.h"
#include "Texture.h"
#include "BufferTypes.h"

namespace GraphicsEngine
{
	class LightEffect
	{
	public:
		struct CameraConstantBuffer
		{
			DirectX::XMFLOAT4X4 ViewProjectionMatrix;
		};
		struct SubsetConstantBuffer
		{
			Material Material;
		};
		struct FrameConstantBuffer
		{
			DirectionalLight DirectionalLight;
			PointLight PointLight;
			SpotLight SpotLight;
			DirectX::XMFLOAT3 EyePositionW;
			float Pad;
		};

		struct InstanceData
		{
			DirectX::XMFLOAT4X4 WorldMatrix;
		};

	public:
		LightEffect();
		LightEffect(ID3D11Device* d3dDevice);
		
		void Initialize(ID3D11Device* d3dDevice);
		void Reset();

		void UpdateCameraConstantBuffer(ID3D11DeviceContext1* d3dDeviceContext, const CameraConstantBuffer& buffer) const;
		void UpdateSubsetConstantBuffer(ID3D11DeviceContext1* d3dDeviceContext, const SubsetConstantBuffer& buffer) const;
		void UpdateFrameConstantBuffer(ID3D11DeviceContext1* d3dDeviceContext, const FrameConstantBuffer& buffer) const;		

		void Set(ID3D11DeviceContext1* d3dDeviceContext) const;

		static void SetTexture(ID3D11DeviceContext1* d3dDeviceContext, const Texture& texture);

	private:
		VertexShader m_vertexShader;
		PixelShader m_pixelShader;
		DynamicConstantBuffer m_cameraConstantBuffer;
		DynamicConstantBuffer m_subsetConstantBuffer;
		DynamicConstantBuffer m_frameConstantBuffer;
		SamplerState m_samplerState;
		Technique m_lightTechnique;
	};
}