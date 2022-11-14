#include "Lighting.h"


void Lighting::Init(Game* game) {
	m_game = game;

	/// TODO: Убрать инициализацию света из Game.Run(). 
	/// Сейчас свет крепится к FloatCamera, для упрощения 
	/// настройки направления. 
}

void Lighting::OnRender() {
	
}