#include "IAsset.h"

//size_t IAsset_assetId_get(CppRef objRef) {
//		auto* obj = CppRefs::ThrowPointer<IAsset>(objRef); 
//		auto str = obj->assetId();
//		return (size_t)str.c_str();
//}
//
//DEF_PROP_SET_STR(IAsset, assetId);

DEF_PROP_GETSET_CSTR(IAsset, assetId);
DEF_PROP_GETSET(IAsset, int, assetIdHash);