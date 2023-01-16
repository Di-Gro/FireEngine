
using System;

namespace Engine {
    public class NullFieldException : Exception {

        public NullFieldException(CSComponent component) : base($"{component.GetType().FullName}: Null Field") {

        }

    }
}