#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <wincodec.h>

template<typename T>
using comptr = Microsoft::WRL::ComPtr<T>;