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
}
