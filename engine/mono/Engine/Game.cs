﻿using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;

namespace Engine {

    static class Game {

        public static CppRef gameRef { get; private set; }
        public static CppRef meshAssetRef { get; private set; }
        public static CppRef assetStoreRef { get; private set; }
        public static CppRef sceneRef { get; private set; }
        public static float DeltaTime => m_updateData.deltaTime;

        public static CameraComponent MainCamera => (CameraComponent)CppLinked.GetObjectByRef(Dll.Game.mainCamera_get(gameRef));

        private static GameUpdateData m_updateData;
        private static GameCallbacks m_gameCallbacks;

        public static EditorSettings editorSettings;

        public static T InstanciateAsset<T>(string assetId) where T : FireYaml.IFile, new() {
            return new FireYaml.FireReader(assetId).Instanciate<T>();
        }

        public static Scene CreateScene() {
            var cppRef = Dll.Game.CreateScene(gameRef, false);
            return new Scene(cppRef);
        }

        public static void DestroyScene(Scene scene){
            Dll.Game.DestroyScene(gameRef, scene.cppRef);
        }

        public static List<Actor> GetRootActors(CppRef cppSceneRef) {
            int count = Dll.Game.GetRootActorsCount(cppSceneRef);
            var refs = new CsRef[count];

            Dll.Game.WriteRootActorsRefs(cppSceneRef, refs);

            var result = new List<Actor>(count);
            foreach(var objRef in refs) {
                var actor = CppLinked.GetObjectByRef(objRef) as Actor;
                result.Add(actor);
            }
            return result;
        }

        public static T FindComponent<T>() where T : Component {
            var roots = GetRootActors(Game.sceneRef);
            foreach (var root in roots) {
                var component = root.GetComponent<T>();
                if(component != null)
                    return component;
            }
            return null;
        }

        public static void Exit() => Dll.Game.Exit(gameRef);     

        private static void cpp_Init(CppRef _gameRef) {
            gameRef = _gameRef;

            m_gameCallbacks = new GameCallbacks();
            m_gameCallbacks.setSceneRef = new GameCallbacks.TakeCppRef(SetSceneRef);
            m_gameCallbacks.setMeshAssetRef = new GameCallbacks.TakeCppRef(SetMeshAssetRef);
            m_gameCallbacks.setAssetStoreRef = new GameCallbacks.TakeCppRef(SetAssetStoreRef);
            m_gameCallbacks.setUpdateData = new GameCallbacks.SetUpdateData(SetUpdateData);
            m_gameCallbacks.onInputUpdate = new GameCallbacks.Void(Input.OnUpdate);
            m_gameCallbacks.saveScene = new GameCallbacks.SaveScene(SaveScene);
            m_gameCallbacks.loadScene = new GameCallbacks.LoadScene(LoadScene);
            m_gameCallbacks.runOrCrush = new GameCallbacks.RunOrCrush(Component.RunOrCrush);
            m_gameCallbacks.isAssignable = new GameCallbacks.IsAssignable(FireYaml.AssetStore.IsAssignable);
            m_gameCallbacks.removeCsRef = new GameCallbacks.TakeCsRef(CppLinked.RemoveCsRef);
            m_gameCallbacks.loadAssetStore = new GameCallbacks.Void(LoadAssets);
            m_gameCallbacks.hasAssetInStore = new GameCallbacks.HasAsset(FireYaml.AssetStore.HasAsset);
            m_gameCallbacks.getStringHash = new GameCallbacks.GetStringHash(m_GetStringHash);
            m_gameCallbacks.loadAsset = new GameCallbacks.LoadAsset(Assets.Load);
            m_gameCallbacks.reloadAsset = new GameCallbacks.ReloadAsset(Assets.Reload);
            m_gameCallbacks.saveAsset = new GameCallbacks.SaveAsset(Assets.Save);
            m_gameCallbacks.pushClipboard = new GameCallbacks.TakeCsRef(Clipboard.Push);
            m_gameCallbacks.peekClipboard = new GameCallbacks.ClipboardPeek(Clipboard.Peek);
            m_gameCallbacks.clipboardIsAssignable = new GameCallbacks.ClipboardIsAssignable(Clipboard.IsAssignable);
            m_gameCallbacks.clipboardIsSameType = new GameCallbacks.ClipboardIsAssignable(Clipboard.IsSameType);

            Dll.Game.SetGameCallbacks(Game.gameRef, m_gameCallbacks);
        }

