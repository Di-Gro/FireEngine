#include "TransformTest.h"

#include <string>

#include "Math.h"

inline Vector3 deg(Vector3 vec) {
	return Vector3(deg(vec.x), deg(vec.y), deg(vec.z));
}

//std::string ToString(Vector3 vec) {
//	return "("
//		+ std::to_string(vec.x) + ", "
//		+ std::to_string(vec.y) + ", "
//		+ std::to_string(vec.z) + ")";
//}

std::string ToString(Transform& transform) {
	std::string str;

	str += "local position: " + ToString(transform.localPosition()) + "\n";
	str += "local rotation: " + ToString(deg(transform.localRotation())) + "\n";
	//str += "local scale:    " + ToString(transform.scale) + "\n";

	str += "world position: " + ToString(transform.worldPosition()) + "\n";
	str += "world rotation: " + ToString(deg(transform.worldRotation())) + "\n";
	//str += "world scale:    " + ToString(transform.worldScale()) + "\n";

	return str;
}

void TransformTest::OnUpdate() {
	//m_TestLocalPosition();
	m_TestLocalPosition2();
	//this->Destroy();
	game()->Exit(0);
}

void TransformTest::m_TestLocalPosition() {
	/*std::cout << "TransformTest 1 " << std::endl;
	std::cout << std::endl;
	std::cout << "LOCAL POSITION" << std::endl;
	std::cout << std::endl;

	auto box1 = CreateGameObject("box1");
	auto box2 = CreateGameObject("box2");
	auto box3 = CreateGameObject("box3");
	box2->transform.position.x = 200;
	box3->transform.position.x = 100;

	std::cout << "box1.worldPosition" << ToString(box1->transform.worldPosition()) << std::endl;
	std::cout << "box2.worldPosition" << ToString(box2->transform.worldPosition()) << std::endl;
	std::cout << "box3.worldPosition" << ToString(box3->transform.worldPosition()) << std::endl;
	std::cout << std::endl;

	std::cout << "box3.SetParent( box1 )" << std::endl;
	box3->SetParent(box1);

	game()->PrintSceneTree();
	std::cout << std::endl;

	std::cout << "box3.worldPosition" << ToString(box3->transform.worldPosition()) << std::endl;
	std::cout << "box3.localPosition" << ToString(box3->transform.position) << std::endl;
	std::cout << std::endl;

	std::cout << "box3.SetParent( box2 )" << std::endl;
	box3->SetParent(box2);

	game()->PrintSceneTree();
	std::cout << std::endl;

	std::cout << "box3.worldPosition" << ToString(box3->transform.worldPosition()) << std::endl;
	std::cout << "box3.localPosition" << ToString(box3->transform.position) << std::endl;
	std::cout << std::endl;

	std::cout << "box3.SetParent( null )" << std::endl;
	box3->SetParent(nullptr);

	game()->PrintSceneTree();
	std::cout << std::endl;

	std::cout << "box3.worldPosition" << ToString(box3->transform.worldPosition()) << std::endl;
	std::cout << "box3.localPosition" << ToString(box3->transform.position) << std::endl;
	std::cout << std::endl;


	std::cout << "SCALE" << std::endl;
	std::cout << std::endl;
	std::cout << "box2.scale = {1, 2, 0.5}" << std::endl;
	std::cout << "box3.SetParent( box2 )" << std::endl;
	box2->transform.scale = { 1, 2, 0.5f };

	std::cout << "box2\n" << ToString(box2->transform) << std::endl;

	box3->SetParent(box2);

	game()->PrintSceneTree();
	std::cout << std::endl;

	std::cout << "box2.scale" << ToString(box2->transform.scale) << std::endl;
	std::cout << "box3.scale" << ToString(box3->transform.scale) << std::endl;
	std::cout << std::endl;

	std::cout << "box3.SetParent( null )" << std::endl;
	box3->SetParent(nullptr);

	game()->PrintSceneTree();
	std::cout << std::endl;

	std::cout << "box3.scale" << ToString(box3->transform.scale) << std::endl;*/
}

void TransformTest::m_TestLocalPosition2() {

	//auto rx = Matrix::CreateRotationX(rad(48));
	//auto ry = Matrix::CreateRotationY(rad(28));
	//auto rz = Matrix::CreateRotationZ(rad(17));
	//auto r = rx * ry * rz;

	//auto m = r; 
	//m = m.Transpose();
	//std::cout << ToString(deg(m.ToEuler())) << std::endl;

	//return;

	std::cout << "TransformTest 2" << std::endl;
	std::cout << std::endl;

	auto box1 = CreateGameObject("box1");
	box1->transform.localPosition({ 200, 45, 50 });
	box1->transform.localRotation({ rad(0), rad(80), rad(0) });
	//box1->transform.scale = { 1, 2, 0.5f };

	auto box2 = CreateGameObject("box2");
	box2->transform.localPosition({ 100, 0, 0 });
	box2->transform.localRotation({ rad(0), rad(90), rad(0) });

	std::cout << "box1\n" << ToString(box1->transform) << std::endl;
	std::cout << "box2\n" << ToString(box2->transform) << std::endl;

	std::cout << "box2.SetParent( box1 )" << std::endl << std::endl;
	box2->SetParent(box1);

	std::cout << "box2\n" << ToString(box2->transform) << std::endl;

	std::cout << "box2.SetParent( null )" << std::endl << std::endl;
	box2->SetParent(nullptr);

	std::cout << "box2\n" << ToString(box2->transform) << std::endl;
}
