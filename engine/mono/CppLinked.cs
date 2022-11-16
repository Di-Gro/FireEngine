using System;
using System.Collections.Generic;
using System.Text;

namespace EngineMono {
	class CppLinked {

		public ClassInfo classInfo { get; private set; }

		public UInt64 objectRef { get; private set; }

		public virtual void Link(UInt64 classInfoRef, UInt64 objRef) {
			classInfo = ClassInfo.GetClassInfo(classInfoRef);
			objectRef = objRef;
		}
	}
}
