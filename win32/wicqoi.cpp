// wicqoi.c - QOI Codec for Windows Imaging Component
//
// Copyright (C) 2022 Piotr Fusik
//
// MIT License:
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <wincodec.h>

#include "QOI.h"

static HINSTANCE g_hDll;
static LONG g_cRef = 0;

static void DllAddRef()
{
	InterlockedIncrement(&g_cRef);
}

static void DllRelease()
{
	InterlockedDecrement(&g_cRef);
}

// 44229228-8E08-49CF-B93E-F6FA0BD3A502
static const GUID GUID_QOIFormat =
	{ 0x44229228, 0x8e08, 0x49cf, { 0xb9, 0x3e, 0xf6, 0xfa, 0x0b, 0xd3, 0xa5, 0x02 } };

#define CLSID_WICQOIDecoder_str "{3786FCFE-A8F4-4AEB-852C-C76CC74D1123}"
static const GUID CLSID_WICQOIDecoder =
	{ 0x3786fcfe, 0xa8f4, 0x4aeb, { 0x85, 0x2c, 0xc7, 0x6c, 0xc7, 0x4d, 0x11, 0x23 } };

class CWICQOIFrameDecode : public IWICBitmapFrameDecode
{
	LONG m_cRef = 1;
	QOIDecoder * const m_qoi;

public:

	CWICQOIFrameDecode(QOIDecoder *qoi) : m_qoi(qoi)
	{
	}

	virtual ~CWICQOIFrameDecode()
	{
		QOIDecoder_Delete(m_qoi);
	}

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == nullptr)
			return E_POINTER;
		if (riid == __uuidof(IUnknown) || riid == __uuidof(IWICBitmapSource) || riid == __uuidof(IWICBitmapFrameDecode)) {
			*ppv = static_cast<IWICBitmapFrameDecode *>(this);
			AddRef();
			return S_OK;
		}
		*ppv = nullptr;
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}

	STDMETHODIMP_(ULONG) Release()
	{
		ULONG r = InterlockedDecrement(&m_cRef);
		if (r == 0)
			delete this;
		return r;
	}

	STDMETHODIMP GetSize(UINT *puiWidth, UINT *puiHeight)
	{
		if (puiWidth == nullptr || puiHeight == nullptr)
			return E_POINTER;
		*puiWidth = QOIDecoder_GetWidth(m_qoi);
		*puiHeight = QOIDecoder_GetHeight(m_qoi);
		return S_OK;
	}

	STDMETHODIMP GetPixelFormat(WICPixelFormatGUID *pPixelFormat)
	{
		if (pPixelFormat == nullptr)
			return E_POINTER;
		*pPixelFormat = GUID_WICPixelFormat32bppBGRA;
		return S_OK;
	}

	STDMETHODIMP GetResolution(double *pDpiX, double *pDpiY)
	{
		if (pDpiX == nullptr || pDpiY == nullptr)
			return E_POINTER;
		*pDpiX = 96;
		*pDpiY = 96;
		return S_OK;
	}

	STDMETHODIMP CopyPalette(IWICPalette *pIPalette)
	{
		return WINCODEC_ERR_PALETTEUNAVAILABLE;
	}

	STDMETHODIMP CopyPixels(const WICRect *prc, UINT cbStride, UINT cbBufferSize, BYTE *pbBuffer)
	{
		if (pbBuffer == nullptr)
			return E_POINTER;
		const int *pixels = QOIDecoder_GetPixels(m_qoi);
		int imageWidth = QOIDecoder_GetWidth(m_qoi);
		int imageHeight = QOIDecoder_GetHeight(m_qoi);
		int width;
		int height;
		if (prc == nullptr) {
			width = imageWidth;
			height = imageHeight;
		}
		else if (prc->X < 0 || prc->Y < 0 || prc->Width < 0 || prc->Height < 0
			|| prc->X + prc->Width > imageWidth || prc->Y + prc->Height > imageHeight)
			return E_INVALIDARG;
		else {
			pixels += prc->Y * imageWidth + prc->X;
			width = prc->Width;
			height = prc->Height;
		}
		if (cbBufferSize < (height - 1) * cbStride + width * sizeof(int))
			return WINCODEC_ERR_INSUFFICIENTBUFFER;
		if (width == imageWidth && cbStride == width * sizeof(int))
			memcpy(pbBuffer, pixels, height * width * sizeof(int)); // optimize
		else {
			for (int y = 0; y < height; y++)
				memcpy(pbBuffer + y * cbStride, pixels + y * imageWidth, width * sizeof(int));
		}
		return S_OK;
	}

	STDMETHODIMP GetMetadataQueryReader(IWICMetadataQueryReader **ppIMetadataQueryReader)
	{
		return WINCODEC_ERR_UNSUPPORTEDOPERATION;
	}

	STDMETHODIMP GetColorContexts(UINT cCount, IWICColorContext **ppIColorContexts, UINT *pcActualCount)
	{
		return WINCODEC_ERR_UNSUPPORTEDOPERATION;
	}

	STDMETHODIMP GetThumbnail(IWICBitmapSource **ppIThumbnail)
	{
		return WINCODEC_ERR_CODECNOTHUMBNAIL;
	}
};

