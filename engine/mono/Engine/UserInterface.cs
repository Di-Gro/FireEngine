﻿using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using UI;
using EngineDll;

using sn = System.Numerics;

namespace Engine
{

    class TestGUIFields {

        public enum TestEnum {One, Two, Three, Four, Five}

        public int _int = 14;
        public float padding = 0;
        public float width = 0;
        public bool _bool = false;
        public Vector2 _vec2 = new Vector2(1, 2);
        public Vector3 _vec3 = new Vector3(1, 2, 3);
        public Quaternion _quat = new Quaternion();
        public TestEnum _enum = TestEnum.Four;
        public string _str = "";

    }


    [StructLayout(LayoutKind.Sequential)]
    public struct Callbacks {
        public delegate void Callback(CsRef csRef, float width);
        public delegate void RequestName(CsRef csRef);

        public Callback onDrawComponent;
        public RequestName requestComponentName;
    }

    class UserInterface
    {
        private delegate void FieldDrawer(string label, FireYaml.Field field, RangeAttribute range = null);

        public static UserInterface Instance { get; private set; }

        private Callbacks _callbacks;

        private Dictionary<int, FieldDrawer> m_fieldDrawers = new Dictionary<int, FieldDrawer>();

        // private float rectWidth = 0;
        // private float floatSpeed = 0.01f;
        // private bool hasBorder = false;

        private int tmp = 155;
        private float tmpF = 155.0f;
        private Vector2 tmpVec = new Vector2(5.0f, 5.0f);
        private Vector3 tmpVec3 = new Vector3(5.0f, 5.0f, 5.0f);
        private Quaternion tmpQuaternion = new Quaternion(5.0f, 5.0f, 5.0f, 1.0f);
        private bool boolCheckbox = true;
        private string testString = "my test string";

        private CppRef _cppRef;


        public UserInterface() {
            m_fieldDrawers.Add(typeof(int).FullName.GetHashCode(), GUI.DrawIntField);
            m_fieldDrawers.Add(typeof(float).FullName.GetHashCode(), GUI.DrawFloatField);
            m_fieldDrawers.Add(typeof(bool).FullName.GetHashCode(), GUI.DrawBoolField);
            m_fieldDrawers.Add(typeof(Vector2).FullName.GetHashCode(), GUI.DrawVector2);
            m_fieldDrawers.Add(typeof(Vector3).FullName.GetHashCode(), GUI.DrawVector3);
            m_fieldDrawers.Add(typeof(Quaternion).FullName.GetHashCode(), GUI.DrawQuaternion);
            m_fieldDrawers.Add(typeof(Enum).FullName.GetHashCode(), GUI.DrawEnum);
            m_fieldDrawers.Add(typeof(string).FullName.GetHashCode(), GUI.DrawString);
            m_fieldDrawers.Add(typeof(FireYaml.IAsset).FullName.GetHashCode(), GUI.DrawAsset);
            m_fieldDrawers.Add(typeof(Actor).FullName.GetHashCode(), GUI.DrawActor);
            m_fieldDrawers.Add(typeof(Component).FullName.GetHashCode(), GUI.DrawComponent);
        }

        public static void cpp_Init(CppRef cppRef)
        {
            Instance = new UserInterface();
            Instance._cppRef = cppRef;

            Instance._callbacks = new Callbacks();
            Instance._callbacks.onDrawComponent = new Callbacks.Callback(Instance.OnDrawComponent);
            Instance._callbacks.requestComponentName = new Callbacks.RequestName(Instance.RequestComponentName);

            dll_SetCallbacks2(cppRef, Instance._callbacks);
        }

        public void OnDrawComponent(CsRef csRef, float width) {
            GUI.rectWidth = width;
            GUI.style = Marshal.PtrToStructure<UI.ImGuiStyle>(ImGui.GetStyle());
            
            var instance = CppLinked.GetObjectByRef(csRef);
            DrawObject(ref instance);
        }

        public void DrawObject(ref object instance) {
            var type = instance.GetType();
            var serializer = FireYaml.Serializer.GetSerializer(type);

            var fields = FireYaml.Serializer.GetFields(type, instance, serializer);

            for (int i = 0; i < fields.Count; i++) {
                m_DrawField(fields[i]);

                if (i != fields.Count - 1)
                    GUI.Space();
            }

            serializer.OnDrawGui(type, ref instance);
        }

        private void m_DrawField(FireYaml.Field field) {
            var typeName = field.type.FullName;

            if(field.type.IsEnum)
                typeName = typeof(Enum).FullName;

            bool isAsset = field.type.GetInterface(nameof(FireYaml.IAsset)) != null;
            if(isAsset)
                typeName = typeof(FireYaml.IAsset).FullName;

            if(FireYaml.Serializer.IsComponent(field.type))
                typeName = typeof(Component).FullName;

            var hash = typeName.GetHashCode();

            if(!m_fieldDrawers.ContainsKey(hash))
                return;

            var range = field.GetCustomAttribute<RangeAttribute>();
            var drawer = m_fieldDrawers[hash];

            drawer(field.name, field, range);
        }        

