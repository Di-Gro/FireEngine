using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using FireBin;
using FireYaml;
using UI;
using EngineDll;

using ImVec2 = System.Numerics.Vector2;
using sn = System.Numerics;

namespace Engine {
    public class ValueField : FireYaml.IField {
        public string name { get; }
        public Type type { get; private set; }
        public object? Value { get; private set; }
        public object? Instance => null;

        public ValueField(Type type, object value)
        {
            Value = value;
            this.type = type;
        }
        public ValueField(object value)
        {
            Value = value;
            type = value.GetType();
        }
        public void SetValue(object? value) => Value = value;
        public object? GetValue() => Value;
        public TAttribute GetCustomAttribute<TAttribute>() where TAttribute : Attribute => null;
    }
    
    public class AssetEditor {
        // public string assetId = "25ddaa33-ea53-4749-a368-c2143d4c6a25";
        //public string assetId = "418dccc8-cf33-4f3e-b401-ae42cb9cc7f7"; 
        //public string assetId = "9a28a3c5-8e65-49da-ac20-61f608fa88bb"; 
        //public string assetId = "fcebda45-6fae-4d9e-86e2-0e3ac87b879f";

        // public int tmp_selectedAssetIdHash = "25ddaa33-ea53-4749-a368-c2143d4c6a25".GetAssetIDHash();

        private int m_assetIdHash;
        private IAsset m_iasset = null;
        private Asset m_asset = null;

        private bool HasLoadedAsset => m_iasset != null;

        public AssetEditor() { }

        public void Draw() {
            ImGui.PushStyleVar((int)ImGuiStyleVar_._WindowPadding, new ImVec2( 0.0f, 0.0f ));
            ImGui.PushStyleVar((int)ImGuiStyleVar_._FramePadding, new ImVec2( 0.0f, 3.0f ));
            ImGui.PushStyleVar((int)ImGuiStyleVar_._FrameRounding, 0.0f);
            ImGui.PushStyleVar((int)ImGuiStyleVar_._ItemSpacing, new ImVec2( 0.0f, 0.0f ));

            // var assetIdHash = tmp_selectedAssetIdHash;
            var assetIdHash = UserInterface.SelectedAsset;

            if(m_assetIdHash != assetIdHash) {
                m_assetIdHash = assetIdHash;
                m_LoadAsset();
            }
            if (HasLoadedAsset) {
                bool isOpen = true;

                var isDirty = Assets.IsDirty(m_iasset.assetIdHash);
                var flags = !isDirty ? 0 : (int)ImGuiWindowFlags_._UnsavedDocument;

                if (Dll.ImGui.Begin($"Asset", ref isOpen, flags)) {
                    m_DrawWindow();

                    if(isDirty && ImGui.IsWindowFocused() && Input.GetButtonDownEd(Key.S, Key.Ctrl))
                        Assets.Save(assetIdHash);
                }
                ImGui.End();

                if(!isOpen)
                    UserInterface.SelectedAsset = 0;
            }
            ImGui.PopStyleVar(4);
        }

        private void m_LoadAsset() {
            m_asset = null;
            m_iasset = null;

            if (m_assetIdHash == 0)
                return;

            var asset = AssetStore.GetAsset(m_assetIdHash);
            if (asset == null)
                return;

            if (asset.scriptType == typeof(Scene) || asset.scriptType == typeof(Actor))
                return;

            m_asset = asset;
            m_iasset = Assets.CreateAssetWrapper(m_assetIdHash, CppRef.NullRef);
            m_iasset.LoadAsset();
        }

        private void m_DrawWindow() {
            GUI.style = Marshal.PtrToStructure<UI.ImGuiStyle>(ImGui.GetStyle());
            GUI.rectWidth = Dll.ImGui.GetContentRegionAvail().X;
            GUI.groupId = m_iasset == null ? 0 : m_iasset.cppRef.value;
            GUI.subGroupId = 0;
            GUI.groupAssetIdHash = m_iasset == null ? 0 : m_iasset.assetIdHash;

            ImGui.PushStyleVar((int)ImGuiStyleVar_._FrameRounding, 3.0f);
            ImGui.PushStyleVar((int)ImGuiStyleVar_._ItemSpacing, new ImVec2(3.0f, 0.0f));
            ImGui.Indent(10);

            m_DrawAsset();

            ImGui.Unindent(10);
            ImGui.PopStyleVar(2);
        }

        private void m_DrawAsset() {
            m_DrawAssetHeader();

            object assetObj = m_iasset;
            UserInterface.Instance.DrawObject(m_iasset.GetType(), ref assetObj);

            var uiDrawer = m_iasset as IEditorUIDrawer;
            if (uiDrawer != null)
                uiDrawer.OnDrawUI();
        }

        private void m_DrawAssetHeader() {
            var typeName = m_asset.scriptType.Name;
            var path = Path.GetRelativePath(AssetStore.Instance.AssetsPath, m_asset.path);
            
            GUI.HeaderSpace();

            int assetIdHash = m_assetIdHash;
            bool changed = Dll.UI_Inspector.ShowAsset(Game.gameRef, "Asset", -1, ref assetIdHash, GUI.active);
            GUI.Space();

            ImGui.PushStyleColor((int)ImGuiCol_._Text, new sn.Vector4(1.0f, 1.0f, 1.0f, 0.3f));

            GUI.DrawText("Type", typeName);
            GUI.Space();
            GUI.DrawTextWrapped("Path", path);

            ImGui.PopStyleColor(1);

            GUI.HeaderSpace();
            ImGui.Separator();
            GUI.HeaderSpace();

            if (changed)
                UserInterface.SelectedAsset = assetIdHash;
                // tmp_selectedAssetIdHash = assetIdHash;
        }

    }
}
