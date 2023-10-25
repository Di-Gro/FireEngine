using System;
using System.Collections.Generic;
using System.Text;

namespace Engine {

	interface IProp {
		void Link(CppLinked linkedObj);
	}

	class Prop<TValue> : IProp
		where TValue : unmanaged 
	{
		private CppLinked m_linkedObj;
		private int m_index;

		public bool IsInited => m_linkedObj != null;


        public Prop(int propIndex) {
			m_index = propIndex;
		}

		public TValue value {
			get { return Ref.GetFieldValue<TValue>(m_linkedObj.classInfo, m_linkedObj.cppRef, m_index); }
			set { Ref.SetFieldValue(m_linkedObj.classInfo, m_linkedObj.cppRef, m_index, value); }
		}

		public void Link(CppLinked linkedObj) {
			m_linkedObj = linkedObj;
		}
	}

	// public class CppProps {
	// 	public ClassInfo classInfo { get; private set; }
	// 	public CppRef cppRef { get; private set; }

	// 	public void Link(CppRef classInfoRef, CppRef objRef) {
	// 		cppRef = objRef;
	// 		classInfo = ClassInfo.GetClassInfo(classInfoRef);
	// 	}

	// 	public TValue Get<TValue>(int index) where TValue : unmanaged {
	// 		return Ref.GetFieldValue<TValue>(classInfo, cppRef, index);
	// 	}

	// 	public void Set<TValue>(int index, TValue value) where TValue : unmanaged {
	// 		Ref.SetFieldValue(classInfo, cppRef, index, value);
	// 	}
	// }
}