class CWICQOIDecoder : IWICBitmapDecoder
{
	LONG m_cRef = 1;
	IWICBitmapFrameDecode *m_pIBitmapFrame = nullptr;

public:

	virtual ~CWICQOIDecoder()
	{
		if (m_pIBitmapFrame != nullptr)
			m_pIBitmapFrame->Release();
	}

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == nullptr)
			return E_POINTER;
		if (riid == __uuidof(IUnknown) || riid == __uuidof(IWICBitmapDecoder)) {
			*ppv = static_cast<IWICBitmapDecoder *>(this);
			AddRef();
			return S_OK;
		}
		*ppv = nullptr;
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}

	STDMETHODIMP_(ULONG) Release()
	{
		ULONG r = InterlockedDecrement(&m_cRef);
		if (r == 0)
			delete this;
		return r;
	}

	STDMETHODIMP QueryCapability(IStream *pIStream, DWORD *pdwCapability)
	{
		if (pdwCapability == nullptr)
			return E_POINTER;
		*pdwCapability = WICBitmapDecoderCapabilityCanDecodeSomeImages;
		return S_OK;
	}

	STDMETHODIMP Initialize(IStream *pIStream, WICDecodeOptions cacheOptions)
	{
		if (pIStream == nullptr)
			return E_INVALIDARG;
		if (m_pIBitmapFrame != nullptr) {
			m_pIBitmapFrame->Release();
			m_pIBitmapFrame = nullptr;
		}

		STATSTG statstg;
		HRESULT hr = pIStream->Stat(&statstg, STATFLAG_NONAME);
		if (FAILED(hr))
			return hr;
		if (statstg.cbSize.QuadPart > INT_MAX)
			return WINCODEC_ERR_UNKNOWNIMAGEFORMAT;
		ULONG contentLen = statstg.cbSize.u.LowPart;
		uint8_t *content = static_cast<uint8_t *>(malloc(contentLen));
		if (content == NULL)
			return E_OUTOFMEMORY;
		hr = pIStream->Read(content, contentLen, &contentLen);
		if (FAILED(hr))
			return hr;

		QOIDecoder *qoi = QOIDecoder_New();
		if (qoi == nullptr) {
			free(content);
			return E_OUTOFMEMORY;
		}
		if (!QOIDecoder_Decode(qoi, content, contentLen)) {
			QOIDecoder_Delete(qoi);
			free(content);
			return WINCODEC_ERR_BADIMAGE;
		}
		free(content);
		m_pIBitmapFrame = new CWICQOIFrameDecode(qoi);
		return S_OK;
	}

	STDMETHODIMP GetContainerFormat(GUID *pguidContainerFormat)
	{
		if (pguidContainerFormat == nullptr)
			return E_POINTER;
		*pguidContainerFormat = GUID_QOIFormat;
		return S_OK;
	}

	STDMETHODIMP GetDecoderInfo(IWICBitmapDecoderInfo **ppIDecoderInfo)
	{
		if (ppIDecoderInfo == nullptr)
			return E_POINTER;
		IWICImagingFactory *pIWICImagingFactory;
		HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID *) &pIWICImagingFactory);
		if (SUCCEEDED(hr)) {
			IWICComponentInfo *pIWICComponentInfo;
			hr = pIWICImagingFactory->CreateComponentInfo(CLSID_WICQOIDecoder, &pIWICComponentInfo);
			if (SUCCEEDED(hr)) {
				hr = pIWICComponentInfo->QueryInterface(IID_IWICBitmapDecoderInfo, (void **) ppIDecoderInfo);
				pIWICComponentInfo->Release();
			}
			pIWICImagingFactory->Release();
		}
		return hr;
	}

	STDMETHODIMP GetFrameCount(UINT *pCount)
	{
		if (pCount == nullptr)
			return E_POINTER;
		*pCount = 1;
		return S_OK;
	}

	STDMETHODIMP GetFrame(UINT index, IWICBitmapFrameDecode **ppIBitmapFrame)
	{
		if (ppIBitmapFrame == nullptr)
			return E_POINTER;
		if (index != 0 || m_pIBitmapFrame == nullptr)
			return WINCODEC_ERR_FRAMEMISSING;
		m_pIBitmapFrame->AddRef();
		*ppIBitmapFrame = m_pIBitmapFrame;
		return S_OK;
	}

	STDMETHODIMP GetPreview(IWICBitmapSource **ppIPreview)
	{
		return WINCODEC_ERR_UNSUPPORTEDOPERATION;
	}

	STDMETHODIMP GetThumbnail(IWICBitmapSource **ppIThumbnail)
	{
		return WINCODEC_ERR_CODECNOTHUMBNAIL;
	}

	STDMETHODIMP GetColorContexts(UINT cCount, IWICColorContext **ppIColorContexts, UINT *pcActualCount)
	{
		return WINCODEC_ERR_UNSUPPORTEDOPERATION;
	}

	STDMETHODIMP GetMetadataQueryReader(IWICMetadataQueryReader **ppIMetadataQueryReader)
	{
		return WINCODEC_ERR_UNSUPPORTEDOPERATION;
	}

	STDMETHODIMP CopyPalette(IWICPalette *pIPalette)
	{
		return WINCODEC_ERR_PALETTEUNAVAILABLE;
	}
};

