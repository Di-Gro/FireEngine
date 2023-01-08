#pragma once

/// ->

template<IsCppAddableComponent TComponent, typename>
TComponent* ActorBase::AddComponent(bool isRuntimeOnly) {
	if (!IsDestroyed())
		return actor()->AddComponent<TComponent>(isRuntimeOnly);
	return nullptr;
}

/// <-

template<typename TComponent, typename>
TComponent* ActorBase::GetComponent() {
	if (!IsDestroyed())
		return actor()->GetComponent<TComponent>();
	return nullptr;
}

template<typename TComponent, typename>
TComponent* ActorBase::GetComponentInChild() {
	if (!IsDestroyed())
		return actor()->GetComponentInChild<TComponent>();
	return nullptr;
}