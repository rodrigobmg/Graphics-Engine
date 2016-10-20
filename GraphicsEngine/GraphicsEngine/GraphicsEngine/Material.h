#pragma once

#include "Common/MathHelper.h"

#include <DirectXMath.h>
#include <string>

namespace GraphicsEngine
{
	struct Material
	{
		std::string Name;
		int MaterialIndex = -1;
		int DiffuseSrvHeapIndex = -1;
		int NormalSrvHeapIndex = -1;

		// Material constant buffer data used for shading:
		DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
		float Roughness = 0.25f;
		DirectX::XMFLOAT4X4 MaterialTransform = MathHelper::Identity4x4();
	};
}
