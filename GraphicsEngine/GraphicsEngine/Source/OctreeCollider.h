﻿#pragma once

#include <DirectXCollision.h>

namespace GraphicsEngine
{
	class OctreeBaseCollider
	{
	public:
		virtual ~OctreeBaseCollider()
		{
		}

		virtual bool Intersects(const DirectX::BoundingBox& box) const = 0;
		virtual bool Intersects(const DirectX::BoundingFrustum& frustum) const = 0;
		virtual void FrustumCull(bool frustumCulled) = 0;
	};

	template<
		typename Type,
		typename = std::enable_if_t<
		std::is_same<Type, DirectX::BoundingBox>::value ||
		std::is_same<Type, DirectX::BoundingSphere>::value ||
		std::is_same<Type, DirectX::BoundingOrientedBox>::value ||
		std::is_same<Type, DirectX::BoundingFrustum>::value
		>
	>
	class OctreeCollider : public OctreeBaseCollider
	{
	public:
		OctreeCollider() :
			m_collider(),
			m_frustumCulled(false)
		{
		}
		explicit OctreeCollider(const Type& collider) :
			m_collider(collider),
			m_frustumCulled(false)
		{
		}
		explicit OctreeCollider(Type&& collider) :
			m_collider(std::move(collider)),
			m_frustumCulled(false)
		{
		}

		bool Intersects(const DirectX::BoundingBox& box) const override
		{
			return box.Intersects(m_collider);
		}
		bool Intersects(const DirectX::BoundingFrustum& frustum) const override
		{
			return frustum.Intersects(m_collider);
		}

		bool IsFrustumCulled() const
		{
			return m_frustumCulled;
		}
		void FrustumCull(bool frustumCulled) override
		{
			m_frustumCulled = frustumCulled;
		}

	private:
		Type m_collider;
		bool m_frustumCulled;
	};
}