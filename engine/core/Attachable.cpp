#include "Attachable.h"

#include "Player.h"


DEF_PURE_COMPONENT(Attachable);

void Attachable::OnInit() {
	m_mesh = AddComponent<MeshComponent>();
	m_bound = AddComponent<MeshComponent>();
	
}

void Attachable::OnStart() {

	float boxSize = boundRadius * 2 * 0.5;
	auto form = Forms4::Box({ boxSize, boxSize, boxSize }, { 0.8, 0.2, 0.4, 1.0 });
	m_mesh->AddShape(&form.verteces, &form.indexes);

	form = Forms4::SphereLined(boundRadius, 8, 8, { 0, 0.6, 0, 1 });
	m_bound->AddShape(&form.verteces, &form.indexes);
	m_bound->mesh()->topology = form.topology;

	m_mesh->visible = showCenter;
	m_bound->visible = false;
}

void Attachable::OnUpdate() {

	if (player != nullptr && player->IsDestroyed())
		player = nullptr;

	if (player == nullptr)
		return;

	if (!m_canAttach) {
		if ((boundRadius / player->radius) <= 0.8f) {
			m_canAttach = true;
			m_bound->visible = true && showBound;
		}
	}

	if (!m_canAttach)
		return;

	auto playerPos = player->transform->worldPosition();
	auto dist = (transform->worldPosition() - playerPos).Length();

	if (dist <= boundRadius + player->radius) {

		if(HasParent() && attachParent)
			player->Attach(GetParent());
		else 
			player->Attach(gameObject());
				
		player = nullptr;
	}
}