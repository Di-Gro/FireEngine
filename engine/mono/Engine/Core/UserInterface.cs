using System;
using System.Collections.Generic;
using System.Collections;
using System.Text;
using System.Runtime.InteropServices;

using UI;
using EngineDll;
using FireYaml;

using sn = System.Numerics;
using System.Linq.Expressions;

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
        public delegate void Callback(CsRef csRef);
        public delegate void RequestName(CsRef csRef);
        public delegate void OnDrawActorTags(CsRef csRef);
        public delegate void DrawAssetEditor();

        public Callback onDrawComponent;
        public RequestName requestComponentName;
        public OnDrawActorTags onDrawActorTags;
        public DrawAssetEditor onDrawAssetEditor;
    }

    class UserInterface
    {
        public static UserInterface Instance { get; private set; }

        public static int SelectedAsset {
            get => Dll.UserInterface.SelectedAsset_get(Game.uiRef);
            set => Dll.UserInterface.SelectedAsset_set(Game.uiRef, value);
        }

        private Callbacks m_callbacks;

        private CppRef m_UICppRef;

        private AssetEditor m_AssetEditor = new AssetEditor();

        public static void cpp_Init(CppRef cppRef)
        {
            Instance = new UserInterface();
            Instance.m_UICppRef = cppRef;

            Instance.m_callbacks = new Callbacks();
            Instance.m_callbacks.onDrawComponent = new Callbacks.Callback(Instance.OnDrawComponent);
            Instance.m_callbacks.requestComponentName = new Callbacks.RequestName(Instance.RequestComponentName);
            Instance.m_callbacks.onDrawActorTags = new Callbacks.OnDrawActorTags(Instance.DrawActorTags);
            Instance.m_callbacks.onDrawAssetEditor = new Callbacks.DrawAssetEditor(Instance.m_AssetEditor.Draw);

            dll_SetCallbacks2(cppRef, Instance.m_callbacks);
        }

        public void DrawActorTags(CsRef csRef) {
            var actor = CppLinked.GetObjectByRef(csRef) as Actor;

            var type = typeof(Flag);
            var names = Enum.GetNames(type);
            var values = Enum.GetValues(type);
            var lastFlags = actor.Flags;

            var label = "Flags";
            var width = Dll.ImGui.CalcTextWidth(label);
            var itemSpacing = new sn.Vector2(3, 5);

            ImGui.SetNextItemWidth(width);           

            if (ImGui.BeginCombo($"##actorFlags_{csRef}", "", (int)ImGuiComboFlags_._HeightLarge)) {
                GUI.Space();
                for (int i = 0; i < names.Length; i++) {
                    var name = names.GetValue(i);
                    var flag = (Flag)values.GetValue(i);
                    var hasFlag = actor.Has(flag);

                    if(flag == Flag.Null)
                        continue;

                    ImGui.PushStyleVar((int)ImGuiStyleVar_._ItemSpacing, in itemSpacing);

                    if (ImGui.Selectable($"  {name}  ##_{csRef}", hasFlag, (int)ImGuiSelectableFlags_._DontClosePopups)) {
                        if(hasFlag)
                            actor.Flags ^= flag;
                        else
                            actor.Flags |= flag;
                    }
                    ImGui.PopStyleVar(1);
                }
                GUI.Space();
                ImGui.EndCombo();
            }
            if (lastFlags != actor.Flags)
                Assets.MakeDirty(actor.scene.assetIdHash);
        }

        public void OnDrawComponent(CsRef csRef) {
            var component = CppLinked.GetObjectByRef(csRef) as Component;

            GUI.style = Marshal.PtrToStructure<UI.ImGuiStyle>(ImGui.GetStyle());
            // GUI.rectWidth = Dll.ImGui.GetContentRegionAvail().X;
            GUI.groupId = csRef.value;
            GUI.groupAssetIdHash = component.actor.scene.assetIdHash;

            var instance = CppLinked.GetObjectByRef(csRef);
            var type = instance.GetType();

            DrawObject(type, ref instance);
        }

        public void DrawObject(Type type, ref object instance) {
            var serializer = FireWriter.GetSerializer(type);

            if (serializer.NeedIncludeBase(type)) {
                DrawObject(type.BaseType, ref instance);
                GUI.Space();
            }
            GUI.subGroupId = type.GetHashCode();

            var fields = FireWriter.GetFields(type, instance, serializer);

            for (int i = 0; i < fields.Count; i++) {
                var field = fields[i];

                if (field.GetCustomAttribute<SpaceAttribute>() != null) {
                    // ImGui.Separator();
                    GUI.Space();
                    GUI.Space();
                }

                m_DrawField(field);

                if (i != fields.Count - 1)
                    GUI.Space();
            }

            serializer.OnDrawGui(type, ref instance);
        }

        
        private void m_DrawField(IField field) {
            var type = GUI.GetSupportedType(field.type);
            if (type == null)
                return;

            var hash = type.FullName.GetHashCode();

            var range = field.GetCustomAttribute<RangeAttribute>();
            var drawer = GUI.fieldDrawers[hash];

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
        public delegate void FieldDrawer(string label, IField field, RangeAttribute range = null);

        public static readonly Dictionary<int, FieldDrawer> fieldDrawers = new Dictionary<int, FieldDrawer>() {
            { typeof(int).FullName.GetHashCode(), DrawIntField },
            { typeof(float).FullName.GetHashCode(), DrawFloatField },
            { typeof(bool).FullName.GetHashCode(), DrawBoolField },
            { typeof(Vector2).FullName.GetHashCode(), DrawVector2 },
            { typeof(Vector3).FullName.GetHashCode(), DrawVector3 },
            { typeof(Quaternion).FullName.GetHashCode(), DrawQuaternion },
            { typeof(Enum).FullName.GetHashCode(), DrawEnum },
            { typeof(string).FullName.GetHashCode(), DrawString },
            { typeof(IAsset).FullName.GetHashCode(), DrawAsset },
            { typeof(Actor).FullName.GetHashCode(), DrawActor },
            { typeof(Component).FullName.GetHashCode(), DrawComponent },
            { typeof(IList).FullName.GetHashCode(), DrawList }
        };
        
        /// Context ->
        public static UI.ImGuiStyle style;

        public static ulong groupId {
            get => Dll.UserInterface.groupId_get(Game.uiRef);
            set => Dll.UserInterface.groupId_set(Game.uiRef, value);
        }

        public static int subGroupId {
            get => Dll.UserInterface.subGroupId_get(Game.uiRef);
            set => Dll.UserInterface.subGroupId_set(Game.uiRef, value);
        }

        public static int groupAssetIdHash {
            get => Dll.UserInterface.groupAssetIdHash_get(Game.uiRef);
            set => Dll.UserInterface.groupAssetIdHash_set(Game.uiRef, value);
        }

        public static float rectWidth {
            get => Dll.UserInterface.rectWidth_get(Game.uiRef);
            set => Dll.UserInterface.rectWidth_set(Game.uiRef, value);
        }

        public static bool active = true;
        /// <-
        
        public static float labelWidth = 100;
        public static float padding = 20;
        public static float floatSpeed = 0.01f;
        public static bool hasBorder = false;

        public static sn.Vector2 lineSpacing = new sn.Vector2(0, 3);
        public static sn.Vector2 headerSpacing = new sn.Vector2(0, 8);

        private static TestGUIFields testObj = new TestGUIFields();

        public static Type GetSupportedType(Type type) {
            if (fieldDrawers.ContainsKey(type.FullName.GetHashCode()))
                return type;

            if (type.IsEnum)
                return typeof(Enum);

            bool isAsset = type.GetInterface(nameof(IAsset)) != null;
            if (isAsset)
                return typeof(IAsset);

            if (FireWriter.IsComponent(type))
                return typeof(Component);

            if (FireWriter.IsList(type))
                return typeof(IList);

            return null;
        }

        public static void Space() { ImGui.Dummy(lineSpacing); }
        public static void HeaderSpace() { ImGui.Dummy(headerSpacing); }

        public static void DrawIntField(string label, IField field, RangeAttribute range = null) {
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
                changed = ImGui.DragInt($"##{label}_{groupId}_{subGroupId}", ref intValue);
            else
                changed = ImGui.SliderInt($"##{label}_{groupId}_{subGroupId}", ref intValue, min, max);

            if (changed) {
                field.SetValue(intValue);
                Assets.MakeDirty(groupAssetIdHash);
            }
            
            ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public static void DrawFloatField(string label, IField field, RangeAttribute range = null) {
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
                changed = ImGui.DragFloat($"##{label}_{groupId}_{subGroupId}", ref floatValue, floatSpeed, min, max);
            else
                changed = ImGui.SliderFloat($"##{label}_{groupId}_{subGroupId}", ref floatValue, min, max);

            if (changed) {
                field.SetValue(floatValue);
                Assets.MakeDirty(groupAssetIdHash);
            }
            ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public static void DrawBoolField(string label, IField field, RangeAttribute range = null) {
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, labelWidth);

            ImGui.Text(label);

            ImGui.NextColumn();

            var boolValue = (bool)field.Value;
            bool changed = ImGui.Checkbox($"##{label}_{groupId}_{subGroupId}", ref boolValue);
            if (changed) {
                field.SetValue(boolValue);
                Assets.MakeDirty(groupAssetIdHash);
            }
            // ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public static void DrawVector2(string label, IField field, RangeAttribute range = null) {
            bool changed = false;

            string nameX = $"##X_{label}_{groupId}_{subGroupId}";
            string nameY = $"##Y_{label}_{groupId}_{subGroupId}";

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

            if (changed) {
                field.SetValue(vecValue);
                Assets.MakeDirty(groupAssetIdHash);
            }
        }

        public static void DrawVector3(string label, IField field, RangeAttribute range = null) {
            bool changed = false;

            var vecValue = (Vector3)field.Value;

            var colorAttr = field.GetCustomAttribute<ColorAttribute>();
            if (colorAttr != null) {
                changed = Dll.UI_Inspector.ShowColor3(Game.gameRef, label, ref vecValue);
            } else {
                string nameX = $"##X_{label}_{groupId}_{subGroupId}";
                string nameY = $"##Y_{label}_{groupId}_{subGroupId}";
                string nameZ = $"##Z_{label}_{groupId}_{subGroupId}";

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
            }
            if (changed) {
                field.SetValue(vecValue);
                Assets.MakeDirty(groupAssetIdHash);
            }
        }

        public static void DrawQuaternion(string label, IField field, RangeAttribute range = null) {
            string nameX = $"##X_{label}_{groupId}_{subGroupId}";
            string nameY = $"##Y_{label}_{groupId}_{subGroupId}";
            string nameZ = $"##Z_{label}_{groupId}_{subGroupId}";
            string nameW = $"##W_{label}_{groupId}_{subGroupId}";

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

            if (changed) {
                field.SetValue(quat);
                Assets.MakeDirty(groupAssetIdHash);
            }
        }

        public static void DrawString(string label, IField field, RangeAttribute range = null) {
            var value = (string)field.Value;

            var readOnlyAttr = field.GetCustomAttribute<ReadOnlyAttribute>();
            var readOnly = readOnlyAttr != null;

            ImGuiInputTextFlags_ flags = 0;
            if (readOnly)
                flags |= ImGuiInputTextFlags_._ReadOnly;

            ulong ptr = 0;
            bool changed = Dll.UI_Inspector.ShowText(Game.gameRef, label, $"##{label}_{groupId}_{subGroupId}", value, value.Length, ref ptr, (int)flags);
            if (changed) {
                field.SetValue(ReadCString(ptr));
                Assets.MakeDirty(groupAssetIdHash);
            }
        }

        public static void DrawText(string label, string text) {
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, labelWidth);

            ImGui.Text(label);

            ImGui.NextColumn();
            ImGui.PushItemWidth(rectWidth - labelWidth - padding);

            ImGui.Text(text);

            ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public static void DrawTextWrapped(string label, string text)
        {
            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, labelWidth);

            ImGui.Text(label);

            ImGui.NextColumn();
            ImGui.PushItemWidth(rectWidth - labelWidth - padding);

            ImGui.TextWrapped(text);

            ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public static void DrawEnum(string label, IField field, RangeAttribute range = null) {
            if (!field.type.IsEnum)
                return;

            var values = Enum.GetNames(field.type);
            string value = Enum.GetName(field.type, field.Value);
            string newValue = "";

            var itemSpacing = new sn.Vector2(3, 5);

            ImGui.Columns(2, "", hasBorder);
            ImGui.SetColumnWidth(0, labelWidth);

            ImGui.Text(label);
            
            ImGui.NextColumn();
            ImGui.PushItemWidth(rectWidth - labelWidth - padding);

            PushEnumPopupStyle();
            if (ImGui.BeginCombo($"##{label}_{groupId}_{subGroupId}", value, (int)ImGuiComboFlags_._HeightLarge)) {
                Space();
                foreach (var item in values) {
                    ImGui.PushStyleVar((int)ImGuiStyleVar_._ItemSpacing, in itemSpacing);
                    ImGui.PushStyleColor((int)ImGuiCol_._Text, new sn.Vector4(0.0f, 0.0f, 0.0f, 1.0f));

                    if (ImGui.Selectable($"  {item}##_{groupId}_{subGroupId}", item == value))
                        newValue = item;

                    ImGui.PopStyleColor(1);
                    ImGui.PopStyleVar(1);
                }
                Space();
                ImGui.EndCombo();
            }
            PopEnumPopupStyle();

            if (newValue.Length != 0) {
                field.SetValue(Enum.Parse(field.type, newValue));
                Assets.MakeDirty(groupAssetIdHash);
            }

            ImGui.PopItemWidth();
            ImGui.Columns(1);
        }

        public static void DrawAsset(string label, IField field, RangeAttribute range = null) {
            var instance = field.Value;
            var iasset = field.Value as FireYaml.IAsset;

            var assetIdHash = 0;
            var scriptId = GUIDAttribute.GetGuid(field.type);
            var scriptIdHash = GUIDAttribute.GetGuidHash(field.type); 

            if (instance != null)
                assetIdHash = iasset.assetIdHash;

            bool changed = Dll.UI_Inspector.ShowAsset(Game.gameRef, label, scriptIdHash, ref assetIdHash, GUI.active);

            if (changed) {
                instance = FireYaml.FireReader.CreateInstance(field.type);
                iasset = instance as FireYaml.IAsset;
                field.SetValue(instance);

                var assetId = AssetStore.GetAssetGuid(assetIdHash);

                FireYaml.FireReader.InitIAsset(ref instance, assetId, 0);

                iasset.LoadAsset();
                Assets.MakeDirty(groupAssetIdHash);
            }
        }

        public static bool DrawAsset(string label, Type type, int assetIdHash, out object changedAsset) {
            changedAsset = null;
            
            var scriptId = GUIDAttribute.GetGuid(type); // store.GetScriptIdByTypeName(type.FullName);
            var scriptIdHash = GUIDAttribute.GetGuidHash(type); // scriptId.GetHashCode();          

            bool changed = Dll.UI_Inspector.ShowAsset(Game.gameRef, label, scriptIdHash, ref assetIdHash, GUI.active);

            if (changed) {
                changedAsset = FireYaml.FireReader.CreateInstance(type);
                var iasset = changedAsset as FireYaml.IAsset;

                var assetId = AssetStore.GetAssetGuid(assetIdHash);

                FireYaml.FireReader.InitIAsset(ref changedAsset, assetId, 0);

                iasset.LoadAsset();
                Assets.MakeDirty(groupAssetIdHash);
            }
            return changed;
        }

        public static void DrawActor(string label, IField field, RangeAttribute range = null) {
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
                Assets.MakeDirty(groupAssetIdHash);
            }
        }

        public static void DrawComponent(string label, IField field, RangeAttribute range = null) {
            var store = AssetStore.Instance;
            var component = field.Value as Engine.Component;

            var csRef = CsRef.NullRef;
            var cppRef = CppRef.NullRef;
            var scriptId = GUIDAttribute.GetGuid(field.type); //store.GetScriptIdByTypeName(field.type.FullName);
            var scriptIdHash = GUIDAttribute.GetGuidHash(field.type); // scriptId.GetHashCode();

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
                Assets.MakeDirty(groupAssetIdHash);
            }
        }

        public static void DrawList(string label, IField field, RangeAttribute range = null) {
            var valueType = field.type.GetGenericArguments()[0];
            var drawerType = GetSupportedType(valueType);
            if (drawerType == null)
                return;

            if (field.Value == null)
                field.SetValue(FireReader.CreateInstance(field.type));

            var list = field.Value as IList;
            if (list == null)
                return;

            var drawer = fieldDrawers[drawerType.FullName.GetHashCode()];

            var flags = ImGuiTreeNodeFlags_._Framed | ImGuiTreeNodeFlags_._DefaultOpen;
            if (CollapsingHeader($" {label}", flags, out var size)) {
                var textSize = Dll.ImGui.CalcTextSize(" + ");

                var prevRectWidth = rectWidth;
                rectWidth = prevRectWidth - 25;

                int indexToRemove = -1;

                for (int index = 0; index < list.Count; index++) {
                    var itemField = new Field(list, valueType, index);

                    var prevPos = GetCursorPos();
                    drawer.Invoke($"{label}: {index}", itemField);
                    var nextPos = GetCursorPos();

                    if (m_DrawListButton($" - ##_{groupId}_{subGroupId}_{label}_{index}", size, prevPos))
                        indexToRemove = index;

                    ImGui.SetCursorPos(nextPos);

                    Space();
                }
                rectWidth = prevRectWidth;

                if (indexToRemove >= 0)
                    list.RemoveAt(indexToRemove);

                if (m_DrawListButton($" + ##_{groupId}_{subGroupId}_{label}_A", size, GetCursorPos())) {
                    if (valueType.IsValueType)
                        list.Add(FireReader.CreateInstance(valueType));
                    else if (valueType == typeof(string))
                        list.Add("");
                    else
                        list.Add(null);
                }
            }
        }

        public static sn.Vector2 GetCursorPos() {
            var x = ImGui.GetCursorPosX();
            var y = ImGui.GetCursorPosY();

            return new sn.Vector2(x, y);
        }

        public static bool m_DrawListButton(string text, Vector3 headerSize, sn.Vector2 cursorPos) {
            // var textSize = Dll.ImGui.CalcTextSize(text);

            cursorPos.X = headerSize.X - 4;
            // cursorPos.Y += (headerSize.Y - textSize.Y) / 2;

            ImGui.SetCursorPos(cursorPos);

            // ImGui.PushStyleColor((int)ImGuiCol_._Button, new sn.Vector4(1.0f, 1.0f, 1.0f, 0.2f));
            var isClicked = ImGui.Button(text);
            // ImGui.PopStyleColor(1);

            return isClicked;
        }

        public static bool CollapsingHeader(string label, ImGuiTreeNodeFlags_ flags, out Vector3 size) {

            float headerWidth = rectWidth - padding;

            Space();

            ImGui.Columns(2, "CollapsingHeader", false);
            ImGui.SetColumnWidth(0, headerWidth);
            ImGui.SetColumnWidth(1, padding);

            bool isOpen = ImGui.CollapsingHeader($"{label}##_{groupId}_{subGroupId}", (int)flags);
            size = Dll.ImGui.GetItemRectSize();

            ImGui.NextColumn();
            ImGui.Columns(1);

            Space();

            return isOpen;
        }

        public static void TestDrawers() {
            object instance = testObj;

            UserInterface.Instance.DrawObject(instance.GetType(), ref instance);
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
    
        public static void PushEnumPopupStyle() {
            ImGui.PushStyleVar((int)ImGuiStyleVar_._WindowPadding, new sn.Vector2( 0.0f, 0.0f ));
            // ImGui.PushStyleVar((int)ImGuiStyleVar_._PopupRounding, 10.0f);
            ImGui.PushStyleVar((int)ImGuiStyleVar_._PopupBorderSize, 0.0f);
            ImGui.PushStyleVar((int)ImGuiStyleVar_._FramePadding, new sn.Vector2( 0.0f, 3.0f ));
            ImGui.PushStyleVar((int)ImGuiStyleVar_._FrameRounding, 3.0f);
            ImGui.PushStyleVar((int)ImGuiStyleVar_._ItemSpacing, new sn.Vector2(0.0f, 0.0f ));

            ImGui.PushStyleColor((int)ImGuiCol_._PopupBg, new sn.Vector4( 0.7f, 0.7f ,0.7f ,1.0f ));
            ImGui.PushStyleColor((int)ImGuiCol_._HeaderHovered, new sn.Vector4( 0.8f, 0.8f ,0.9f ,1.0f ));
            ImGui.PushStyleColor((int)ImGuiCol_._Header, new sn.Vector4( 0.2f, 0.2f, 0.4f, 0.4f ));
        }

        public static void PopEnumPopupStyle() {
            ImGui.PopStyleVar(5);
            ImGui.PopStyleColor(3);
        }

    }
}
