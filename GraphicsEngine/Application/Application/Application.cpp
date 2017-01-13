﻿#include "Application.h"
#include "GraphicsEngine/CameraAnimation.h"
#include "GraphicsEngine/FogAnimation.h"

#include <functional>

using namespace Common;
using namespace Win32Application;
using namespace GraphicsEngine;
using namespace DirectX;

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// Check if the window is being destroyed:
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// Check if the window is being closed:
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

		// All other messages pass to the message handler in the application class:
	default:
		auto application = Application::GetInstance();
		if (application)
			return application->MessageHandler(hwnd, msg, wParam, lParam);

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

Application* Application::GetInstance()
{
	if (!s_instance)
	{
		if (!s_mutex.try_lock())
			return nullptr;

		s_instance = std::make_unique<Application>();

		s_mutex.unlock();
	}

	return s_instance.get();
}

int Application::Run()
{
	MSG message = {};

	auto update = [this](const Timer& timer)
	{
		auto camera = m_graphics.GetCamera();
		auto scalar = 0.05f *  static_cast<float>(timer.GetMillisecondsPerUpdate());
		if (m_input.IsKeyDown(DIK_W))
			camera->MoveForward(scalar);
		if (m_input.IsKeyDown(DIK_S))
			camera->MoveForward(-scalar);
		if (m_input.IsKeyDown(DIK_A))
			camera->MoveRight(-scalar);
		if (m_input.IsKeyDown(DIK_D))
			camera->MoveRight(scalar);

		static const auto rotationSensibility = 0.005f;
		if (m_input.IsKeyDown(DIK_Q))
			camera->RotateWorldZ(-rotationSensibility);
		if (m_input.IsKeyDown(DIK_E))
			camera->RotateWorldZ(rotationSensibility);

		static const auto mouseSensibility = rotationSensibility;
		int mouseDeltaX, mouseDeltaY;
		m_input.GetMouseVelocity(mouseDeltaX, mouseDeltaY);
		camera->RotateWorldY(-mouseDeltaX * mouseSensibility);
		camera->RotateWorldX(-mouseDeltaY * mouseSensibility);

		//const auto& terrain = m_graphics.GetScene()->GetTerrain();

		// Clamp camera to the ground:
		/*XMFLOAT3 position;
		XMStoreFloat3(&position, camera->GetPosition());
		auto terrainHeight = m_graphics.GetScene()->GetTerrain().GetTerrainHeight(position.x, position.z);
		camera->SetPosition(position.x, 5.0f + terrainHeight, position.z);*/

		// Ray collision with terrain:
		/*{
			auto ray = camera->CreateRay();
			const std::vector<VertexTypes::PositionVertexType>* vertices;
			const std::vector<uint32_t>* indices;
			terrain.GetMeshData(vertices, indices);
			float distance;
			if (ray.IntersectsTriangleMesh<VertexTypes::PositionVertexType, uint32_t>(*vertices, *indices, distance))
			{
				XMFLOAT3 intersection;
				XMStoreFloat3(&intersection, ray.CalculatePoint(distance));
				auto debugString = L"Intersected Position: (" + std::to_wstring(intersection.x) + L", " + std::to_wstring(intersection.y) + +L", " + std::to_wstring(intersection.z) + L")\n";
				OutputDebugStringW(debugString.c_str());
			}
		}*/

		if(!m_animationBuildMode)
		{
			m_animationManager.FixedUpdate(m_timer);
		}

		m_graphics.FixedUpdate(m_timer);
	};

	auto render = [this](const Timer& timer)
	{
		m_graphics.RenderUpdate(timer);
		m_graphics.Render(timer);
	};

	auto processInput = [this]()
	{
		m_input.Update();

		// Exit application if exit button is pressed:
		if (m_input.IsKeyDown(DIK_ESCAPE))
			return false;

		if (m_input.IsKeyDown(DIK_T))
			m_soundManager.Play2DSound("TestSound", 1.0f);

		return true;
	};

	auto processFrameStatistics = [this](const Timer& timer)
	{
		auto camera = m_graphics.GetCamera();
		XMFLOAT3 cameraPosition;
		XMStoreFloat3(&cameraPosition, camera->GetPosition());

		std::wstringstream extraCaption;

		extraCaption << L"FPS: " << std::to_wstring(timer.GetFramesPerSecond());
		if(!m_animationBuildMode) extraCaption << L" | V: " << std::to_wstring(m_graphics.GetVisibleInstances());
		if(m_animationBuildMode) extraCaption << L" | Begin From Last Spot: " << (m_beginCameraAnimationFromLastSpot ? L"Yes" : L"No");
		extraCaption << L" | " << camera->ToWString();

		m_window.SetWindowExtraCaption(extraCaption.str());
	};

	m_timer.Reset();

	while (message.message != WM_QUIT)
	{
		// If there are Window messages then process them:
		if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		// Otherwise, do animation/game stuff:
		else if (!m_timer.UpdateAndRender(update, render, processInput, processFrameStatistics))
		{
			break;
		}
	}

	return static_cast<int>(message.wParam);
}

