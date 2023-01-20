
using System;

namespace Engine {
    public class NullFieldException : Exception {

        public NullFieldException(CSComponent component, string msg = "") 
        : base($"Null Field in {component.GetType().FullName}: '{msg}'") {

        }

    }
}