        public void RequestComponentName(CsRef csRef) {
            var component = CppLinked.GetObjectByRef(csRef);
            var name = component.GetType().Name;

            Dll.UI_Inspector.SetComponentName(Game.gameRef, name);
        }


        [DllImport(Paths.Exe, EntryPoint = "UserInterface_SetCallbacks2")]
        private static extern void dll_SetCallbacks2(CppRef cppRef, Callbacks callbacks);
    }

    public static class GUI {

        public static float rectWidth = 0;
        public static float labelWidth = 100;
        public static float padding = 20;
        public static float floatSpeed = 0.01f;
        public static bool hasBorder = false;

        public static sn.Vector2 lineSpacing = new sn.Vector2(0, 3);
        public static sn.Vector2 headerSpacing = new sn.Vector2(0, 8);

        public static UI.ImGuiStyle style;

        private static TestGUIFields testObj = new TestGUIFields();

        public static void Space() { ImGui.Dummy(lineSpacing); }
        public static void HeaderSpace() { ImGui.Dummy(headerSpacing); }


        public static void DrawIntField(string label, FireYaml.Field field, RangeAttribute range = null) {
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, labelWidth);

            ImGui.Text(label);

            ImGui.NextColumn();
            ImGui.PushItemWidth(rectWidth - labelWidth - padding);

            var intValue = (int)field.Value;
            int min = range != null ? range.imin : 0;
            int max = range != null ? range.imax : 0;

            bool changed = false;

            if (range == null)
                changed = ImGui.DragInt($"##{label}", ref intValue);
            else
                changed = ImGui.SliderInt($"##{label}", ref intValue, min, max);

            if (changed)
                field.SetValue(intValue);

            ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public static void DrawFloatField(string label, FireYaml.Field field, RangeAttribute range = null) {
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, labelWidth);

            ImGui.Text(label);

            ImGui.NextColumn();
            ImGui.PushItemWidth(rectWidth - labelWidth - padding);

            var floatValue = (float)field.Value;
            float min = range != null ? range.fmin : 0;
            float max = range != null ? range.fmax : 0;

            bool changed = false;

            if (range == null)
                changed = ImGui.DragFloat($"##{label}", ref floatValue, floatSpeed, min, max);
            else
                changed = ImGui.SliderFloat($"##{label}", ref floatValue, min, max);

            if (changed)
                field.SetValue(floatValue);

            ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public static void DrawBoolField(string label, FireYaml.Field field, RangeAttribute range = null) {
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, labelWidth);

            ImGui.Text(label);

            ImGui.NextColumn();

            var boolValue = (bool)field.Value;
            bool changed = ImGui.Checkbox($"##{label}", ref boolValue);
            if (changed)
                field.SetValue(boolValue);

            // ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public static void DrawVector2(string label, FireYaml.Field field, RangeAttribute range = null) {
            bool changed = false;

            string nameX = "##X_" + label;
            string nameY = "##Y_" + label;

            var vecValue = (Vector2)field.Value;
            float min = range != null ? range.fmin : 0;
            float max = range != null ? range.fmax : 0;

            float textSize = Dll.ImGui.CalcTextWidth(" X ");
            float textWidth = (textSize + style.ItemSpacing.X) * 2;
            float columns2Width = rectWidth - labelWidth - textWidth;
                        
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, labelWidth);

            ImGui.Text(label);

            ImGui.NextColumn();
            ImGui.PushMultiItemsWidths(2, columns2Width - padding);

            /// Item 1
            ImGui.PushID(1);
            ImGui.Text(" X ");
            ImGui.SameLine();

            if (range == null)
                changed = changed || ImGui.DragFloat(nameX, ref vecValue.X, floatSpeed, min, max);
            else
                changed = changed || ImGui.SliderFloat(nameX, ref vecValue.X, min, max);

            ImGui.PopID();
            ImGui.PopItemWidth();

            /// Item 2
            ImGui.SameLine();
            ImGui.PushID(2);
            ImGui.Text(" Y ");
            ImGui.SameLine();

            if (range == null)
                changed = changed || ImGui.DragFloat(nameY, ref vecValue.Y, floatSpeed, min, max);
            else
                changed = changed || ImGui.SliderFloat(nameY, ref vecValue.Y, min, max);

            ImGui.PopID();
            ImGui.PopItemWidth();

            /// End
            ImGui.Columns(1);

