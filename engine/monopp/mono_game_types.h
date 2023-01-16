#pragma once

#include <iostream>


class CppRef {
public:
	static const CppRef Void;

	size_t value = 0;

	//CppRef() {}
	//CppRef(size_t v) : value(v) {}

	operator size_t() const { return value; }

	CppRef& operator=(size_t v) {
		value = v; 
		return *this;
	}

	static CppRef Create(size_t v) {
		CppRef ref;
		ref.value = v;
		return ref;
	}

	friend std::ostream& operator<< (std::ostream& out, const CppRef& ref);
};

class CsRef {
public:
	static const CsRef Void;

	size_t value = 0;

	//CsRef() {}
	//CsRef(size_t v) : value(v) {}

	operator size_t() const { return value; }

	CsRef& operator=(size_t v) {
		value = v;
		return *this;
	}

	static CsRef Create(size_t v) {
		CsRef ref;
		ref.value = v;
		return ref;
	}

	friend std::ostream& operator<< (std::ostream& out, const CsRef& ref);
};

CppRef RefCpp(size_t v);
CsRef RefCs(size_t v);

class CppObjectInfo {
public:
	CppRef cppRef;
	CppRef classRef;
};

class GameObjectInfo {
public:
	CppRef objectRef;
	CppRef classRef;
	//CsRef transformRef;
};

class GameUpdateData {
public:
	float deltaTime;
};

class ComponentCallbacks {
public:
	void(*onInit)();
	void(*onStart)();
	void(*onUpdate)();
	void(*onFixedUpdate)();
	void(*onDestroy)();
};

class GameCallbacks {
public:
	void (*setSceneRef)(CppRef value);
	void (*setMeshAssetRef)(CppRef value);
	void (*setAssetStoreRef)(CppRef value);
	void (*setUpdateData)(GameUpdateData value);
	void (*onInputUpdate)();

	int (*saveScene)(CppRef cppSceneRef, size_t pathPtr);
	bool (*loadScene)(CppRef cppSceneRef, int assetGuidHash);

	//bool(*saveScene)(CppRef cppSceneRef, size_t assetIdPtr, size_t pathPtr);
	//bool(*loadScene)(CppRef cppSceneRef, size_t assetIdPtr);

	bool (*runOrCrush)(CsRef componentRef, void(*method)());

	bool (*isAssignable)(CsRef objRef, int typeIdHash);

	void (*removeCsRef)(CsRef value);

	void (*loadAssetStore)();
	bool (*hasAssetInStore)(int assetIdHash);

	int (*getStringHash)(size_t stringPtr);

	bool (*loadAsset)(int assetIdHash, CppRef cppRef);
	void (*reloadAsset)(int assetIdHash);
	void (*saveAsset)(int assetIdHash);

	void (*pushClipboard)(CsRef value);
	CppRef (*peekClipboard)();
	bool (*clipboardIsAssignable)(int scriptIdHash);
	bool (*clipboardIsSameType)(int scriptIdHash);
	void (*clipboardSetActor)(CsRef value);

	bool (*createAsset)(size_t pathPtr);
	bool (*renameAsset)(int assetGuidHash, size_t newPathPtr);
	void (*removeAsset)(int assetGuidHash);

	int (*createPrefab)(CsRef actorRef, size_t pathPtr);
	bool (*loadPrefab)(int assetGuidHash, CsRef actorRef);
	bool (*updatePrefab)(CsRef actorRef, int assetGuidHash);

	void (*setPrefabId)(CsRef actorRef, int prefabGuidHash);

	int (*createSceneAsset)(size_t pathPtr);
	bool (*renameSceneAsset)(int assetIdHash, size_t pathPtr);

	void (*requestAssetGuid)(int assetIdHash);
};

