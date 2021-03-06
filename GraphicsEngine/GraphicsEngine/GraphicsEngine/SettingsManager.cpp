﻿#include "stdafx.h"
#include "SettingsManager.h"
#include "Common/Helpers.h"

#include <fstream>
#include <rapidxml/rapidxml_print.hpp>
#include <rapidxml/rapidxml_utils.hpp>

using namespace Common;
using namespace GraphicsEngine;
using namespace std;
using namespace rapidxml;
using namespace Microsoft::WRL;

constexpr auto TAG_SETTINGS = L"Settings";
constexpr auto TAG_VIDEO_CARDS = L"VideoCards";
constexpr auto TAG_VIDEO_CARD = L"VideoCard";
constexpr auto TAG_INDEX = L"Index";
constexpr auto TAG_DESCRIPTION = L"Description";
constexpr auto TAG_DEDICATED_VIDEO_MEMORY = L"DedicatedVideoMemory";
constexpr auto TAG_DEFAULT_VIDEO_CARD_INDEX = L"DefaultVideoCardIndex";

SettingsManager::SettingsManager() :
	m_adapterIndex(0),
	m_adapterDescription(),
	m_adapterDedicatedVideoMemory(0)
{
}

SettingsManager SettingsManager::Build(const wstring& filename)
{
	SettingsManager settings;

	// If file exists, read configuration from file:
	if (Helpers::FileExists(filename))
		settings.ReadFile(filename);

	// If file doesn't exist, then use DirectX API and create a settings file: 
	else
		settings.CreateFileW(filename);

	return settings;
}

uint32_t SettingsManager::GetAdapterIndex() const
{
	return m_adapterIndex;
}

const wstring& SettingsManager::GetAdapterDescription() const
{
	return m_adapterDescription;
}

SIZE_T SettingsManager::GetAdapterDedicatedVideoMemory() const
{
	return m_adapterDedicatedVideoMemory;
}

void SettingsManager::ReadFile(const wstring& filename)
{
	// Read settings file:
	file<wchar_t> settingsFile(Helpers::WStringToString(filename).data());
	xml_document<wchar_t> document;
	document.parse<0>(settingsFile.data());

	// Get root node:
	auto settingsNode = document.first_node(TAG_SETTINGS);

	// Get video cards node:
	auto videoCardsNode = settingsNode->first_node(TAG_VIDEO_CARDS);

	// Get default video card index:
	auto defaultVideoCardIndexNode = videoCardsNode->first_node(TAG_DEFAULT_VIDEO_CARD_INDEX);
	auto defaultVideoCardIndex = stoi(defaultVideoCardIndexNode->value());

	// Get default video card:
	auto defaultVideoCard = videoCardsNode->first_node(TAG_VIDEO_CARD);
	for (auto i = 0; i < defaultVideoCardIndex; i++)
		defaultVideoCard = defaultVideoCard->next_sibling(TAG_VIDEO_CARD);

	// Get default adapter values:
	m_adapterIndex = stoi(defaultVideoCard->first_node(TAG_INDEX)->value());
	m_adapterDescription = defaultVideoCard->first_node(TAG_DESCRIPTION)->value();
	m_adapterDedicatedVideoMemory = stoi(defaultVideoCard->first_node(TAG_DEDICATED_VIDEO_MEMORY)->value());
}

void SettingsManager::CreateFile(const wstring& filename)
{
	// Create document:
	xml_document<wchar_t> document;

	// Create root node:
	xml_node<wchar_t>* settingsNode = document.allocate_node(node_type::node_element, TAG_SETTINGS);
	document.append_node(settingsNode);

	// Output adapters info:
	AddAdaptersInfo(&document, settingsNode);

	// Create settings file:
	wofstream configFile(filename, ios::out);

	// Output document to file:
	rapidxml::print(ostream_iterator<wchar_t, wchar_t, char_traits<wchar_t>>(configFile), document);
}

void SettingsManager::AddAdaptersInfo(rapidxml::xml_document<wchar_t>* document, rapidxml::xml_node<wchar_t>* parent)
{
	// Create video cards node and append it to the settings node:
	xml_node<wchar_t>* videoCardsNode = document->allocate_node(node_type::node_element, TAG_VIDEO_CARDS);
	parent->append_node(videoCardsNode);

	// Creates a DXGI factory that can be used to generate other DXGI objects:
	ComPtr<IDXGIFactory2> factory;
	ThrowIfFailed(CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(factory.GetAddressOf())));

	// Enumerate through all the adapters, output info to file and select best adapter based on the Dedicated Video Memory:
	ComPtr<IDXGIAdapter1> adapter;
	for (auto adapterIndex = 0; factory->EnumAdapters1(adapterIndex, adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; adapterIndex++)
	{
		ComPtr<IDXGIAdapter2> adapter2;
		adapter.As(&adapter2);

		DXGI_ADAPTER_DESC2 adapterDesc;
		adapter2->GetDesc2(&adapterDesc);

		// Select best adapter based on the Dedicated Video Memory:
		if(adapterDesc.DedicatedVideoMemory > m_adapterDedicatedVideoMemory)
		{
			m_adapterIndex = adapterIndex;
			m_adapterDescription = adapterDesc.Description;
			m_adapterDedicatedVideoMemory = adapterDesc.DedicatedVideoMemory;
		}

		// Write adapter info to file:
		AddAdapterInfo(document, videoCardsNode, adapterIndex, adapterDesc);
	}

	// Create default video card index node and add it to the video cards node:
	xml_node<wchar_t>* defaultVideoCardIndexNode = document->allocate_node(node_type::node_element, TAG_DEFAULT_VIDEO_CARD_INDEX, AllocateValue(document, m_adapterIndex));
	videoCardsNode->append_node(defaultVideoCardIndexNode);
}

void SettingsManager::AddAdapterInfo(xml_document<wchar_t>* document, rapidxml::xml_node<wchar_t>* parent, uint32_t adapterIndex, const DXGI_ADAPTER_DESC2& adapterDesc) const
{
	// Create nodes to describe a video card:
	xml_node<wchar_t>* videoCardNode = document->allocate_node(node_type::node_element, TAG_VIDEO_CARD);
	xml_node<wchar_t>* indexNode = document->allocate_node(node_type::node_element, TAG_INDEX, AllocateValue(document, adapterIndex));
	xml_node<wchar_t>* descriptionNode = document->allocate_node(node_type::node_element, TAG_DESCRIPTION, document->allocate_string(adapterDesc.Description));
	xml_node<wchar_t>* dedicatedVideoMemoryNode = document->allocate_node(node_type::node_element, TAG_DEDICATED_VIDEO_MEMORY, AllocateValue(document, adapterDesc.DedicatedVideoMemory));
	
	// Add video card node to parent node:
	parent->append_node(videoCardNode);

	// Add all others as children of the video card node:
	videoCardNode->append_node(indexNode);
	videoCardNode->append_node(descriptionNode);
	videoCardNode->append_node(dedicatedVideoMemoryNode);
}