            if (changed)
                field.SetValue(vecValue);
        }

        public static void DrawVector3(string label, FireYaml.Field field, RangeAttribute range = null) {
            bool changed = false;

            string nameX = "##X_" + label;
            string nameY = "##Y_" + label;
            string nameZ = "##Z_" + label;

            var vecValue = (Vector3)field.Value;
            float min = range != null ? range.fmin : 0;
            float max = range != null ? range.fmax : 0;        

            float textSize = Dll.ImGui.CalcTextWidth(" X ");
            float textWidth = (textSize + style.ItemSpacing.X) * 3;
            float columns2Width = rectWidth - labelWidth - textWidth;

            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, labelWidth);

            ImGui.Text(label);
           
            ImGui.NextColumn();
            ImGui.PushMultiItemsWidths(3, columns2Width - padding);

            ImGui.PushID(1);
            ImGui.Text(" X ");

            ImGui.SameLine();
            if (range == null)
                changed = changed || ImGui.DragFloat($"{nameX}", ref vecValue.X, floatSpeed, min, max);
            else
                changed = changed || ImGui.SliderFloat($"{nameX}", ref vecValue.X, min, max);
            ImGui.PopID();
            ImGui.PopItemWidth();

            ImGui.PushID(3);
            ImGui.SameLine();
            ImGui.Text(" Y ");
            
            ImGui.SameLine();
            if (range == null)
                changed = changed || ImGui.DragFloat($"{nameY}", ref vecValue.Y, floatSpeed, min, max);
            else
                changed = changed || ImGui.SliderFloat($"{nameY}", ref vecValue.Y, min, max);
            ImGui.PopID();
            ImGui.PopItemWidth();

            ImGui.PushID(5);
            ImGui.SameLine();
            ImGui.Text(" Z ");
            
            ImGui.SameLine();
            if (range == null)
                changed = changed || ImGui.DragFloat($"{nameZ}", ref vecValue.Z, floatSpeed, min, max);
            else
                changed = changed || ImGui.SliderFloat($"{nameZ}", ref vecValue.Z, min, max);
            ImGui.PopID();
            ImGui.PopItemWidth();

            ImGui.Columns(1);

            if (changed)
                field.SetValue(vecValue);
        }

        public static void DrawQuaternion(string label, FireYaml.Field field, RangeAttribute range = null) {
            string nameX = "##X_" + label;
            string nameY = "##Y_" + label;
            string nameZ = "##Z_" + label;
            string nameW = "##W_" + label;

            var quat = (Quaternion)field.Value;
            float min = range != null ? range.fmin : 0;
            float max = range != null ? range.fmax : 0;

            int itemCount = 4;
            float textSize = Dll.ImGui.CalcTextWidth(" X ");
            float textWidth = (textSize + style.ItemSpacing.X) * itemCount;
            float columns2Width = rectWidth - labelWidth - textWidth;
            
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, labelWidth);

            ImGui.Text(label);

            ImGui.NextColumn();
            ImGui.PushMultiItemsWidths(itemCount, columns2Width - padding);

            ImGui.Text(" X ");
            ImGui.SameLine();
            bool changed = ImGui.DragFloat(nameX, ref quat.X, 1, min, max);
            ImGui.PopItemWidth();
            ImGui.SameLine();

            ImGui.Text(" Y ");
            ImGui.SameLine();
            changed = changed || ImGui.DragFloat(nameY, ref quat.Y, 1, min, max);
            ImGui.PopItemWidth();
            ImGui.SameLine();

            ImGui.Text(" Z ");
            ImGui.SameLine();
            changed = changed || ImGui.DragFloat(nameZ, ref quat.Z, 1, min, max);
            ImGui.PopItemWidth();
            ImGui.SameLine();

            ImGui.Text(" W ");
            ImGui.SameLine();
            changed = changed || ImGui.DragFloat(nameW, ref quat.W, 1, min, max);
            ImGui.PopItemWidth();

            ImGui.Columns(1);

            if (changed)
                field.SetValue(quat);
        }

        public static void DrawString(string label, FireYaml.Field field, RangeAttribute range = null) {
            var value = (string)field.Value;

            ulong ptr = 0;
            bool changed = Dll.UI_Inspector.ShowText(Game.gameRef, label, value, value.Length, ref ptr);
            if (changed)
                field.SetValue(ReadCString(ptr));
        }

        public static void DrawEnum(string label, FireYaml.Field field, RangeAttribute range = null) {
            if (!field.type.IsEnum)
                return;

            var values = Enum.GetNames(field.type);
            string value = Enum.GetName(field.type, field.Value);
            string newValue = "";

            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, labelWidth);

            ImGui.Text(label);

            ImGui.NextColumn();
            ImGui.PushItemWidth(rectWidth - labelWidth - padding);

            if (ImGui.BeginCombo($"##{label}", value)) {
                foreach (var item in values) {
                    if (ImGui.Selectable(item, item == value))
                        newValue = item;
                }
                ImGui.EndCombo();
            }
            if (newValue.Length != 0) {
                field.SetValue(Enum.Parse(field.type, newValue));
            }

            ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public static void DrawAsset(string label, FireYaml.Field field, RangeAttribute range = null) {
            var store = FireYaml.AssetStore.Instance;
            var instance = field.Value;
            var iasset = field.Value as FireYaml.IAsset;

            var assetIdHash = 0;
            var scriptId = store.GetScriptIdByTypeName(field.type.FullName);
            var scriptIdHash = scriptId.GetHashCode();

            if (instance != null)
                assetIdHash = iasset.assetIdHash;

            bool changed = Dll.UI_Inspector.ShowAsset(Game.gameRef, label, scriptIdHash, ref assetIdHash);

            if (changed) {
                instance = FireYaml.Deserializer.CreateInstance(field.type);
                iasset = instance as FireYaml.IAsset;
                field.SetValue(instance);

                var assetId = store.GetAssetId(assetIdHash);

                FireYaml.Deserializer.InitIAsset(ref instance, assetId);

                iasset.LoadAsset();
            }
        }

        public static bool DrawAsset(string label, Type type, int assetIdHash,  out object changedAsset) {
            var store = FireYaml.AssetStore.Instance;

            changedAsset = null;
            
            var scriptId = store.GetScriptIdByTypeName(type.FullName);
            var scriptIdHash = scriptId.GetHashCode();          

            bool changed = Dll.UI_Inspector.ShowAsset(Game.gameRef, label, scriptIdHash, ref assetIdHash);

            if (changed) {
                changedAsset = FireYaml.Deserializer.CreateInstance(type);
                var iasset = changedAsset as FireYaml.IAsset;

                var assetId = store.GetAssetId(assetIdHash);

                FireYaml.Deserializer.InitIAsset(ref changedAsset, assetId);

                iasset.LoadAsset();
            }
            return changed;
        }

        public static void DrawActor(string label, FireYaml.Field field, RangeAttribute range = null) {
            var actor = field.Value as Engine.Actor;

            var csRef = CsRef.NullRef;
            var cppRef = CppRef.NullRef;

            if (actor != null) {
                if (actor.IsDestroyed) {
                    csRef = CsRef.MissingRef;
                } else {
                    csRef = actor.csRef;
                    cppRef = actor.cppRef;
                }
            }
            bool changed = Dll.UI_Inspector.ShowActor(Game.gameRef, label, ref csRef, cppRef);

            if (changed) {
                var newActor = CppLinked.GetObjectByRef(csRef);
                field.SetValue(newActor);
            }
        }

        public static void DrawComponent(string label, FireYaml.Field field, RangeAttribute range = null) {
            var store = FireYaml.AssetStore.Instance;
            var component = field.Value as Engine.Component;

            var csRef = CsRef.NullRef;
            var cppRef = CppRef.NullRef;
            var scriptId = store.GetScriptIdByTypeName(field.type.FullName);
            var scriptIdHash = scriptId.GetHashCode();

            if (component != null) {
                if(component.IsDestroyed) {
                    csRef = CsRef.MissingRef;
                } else {
                    csRef = component.csRef;
                    cppRef = component.cppRef;
                }
            }
            bool changed = Dll.UI_Inspector.ShowComponent(Game.gameRef, label, ref csRef, cppRef, scriptIdHash);

            if (changed) {
                var newComponent = CppLinked.GetObjectByRef(csRef);
                field.SetValue(newComponent);
            }
        }

        public static bool CollapsingHeader(string label, ImGuiTreeNodeFlags_ flags) {

            float headerWidth = rectWidth - padding;

            // ImGui.PushStyleVar((int)ImGuiStyleVar_._FrameRounding, 10);

            HeaderSpace();

            ImGui.Columns(2, "CollapsingHeader", false);
            ImGui.SetColumnWidth(0, headerWidth);
            ImGui.SetColumnWidth(1, padding);

            bool isOpen = ImGui.CollapsingHeader(label, (int)flags);
            
            ImGui.NextColumn();
            ImGui.Columns(1);
            // ImGui.PopStyleVar();

            HeaderSpace();

            return isOpen;
        }

        public static void TestDrawers() {
            object instance = testObj;

            UserInterface.Instance.DrawObject(ref instance);
        }


        public static string ReadCString(ulong ptr) {
            string str = "";
            unsafe {
                var bytes = new List<byte>();
                byte* byte_ptr = (byte*)ptr;
                for (ulong i = 0; i < 1024; i++, byte_ptr++) {
                    byte m_byte = *byte_ptr;
                    if (m_byte == 0)
                        break;
                    bytes.Add(m_byte);
                }
                str = Encoding.UTF8.GetString(bytes.ToArray());
            }
            return str;
        }
    
    }
}
