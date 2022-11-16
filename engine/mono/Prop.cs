using System;
using System.Collections.Generic;
using System.Text;

namespace EngineMono {

	class Prop<TValue>
		where TValue : unmanaged 
	{
		private CppLinked m_linkedObj;
		private int m_index;

		public TValue value {
			get { return Ref.GetFieldValue<TValue>(m_linkedObj.classInfo, m_linkedObj.objectRef, m_index); }
			set { Ref.SetFieldValue(m_linkedObj.classInfo, m_linkedObj.objectRef, m_index, value); }
		}

		public void Link(CppLinked linkedObj, int propIndex) {
			m_linkedObj = linkedObj;
			m_index = propIndex;
		}
	}
}
