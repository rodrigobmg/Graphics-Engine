﻿#include "stdafx.h"
#include "D3DBase.h"
#include "SettingsManager.h"

using namespace Common;
using namespace GraphicsEngine;
using namespace Microsoft::WRL;
using namespace std;

D3DBase::D3DBase(HWND outputWindow, uint32_t clientWidth, uint32_t clientHeight, bool fullscreen) :
	m_outputWindow(outputWindow),
	m_clientWidth(clientWidth),
	m_clientHeight(clientHeight),
	m_fullscreen(fullscreen),
	m_verticalSync(true)
{
	Initialize();
}

void D3DBase::OnResize(uint32_t clientWidth, uint32_t clientHeight)
{
	// TODO
}

void D3DBase::BeginScene() const
{
	// Reset the viewport:
	m_immediateContext->RSSetViewports(1, &m_viewport);

	// Bind the render target view and depth stencil buffer to the output render pipeline:
	m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

	// Clear the back buffer:
	m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), m_clearColor.data());

	// Clear the depth buffer:
	m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

}
void D3DBase::EndScene() const
{
	// Present as fast as possible:
	Common::ThrowIfFailed(m_swapChain->Present(m_verticalSync ? 1 : 0, 0));
	
	// Discard the contents of the render target and depth stencil:
	m_immediateContext->DiscardView(m_renderTargetView.Get());
	m_immediateContext->DiscardView(m_depthStencilView.Get());
}

ID3D11Device2* D3DBase::GetDevice() const
{
	return m_device.Get();
}
ID3D11DeviceContext2* D3DBase::GetDeviceContext() const
{
	return m_immediateContext.Get();
}
IDXGISwapChain2* D3DBase::GetSwapChain() const
{
	return m_swapChain.Get();
}
ID3D11DepthStencilView* D3DBase::GetDepthStencilView() const
{
	return m_depthStencilView.Get();
}
float D3DBase::GetAspectRatio() const
{
	return static_cast<float>(m_clientWidth) / m_clientHeight;
}
uint32_t D3DBase::GetClientWidth() const
{
	return m_clientWidth;
}
uint32_t D3DBase::GetClientHeight() const
{
	return m_clientHeight;
}

void D3DBase::SetViewport() const
{
	m_immediateContext->RSSetViewports(1, &m_viewport);
}

void D3DBase::SetClearColor(const DirectX::XMFLOAT3 clearColor)
{
	m_clearColor[0] = clearColor.x;
	m_clearColor[1] = clearColor.y;
	m_clearColor[2] = clearColor.z;
	m_clearColor[3] = 1.0f;
}

void D3DBase::SetDefaultRenderTargets() const
{
	m_immediateContext->RSSetViewports(1, &m_viewport);
	m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
}

void D3DBase::Initialize()
{
	InitializeDeviceResources();
	InitializeDepthStencilResources();
	InitializeViewport();
}
void D3DBase::InitializeDeviceResources()
{
	// Create a DirectX graphics interface factory:
	ComPtr<IDXGIFactory2> factory;
	Common::ThrowIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(factory.GetAddressOf())));

	// Create a settings manager object which will select the adapter:
	//auto settingsManager = SettingsManager::Build(L"Settings.conf");

	// Use the factory to create an adapter for the primary graphics interface (video card):
	ComPtr<IDXGIAdapter2> adapter;
	{
		ComPtr<IDXGIAdapter1> adapterTmp;
		Common::ThrowIfFailed(factory->EnumAdapters1(0, adapterTmp.GetAddressOf()));
		adapterTmp.As(&adapter);
	}

	// Create the Direct3D device and immediate context:
	{
		// Set the feature level to DirectX 11.
		array<D3D_FEATURE_LEVEL, 2> featureLevels =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};

		unsigned int creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> immediateContext;
		Common::ThrowIfFailed(
			D3D11CreateDevice(
				adapter.Get(),
				D3D_DRIVER_TYPE_UNKNOWN,
				nullptr,
				creationFlags,
				featureLevels.data(),
				static_cast<UINT>(featureLevels.size()),
				D3D11_SDK_VERSION,
				device.GetAddressOf(),
				nullptr,
				immediateContext.GetAddressOf()
			)
		);
		device.As(&m_device);
		immediateContext.As(&m_immediateContext);
	}

	// Create the swap chain:
	{
		// Initialize the swap chain description:
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = m_clientWidth;
		swapChainDesc.Height = m_clientHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = 0;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenSwapChainDesc;
		fullscreenSwapChainDesc.RefreshRate.Numerator = 0;
		fullscreenSwapChainDesc.RefreshRate.Denominator = 1;
		fullscreenSwapChainDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fullscreenSwapChainDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fullscreenSwapChainDesc.Windowed = !m_fullscreen;
		
		ComPtr<IDXGISwapChain1> swapChain;
		Common::ThrowIfFailed(
			factory->CreateSwapChainForHwnd(
				m_device.Get(),
				m_outputWindow,
				&swapChainDesc,
				&fullscreenSwapChainDesc,
				nullptr,
				swapChain.GetAddressOf()
			)
		);
		swapChain.As(&m_swapChain);
	}

	// Get the pointer to the back buffer.
	ComPtr<ID3D11Texture2D> backBuffer;
	Common::ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(backBuffer.GetAddressOf())));

	// Create the render target view with the back buffer pointer.
	Common::ThrowIfFailed(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf()));
}
void D3DBase::InitializeDepthStencilResources()
{
	// Create depth stencil buffer:
	{
		D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};
		depthStencilBufferDesc.Width = m_clientWidth;
		depthStencilBufferDesc.Height = m_clientHeight;
		depthStencilBufferDesc.MipLevels = 1;
		depthStencilBufferDesc.ArraySize = 1;
		depthStencilBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilBufferDesc.SampleDesc.Count = 1;
		depthStencilBufferDesc.SampleDesc.Quality = 0;
		depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilBufferDesc.CPUAccessFlags = 0;
		depthStencilBufferDesc.MiscFlags = 0;

		// Create the texture for the depth buffer using the filled out description:
		Common::ThrowIfFailed(m_device->CreateTexture2D(&depthStencilBufferDesc, nullptr, m_depthStencilBuffer.GetAddressOf()));
	}

	// Create depth stencil view:
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		Common::ThrowIfFailed(m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilViewDesc, m_depthStencilView.GetAddressOf()));
	}
}
void D3DBase::InitializeViewport()
{
	// Setup the viewport for rendering:
	m_viewport.Width = static_cast<float>(m_clientWidth);
	m_viewport.Height = static_cast<float>(m_clientHeight);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
}