void Application::OnKeyboardKeyDown(void* sender, const DXInputHandler::KeyboardEventArgs& eventArgs)
{
	auto pScene = m_graphics.GetScene();
	auto pCamera = m_graphics.GetCamera();

	if (eventArgs.Key == DIK_SPACE)
	{
		SceneBuilder::RenderItemInstanceData instanceData;

		XMFLOAT3 spawnPosition;
		XMStoreFloat3(&spawnPosition, pCamera->GetPosition() + pCamera->GetLocalForward() * 1.0f);

		auto scale = m_randomScales(m_randomEngine);
		instanceData =
		{
			{ spawnPosition.x, spawnPosition.z },
			{ 0.0f, m_randomAngles(m_randomEngine), 0.0f },
			{ scale, scale, scale },
		};

		pScene->AddTreeInstances(&m_graphics, { instanceData });
	}
	else if (eventArgs.Key == DIK_Z)
	{
		pScene->RemoveLastInstance(&m_graphics, "Tree", { "Trunk", "Leaves" });
	}
	else if(eventArgs.Key == DIK_C)
	{
		if (m_animationBuildMode)
		{
			m_animationManager.AddAnimation(std::make_unique<CameraAnimation>(
				*pCamera,
				m_animationManager.GetLastCameraFinalTime(),
				static_cast<float>(m_timer.GetTotalMilliseconds() - m_animationManager.GetLastCameraFinalTime()),
				m_animationManager.GetLastCameraPosition(),
				pCamera->GetPosition(),
				m_animationManager.GetLastCameraRotationQuaternion(),
				pCamera->GetRotationQuaternion()
				));
		}
	}

}

LRESULT Application::MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

std::mutex Application::s_mutex;
std::unique_ptr<Application> Application::s_instance;
Application::Application() :
	m_window(MainWindowProc),
	m_timer(c_millisecondsPerUpdate),
	m_input(m_window.GetHInstance(), m_window.GetWindowHandle(), m_window.GetClientWidth(), m_window.GetClientHeight()),
	m_graphics(m_window.GetWindowHandle(), m_window.GetClientWidth(), m_window.GetClientHeight(), m_window.IsFullscreen()),
	m_soundManager(m_window.GetWindowHandle()),
	m_randomDevice(),
	m_randomEngine(m_randomDevice()),
	m_randomAngles(0.0f, 2.0 * XM_PI),
	m_randomScales(0.5f, 1.0f),
	m_animationManager(m_graphics, L"Animations.json"),
	m_animationBuildMode(false),
	m_beginCameraAnimationFromLastSpot(false)
{
	m_soundManager.Create2DSoundFromWaveFile("TestSound", L"Sounds/Sound01.wav");

	using namespace std::placeholders;
	m_input.SubscribeToOnKeyDownEvents(DIK_SPACE, std::bind(&Application::OnKeyboardKeyDown, this, _1, _2));
	m_input.SubscribeToOnKeyDownEvents(DIK_Z, std::bind(&Application::OnKeyboardKeyDown, this, _1, _2));
	m_input.SubscribeToOnKeyDownEvents(DIK_C, std::bind(&Application::OnKeyboardKeyDown, this, _1, _2));
	m_input.SubscribeToOnKeyDownEvents(DIK_V, std::bind(&Application::OnKeyboardKeyDown, this, _1, _2));
	m_input.SubscribeToOnKeyDownEvents(DIK_B, std::bind(&Application::OnKeyboardKeyDown, this, _1, _2));

	if(m_animationBuildMode)
	{
		/*auto pCamera = m_graphics.GetCamera();
		//pCamera->SetPosition(m_animationManager.GetLastCameraPosition());
		pCamera->SetRotationQuaternion(m_animationManager.GetLastCameraRotationQuaternion());

		pCamera->SetPosition(220.0f - 512.0f, 27.0f, -(0.0f - 512.0f));
		pCamera->Update();
		pCamera->RotateWorldY(XM_PI);*/
	}
}
