using EngineDll;

namespace Engine {

    class BoxCollider : CppComponent {
        public override CppObjectInfo CppConstructor() {
            return Dll.BoxCollider.Create(csRef);
        }
    }

    class SphereCollider : CppComponent {
        public override CppObjectInfo CppConstructor() {
            return Dll.SphereCollider.Create(csRef);
        }
    };

}