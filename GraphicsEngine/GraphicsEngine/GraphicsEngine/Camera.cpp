﻿#include "stdafx.h"
#include "Camera.h"

using namespace DirectX;
using namespace GraphicsEngine;

Camera::Camera() :
	m_position(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f)),
	m_rotationQuaternion(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f)),
	m_rotationMatrix(),
	m_dirty(true),
	m_aspectRatio(2.0f),
	m_fovAngleY(70.0f * XM_PI / 180.0f),
	m_nearZ(0.01f),
	m_farZ(10000.0f)
{
	// Initialize projection matrix, given orientation matrix equals to the identity matrix:
	InitializeProjectionMatrix(XMMatrixIdentity());
}

Camera::Camera(float aspectRatio, float fovAngleY, float nearZ, float farZ, CXMMATRIX orientationMatrix) :
	m_position(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f)),
	m_rotationQuaternion(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f)),
	m_rotationMatrix(),
	m_dirty(true),
	m_aspectRatio(aspectRatio),
	m_fovAngleY(fovAngleY),
	m_nearZ(nearZ),
	m_farZ(farZ)
{
	InitializeProjectionMatrix(orientationMatrix);
}

void Camera::Update()
{
	if (m_dirty)
	{
		// Update the rotation matrix:
		m_rotationMatrix = XMMatrixRotationQuaternion(m_rotationQuaternion);

		// Build view matrix:
		auto up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		auto forward = m_rotationMatrix.r[2];
		m_viewMatrix = XMMatrixLookToLH(m_position, forward, up);

		m_dirty = false;
	}
}

void Camera::Move(FXMVECTOR axis, float scalar)
{
	// Calculate the translation:
	auto translation = XMVectorScale(axis, scalar);

	// Apply the translation:
	m_position = XMVectorAdd(m_position, translation);

	m_dirty = true;
}
void Camera::MoveRight(float scalar)
{
	// Translate along the X-axis:
	const auto& right = XMVectorSet(XMVectorGetX(m_viewMatrix.r[0]), XMVectorGetX(m_viewMatrix.r[1]), XMVectorGetX(m_viewMatrix.r[2]), 0.0f);
	Move(right, scalar);
}
void Camera::MoveForward(float scalar)
{
	// Translate along the Z-axis:
	const auto& forward = m_rotationMatrix.r[2];
	Move(forward, scalar);
}

void Camera::Rotate(FXMVECTOR axis, float radians)
{
	// Calculate the rotation arround the axis:
	auto rotation = XMQuaternionRotationNormal(axis, radians);

	// Apply the rotation:
	m_rotationQuaternion = XMQuaternionMultiply(m_rotationQuaternion, rotation);

	m_dirty = true;
}
void Camera::RotateLocalX(float radians)
{
	// Calculate the rotation arround the camera local X-axis:
	const auto& right = XMVectorSet(XMVectorGetX(m_viewMatrix.r[0]), XMVectorGetX(m_viewMatrix.r[1]), XMVectorGetX(m_viewMatrix.r[2]), 0.0f);
	Rotate(right, radians);
}
void Camera::RotateWorldY(float radians)
{
	// Calculate the rotation arround the world Y-axis:
	static const auto WORLD_Y_AXIS = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	Rotate(WORLD_Y_AXIS, radians);
}

BoundingFrustum Camera::BuildViewSpaceBoundingFrustum() const
{
	BoundingFrustum boundingFrustum;
	BoundingFrustum::CreateFromMatrix(boundingFrustum, m_projectionMatrix);

	return boundingFrustum;
}

const XMVECTOR& Camera::GetPosition() const
{
	return m_position;
}
const XMMATRIX& Camera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}
const XMVECTOR& Camera::GetForward() const
{
	return m_rotationMatrix.r[2];
}

Ray Camera::CreateRay() const
{
	return Ray(m_position, GetForward());
}

float Camera::GetNearZ() const
{
	return m_nearZ;
}
float Camera::GetFarZ() const
{
	return m_farZ;
}

const XMMATRIX& Camera::GetViewMatrix() const
{
	return m_viewMatrix;
}

void Camera::SetPosition(float x, float y, float z)
{
	m_position = XMVectorSet(x, y, z, 1.0f);
	m_dirty = true;
}
void Camera::SetAspectRatio(float aspectRatio)
{
	m_aspectRatio = aspectRatio;
	InitializeProjectionMatrix(XMMatrixIdentity());
}

bool Camera::IsDirty() const
{
	return m_dirty;
}

void Camera::InitializeProjectionMatrix(FXMMATRIX orientationMatrix)
{
	// Build a perspective matrix:
	auto perspectiveMatrix = XMMatrixPerspectiveFovLH(
		m_fovAngleY,
		m_aspectRatio,
		m_nearZ,
		m_farZ
		);

	// Create a projection matrix by multiplying the perspective matrix with the orientation matrix:
	m_projectionMatrix = perspectiveMatrix * orientationMatrix;
}
