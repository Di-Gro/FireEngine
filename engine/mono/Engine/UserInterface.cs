using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using UI;
using sn = System.Numerics;
using EngineDll;

namespace Engine
{
    enum myEnum { HELLO, WORLD };
    class UserInterface
    {
        public static UserInterface Instance { get; private set; }

        [StructLayout(LayoutKind.Sequential)]
        public struct Callbacks
        {
            public delegate void Callback(CsRef csRef, float width);

            public Callback onDrawComponent;
        }

        private Callbacks _callbacks;

        private float rectWidth = 0;

        private int tmp = 155;
        private float tmpF = 155.0f;
        private Vector2 tmpVec = new Vector2(5.0f, 5.0f);
        private Vector3 tmpVec3 = new Vector3(5.0f, 5.0f, 5.0f);
        private Quaternion tmpQuaternion = new Quaternion(5.0f, 5.0f, 5.0f, 1.0f);
        private bool boolCheckbox = true;
        private string testString = "my test string";

        private myEnum mm = myEnum.WORLD;

        private bool hasBorder = false;

        private sn.Vector2 m_lineSpacing = new sn.Vector2( 0, 3 );

        private CppRef _cppRef;

        void Space() { ImGui.Dummy(m_lineSpacing); }

        public static void cpp_Init(CppRef cppRef)
        {
            Instance = new UserInterface();
            Instance._cppRef = cppRef;
            Instance._callbacks = new Callbacks();
            Instance._callbacks.onDrawComponent = new Callbacks.Callback(Instance.OnDrawComponent);
            dll_SetCallbacks(cppRef, Instance._callbacks.onDrawComponent);
        }

        //public static void OnDrawComponent(ulong csref)
        public void OnDrawComponent(CsRef csRef, float width)
        {
            rectWidth = width;
            DrawIntField("Test Int", ref tmp);
            Space();
            DrawFloatField("Test Float", ref tmpF);
            Space();
            DrawVector2("Test Vector2", ref tmpVec);
            Space();
            DrawVector3("Test Vector3", ref tmpVec3);
            Space();
            DrawQuaternion("Test Quaternion", ref tmpQuaternion);
            Space();
            DrawBoolField("Test Bool", ref boolCheckbox);
            Space();
            DrawString("Test String", ref testString);
            Space();
            object oo = mm;
            DrawEnum("Test Combo", ref mm);
        }

        public void DrawIntField(string label, ref int value)
        {
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, 100.0f);

            ImGui.Text(label);

            ImGui.NextColumn();
            ImGui.PushItemWidth(rectWidth - 97.0f);

            ImGui.DragInt($"##{label}", ref value);

            ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public void DrawFloatField(string label, ref float value)
        {
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, 100.0f);

            ImGui.Text(label);

            ImGui.NextColumn();
            ImGui.PushItemWidth(rectWidth - 97.0f);

            ImGui.DragFloat($"##{label}", ref value);

            ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public void DrawBoolField(string label, ref bool value)
        {
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, 100.0f);

            ImGui.Text(label);

            ImGui.NextColumn();

            ImGui.Checkbox($"##{label}", ref value);

            ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public void DrawVector2(string label, ref Vector2 value)
        {
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, 100.0f);

            ImGui.Text(label);

            var paddingVec = new sn.Vector2() { X = 0.0f, Y = 0.0f };

            ImGui.NextColumn();
            ImGui.PushItemWidth(rectWidth - 135.0f);
            ImGui.PushMultiItemsWidths(2, ImGui.CalcItemWidth());
            ImGui.PushStyleVar((int)ImGuiStyleVar_._ItemSpacing, in paddingVec);

            string nameX = "##X_" + label;
            string nameY = "##Y_" + label;

            ImGui.Text(" X ");
            ImGui.SameLine();
            ImGui.DragFloat(nameX, ref value.X, 0.1f);
            ImGui.PopItemWidth();
            ImGui.SameLine();

            ImGui.Text(" Y ");
            ImGui.SameLine();
            ImGui.DragFloat(nameY, ref value.Y, 0.1f);
            ImGui.PopItemWidth();
            ImGui.PopStyleVar();
            ImGui.Columns(1);
        }

        public void DrawVector3(string label, ref Vector3 value)
        {
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, 100.0f);

            ImGui.Text(label);

            var paddingVec = new sn.Vector2() { X = 0.0f, Y = 0.0f };