class CWICQOIClassFactory : IClassFactory
{
public:

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == nullptr)
			return E_POINTER;
		if (riid == __uuidof(IUnknown) || riid == __uuidof(IClassFactory)) {
			*ppv = static_cast<IClassFactory *>(this);
			DllAddRef();
			return S_OK;
		}
		*ppv = nullptr;
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		DllAddRef();
		return 2;
	}

	STDMETHODIMP_(ULONG) Release()
	{
		DllRelease();
		return 1;
	}

	STDMETHODIMP CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void **ppv)
	{
		*ppv = nullptr;
		if (punkOuter != nullptr)
			return CLASS_E_NOAGGREGATION;
		CWICQOIDecoder *punk = new CWICQOIDecoder;
		if (punk == nullptr)
			return E_OUTOFMEMORY;
		HRESULT hr = punk->QueryInterface(riid, ppv);
		punk->Release();
		return hr;
	}

	STDMETHODIMP LockServer(BOOL fLock)
	{
		if (fLock)
			DllAddRef();
		else
			DllRelease();
		return S_OK;
	}
};

STDAPI __declspec(dllexport) DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	if (ppv == nullptr)
		return E_INVALIDARG;
	if (rclsid == CLSID_WICQOIDecoder) {
		static CWICQOIClassFactory g_ClassFactory;
		return g_ClassFactory.QueryInterface(riid, ppv);
	}
	*ppv = nullptr;
	return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		g_hDll = hInstance;
	return TRUE;
}

static bool MyRegCreateKey(HKEY hk1, LPCSTR subkey, PHKEY hk2)
{
	return RegCreateKeyEx(hk1, subkey, 0, nullptr, 0, KEY_WRITE, nullptr, hk2, nullptr) == ERROR_SUCCESS;
}

static bool MyRegSetValueString(HKEY hk1, LPCSTR name, LPCSTR data)
{
	return RegSetValueEx(hk1, name, 0, REG_SZ, reinterpret_cast<const BYTE *>(data), strlen(data) + 1) == ERROR_SUCCESS;
}

