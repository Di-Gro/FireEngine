using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Engine;

namespace FireBinEditor {

    [GUID("bc133055-6560-456f-9241-9ffea417633a")]
    public class TestClass_Scalars {
        public bool BoolField = true; /// Has in ImGui
        public byte ByteField = 1; 
        public char CharField = 'A'; 
        public decimal DecimalField = 2; /// !CanSerialize()
        public double DoubleField = 3.3;
        public short ShortField = 4; 
        public int IntField = 5; /// Has in ImGui
        public long LongField = 6; 
        public sbyte SByteField = 7;
        public float FloatField = 8.8f; /// Has in ImGui
        public ushort UShortField = 9; 
        public uint UIntField = 10; 
        public ulong ULongField = 11; 

        public TestClass_Scalars() { }
    }

    [GUID("6edc16e2-72b2-4105-b59f-c2f6bf19da06")]
    public class TestClass_Strings {
        public string stringField = "abc";

        public TestClass_Strings() { }
    }

    [GUID("49c0a90b-720e-4d84-817a-ce61006a4bbf")]
    public class TestClass_Lists {
        public List<int> intList = new List<int>() { 1, 2, 3 };
        public char[] charsList = new char[] { 'a', 'b', 'c' };

        public TestClass_Lists() { }
    }

    [GUID("7f875e0c-e212-4f11-9ff4-041ebe932699")]
    public class TestClass {
        //public float floatValue = 1.1f;
        //public uint uintValue = 2;

        //public TestClass_Scalars scalars = new TestClass_Scalars();
        public TestClass_Scalars nullField = null;
        public TestClass_Scalars? nullableField = null;

        public TestClass() { }
    }
}