            ImGui.NextColumn();
            ImGui.PushItemWidth(rectWidth - 152.0f);
            ImGui.PushMultiItemsWidths(3, ImGui.CalcItemWidth());
            ImGui.PushStyleVar((int)ImGuiStyleVar_._ItemSpacing, in paddingVec);

            string nameX = "##X_" + label;
            string nameY = "##Y_" + label;
            string nameZ = "##Z_" + label;

            ImGui.Text(" X ");
            ImGui.SameLine();
            ImGui.DragFloat(nameX, ref value.X, 0.1f);
            ImGui.PopItemWidth();
            ImGui.SameLine();

            ImGui.Text(" Y ");
            ImGui.SameLine();
            ImGui.DragFloat(nameY, ref value.Y, 0.1f);
            ImGui.PopItemWidth();
            ImGui.SameLine();

            ImGui.Text(" Z ");
            ImGui.SameLine();
            ImGui.DragFloat(nameZ, ref value.Z, 0.1f);
            ImGui.PopItemWidth();

            ImGui.PopStyleVar();
            ImGui.Columns(1);
        }

        public void DrawQuaternion(string label, ref Quaternion value)
        {
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, 100.0f);

            ImGui.Text(label);

            var paddingVec = new sn.Vector2() { X = 0.0f, Y = 0.0f };

            ImGui.NextColumn();
            ImGui.PushItemWidth(rectWidth - 170.0f);
            ImGui.PushMultiItemsWidths(4, ImGui.CalcItemWidth());
            ImGui.PushStyleVar((int)ImGuiStyleVar_._ItemSpacing, in paddingVec);

            string nameX = "##X_" + label;
            string nameY = "##Y_" + label;
            string nameZ = "##Z_" + label;
            string nameW = "##W_" + label;

            ImGui.Text(" X ");
            ImGui.SameLine();
            ImGui.DragFloat(nameX, ref value.X, 0.1f);
            ImGui.PopItemWidth();
            ImGui.SameLine();

            ImGui.Text(" Y ");
            ImGui.SameLine();
            ImGui.DragFloat(nameY, ref value.Y, 0.1f);
            ImGui.PopItemWidth();
            ImGui.SameLine();

            ImGui.Text(" Z ");
            ImGui.SameLine();
            ImGui.DragFloat(nameZ, ref value.Z, 0.1f);
            ImGui.PopItemWidth();
            ImGui.SameLine();

            ImGui.Text(" W ");
            ImGui.SameLine();
            ImGui.DragFloat(nameW, ref value.W, 0.1f);
            ImGui.PopItemWidth();

            ImGui.PopStyleVar();
            ImGui.Columns(1);
        }

        public void DrawString(string label, ref string str)
        {
            ulong ptr = 2;
            Dll.UI_Inspector.ShowText(Game.gameRef, label, str, str.Length, ref ptr);
            string str2 = ReadCString(ptr);
        }

        public void DrawEnum(string label, ref myEnum obj)
        {
            Type type = obj.GetType();
            if(type.IsEnum)
            {
                var items = Enum.GetNames(type);
                string combo_preview_value = Enum.GetName(type, obj);
                string newValue = "";

                ImGui.Columns(2, "", hasBorder);
                ImGui.SetColumnWidth(0, 100.0f);

                ImGui.Text(label);

                ImGui.NextColumn();
                ImGui.PushItemWidth(rectWidth - 97.0f);

                if (ImGui.BeginCombo($"##{label}", combo_preview_value))
                {
                    foreach (var item in items)
                    {
                        bool is_selected = (item == combo_preview_value);
                        if (ImGui.Selectable(item, is_selected))
                            newValue = item;
                    }
                    ImGui.EndCombo();
                }
                if(newValue.Length != 0)
                {
                    obj = (myEnum)Enum.Parse(type, newValue);
                    Console.WriteLine(obj);
                }

                ImGui.PopItemWidth();
                ImGui.Columns(1);
            }
        }

        public static string ReadCString(ulong ptr)
        {
            string str = "";
            unsafe
            {
                var bytes = new List<byte>();
                byte* byte_ptr = (byte*)ptr;
                for (ulong i = 0; i < 1024; i++, byte_ptr++)
                {
                    byte m_byte = *byte_ptr;
                    if (m_byte == 0)
                        break;
                    bytes.Add(m_byte);
                }
                str = Encoding.UTF8.GetString(bytes.ToArray());
            }
            return str;
        }

        [DllImport(Paths.Exe, EntryPoint = "UserInterface_SetCallbacks")]
        private static extern void dll_SetCallbacks(CppRef cppRef, Callbacks.Callback callbacks);
    }
}