static bool MyRegSetValueDword(HKEY hk1, LPCSTR name, DWORD type, DWORD data)
{
	return RegSetValueEx(hk1, name, 0, type, reinterpret_cast<const BYTE *>(&data), sizeof(data)) == ERROR_SUCCESS;
}

STDAPI __declspec(dllexport) DllRegisterServer()
{
	char szModulePath[MAX_PATH];
	if (GetModuleFileName(g_hDll, szModulePath, MAX_PATH) == 0)
		return E_FAIL;
	HKEY hk1;
	HKEY hk2;
	if (!MyRegCreateKey(HKEY_CLASSES_ROOT, "CLSID\\" CLSID_WICQOIDecoder_str, &hk1))
		return E_FAIL;
	if (!MyRegCreateKey(hk1, "InProcServer32", &hk2)) {
		RegCloseKey(hk1);
		return E_FAIL;
	}
	if (!MyRegSetValueString(hk2, nullptr, szModulePath)
	 || !MyRegSetValueString(hk2, "ThreadingModel", "Both")) {
		RegCloseKey(hk2);
		RegCloseKey(hk1);
		return E_FAIL;
	}
	RegCloseKey(hk2);
	if (!MyRegCreateKey(hk1, "Patterns\\0", &hk2)) {
		RegCloseKey(hk1);
		return E_FAIL;
	}
	if (!MyRegSetValueDword(hk2, "Position", REG_DWORD, 0)
	 || !MyRegSetValueDword(hk2, "Length", REG_DWORD, 4)
	 || !MyRegSetValueDword(hk2, "Pattern", REG_BINARY, 'q' | 'o' << 8 | 'i' << 16 | 'f' << 24)
	 || !MyRegSetValueDword(hk2, "Mask", REG_BINARY, 0xffffffff)) {
		RegCloseKey(hk2);
		RegCloseKey(hk1);
		return E_FAIL;
	}
	RegCloseKey(hk2);
	RegCloseKey(hk1);

	if (!MyRegCreateKey(HKEY_CLASSES_ROOT, "CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance\\" CLSID_WICQOIDecoder_str, &hk1))
		return E_FAIL;
	if (!MyRegSetValueString(hk1, "CLSID", CLSID_WICQOIDecoder_str)
	 || !MyRegSetValueString(hk1, "FriendlyName", "Quite OK Image Decoder")) {
		RegCloseKey(hk1);
		return E_FAIL;
	}
	RegCloseKey(hk1);

	if (!MyRegCreateKey(HKEY_CLASSES_ROOT, ".qoi", &hk1))
		return E_FAIL;
	if (!MyRegCreateKey(hk1, "ShellEx\\{e357fccd-a995-4576-b01f-234630154e96}", &hk2)) {
		RegCloseKey(hk1);
		return E_FAIL;
	}
	if (!MyRegSetValueString(hk2, nullptr, "{C7657C4A-9F68-40fa-A4DF-96BC08EB3551}")
	 || !MyRegSetValueString(hk1, "PerceivedType", "image")) {
		RegCloseKey(hk2);
		RegCloseKey(hk1);
		return E_FAIL;
	}
	RegCloseKey(hk2);
	RegCloseKey(hk1);

	return S_OK;
}

STDAPI __declspec(dllexport) DllUnregisterServer()
{
	HKEY hk1;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, ".qoi\\ShellEx", 0, DELETE, &hk1) == ERROR_SUCCESS) {
		RegDeleteKey(hk1, "{e357fccd-a995-4576-b01f-234630154e96}");
		RegCloseKey(hk1);
	}
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, "CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance", 0, DELETE, &hk1) == ERROR_SUCCESS) {
		RegDeleteKey(hk1, CLSID_WICQOIDecoder_str);
		RegCloseKey(hk1);
	}
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, "CLSID", 0, DELETE, &hk1) == ERROR_SUCCESS) {
		RegDeleteKey(hk1, CLSID_WICQOIDecoder_str "\\Patterns\\0");
		RegDeleteKey(hk1, CLSID_WICQOIDecoder_str "\\Patterns");
		RegDeleteKey(hk1, CLSID_WICQOIDecoder_str "\\InProcServer32");
		RegDeleteKey(hk1, CLSID_WICQOIDecoder_str);
		RegCloseKey(hk1);
	}
	return S_OK;
}
