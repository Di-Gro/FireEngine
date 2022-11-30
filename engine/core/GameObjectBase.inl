#pragma once

/// ->

//template<typename TComponent, typename>
//TComponent* GameObjectBase::AddComponent() {
//	if (!IsDestroyed())
//		return gameObject()->AddComponent<TComponent>();
//	return nullptr;
//}

template<HasCsMetaData TComponent, typename>
TComponent* GameObjectBase::AddComponent() {
	if (!IsDestroyed())
		return gameObject()->AddComponent<TComponent>();
	return nullptr;
}
//
//template<IsCSpp TComponent, typename>
//TComponent* GameObjectBase::AddComponent() {
//	if (!IsDestroyed())
//		return gameObject()->AddComponent<TComponent>();
//	return nullptr;
//}

/// <-

template<typename TComponent, typename>
TComponent* GameObjectBase::GetComponent() {
	if (!IsDestroyed())
		return gameObject()->GetComponent<TComponent>();
	return nullptr;
}

template<typename TComponent, typename>
TComponent* GameObjectBase::GetComponentInChild() {
	if (!IsDestroyed())
		return gameObject()->GetComponentInChild<TComponent>();
	return nullptr;
}