        private static void LoadAssets() {
            FireYaml.AssetStore.UpdateTypesInCpp();
            FireYaml.AssetStore.UpdateAssetsInCpp();

            editorSettings = InstanciateAsset<EditorSettings>("editor_settings");
            editorSettings.UpdateInCpp();
        }

        private static int m_GetStringHash(ulong stringPtr){
            var str = Assets.ReadCString(stringPtr);
            return str.GetHashCode();
        }

        private static void SetSceneRef(CppRef value) => sceneRef = value;
        private static void SetMeshAssetRef(CppRef value) => meshAssetRef = value;
        private static void SetAssetStoreRef(CppRef value) => assetStoreRef = value;
        private static void SetUpdateData(GameUpdateData value) => m_updateData = value;

        public static void PushScene(Scene scene){
            Dll.Game.PushScene(gameRef, scene.cppRef);
        }

        public static void PopScene() {
            Dll.Game.PopScene(gameRef);
        }

        private static bool SaveScene(CppRef cppSceneRef, ulong assetIdPtr, ulong pathPtr) {
            var assetId = Assets.ReadCString(assetIdPtr);
            var path = Assets.ReadCString(pathPtr);

            try {
                object scene = new Scene(cppSceneRef);
                FireYaml.AssetStore.Instance.CreateAsset(path, scene, assetId);
                return true;

            } catch (Exception e) {

                Console.WriteLine("Exception on SaveScene:");

                if (e.InnerException != null) {
                    Console.WriteLine(e.InnerException.Message);
                    Console.WriteLine(e.InnerException.StackTrace);
                }
                Console.WriteLine(e.Message);
                Console.WriteLine(e.StackTrace);
                return false;
            }
        }

        private static bool LoadScene(CppRef cppSceneRef, ulong assetIdPtr) {
            var assetId = Assets.ReadCString(assetIdPtr);

            try {
                var scene = new Scene(cppSceneRef);
                object sceneObj = scene;
                FireYaml.FireReader.InitIAsset(ref sceneObj, assetId, cppSceneRef);

                scene.LoadAsset();
                return true;

            } catch (Exception e) {
                Console.WriteLine("Exception on LoadScene:");

                if(e.InnerException != null) {
                    Console.WriteLine(e.InnerException.Message);
                    Console.WriteLine(e.InnerException.StackTrace);
                }
                Console.WriteLine(e.Message);
                Console.WriteLine(e.StackTrace);
                return false;
            }
        }

    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GameUpdateData {
        public float deltaTime;
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct GameCallbacks {
        public delegate void Void();
        public delegate void TakeCppRef(CppRef value);
        public delegate void TakeCsRef(CsRef value);
        public delegate CppRef ClipboardPeek();
        public delegate bool ClipboardIsAssignable(int scriptIdHash);
        public delegate void SetUpdateData(GameUpdateData value);
        public delegate bool SaveScene(CppRef cppSceneRef, ulong assetIdPtr, ulong pathPtr);
        public delegate bool LoadScene(CppRef cppSceneRef, ulong assetIdPtr);
        public delegate bool RunOrCrush(CsRef componentRef, ComponentCallbacks.ComponentCallback method);
        public delegate bool IsAssignable(CsRef csRef, int typeIdHash);
        public delegate bool HasAsset(int typeIdHash);
        public delegate int GetStringHash(ulong stringPtr);
        public delegate bool LoadAsset(int assetIdHash, CppRef cppRef);
        public delegate void ReloadAsset(int assetIdHash);
        public delegate void SaveAsset(int assetIdHash);

        public TakeCppRef setSceneRef;
        public TakeCppRef setMeshAssetRef;
        public TakeCppRef setAssetStoreRef;
        public SetUpdateData setUpdateData;
        public Void onInputUpdate;

        public SaveScene saveScene;
        public LoadScene loadScene;

        public RunOrCrush runOrCrush;

        public IsAssignable isAssignable; // From, To
        public TakeCsRef removeCsRef;

        public Void loadAssetStore;
        public HasAsset hasAssetInStore;

        public GetStringHash getStringHash;

        public LoadAsset loadAsset;
        public ReloadAsset reloadAsset;
        public SaveAsset saveAsset;

        public TakeCsRef pushClipboard;
        public ClipboardPeek peekClipboard;
        public ClipboardIsAssignable clipboardIsAssignable;
        public ClipboardIsAssignable clipboardIsSameType;
    }
}


