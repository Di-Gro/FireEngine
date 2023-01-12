using EngineDll;

namespace Engine {
    public class Rigidbody : CppComponent
    {
        public override CppObjectInfo CppConstructor()
        {
            return Dll.Rigidbody.Create(csRef);
        }
    }
}