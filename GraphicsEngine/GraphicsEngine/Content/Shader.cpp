﻿#include "stdafx.h"
#include "Shader.h"

#include <D3Dcompiler.h>

using namespace GraphicsEngine;
using namespace Microsoft::WRL;
using namespace std;

Shader::Shader(const wstring& filename)
{
	LoadBinary(filename);
}

void Shader::LoadBinary(const wstring& filename)
{
	ifstream filestream(filename, ios::binary);

	filestream.seekg(0, ios_base::end);
	auto size = filestream.tellg();
	filestream.seekg(0, ios_base::beg);

	DX::ThrowIfFailed(
		D3DCreateBlob(size, m_byteCode.GetAddressOf())
		);

	filestream.read(static_cast<char*>(m_byteCode->GetBufferPointer()), size);
	filestream.close();
}