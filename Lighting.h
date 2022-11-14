#pragma once

class DirectionLight;

class Game;

class Lighting {

	/// ������ ����� �������� ������������� �� Game.Run()
	friend Game;

private:
	Game* m_game;

	DirectionLight* f_directionLight = nullptr;

public:
	DirectionLight* directionLight() { return f_directionLight; }

	void Init(Game*);

	void OnRender();

};

