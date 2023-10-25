using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;
using FireYaml;

namespace Engine {

    static class Game {

        public static CppRef gameRef { get; private set; }
        public static CppRef assetStoreRef { get; private set; }
        public static CppRef uiRef { get; private set; }
        public static CppRef sceneRef { get; private set; }
        public static float DeltaTime => m_updateData.deltaTime;
        public static float DeltaFixedTime => m_updateData.deltaFixedTime;

        public static CameraComponent MainCamera => (CameraComponent)CppLinked.GetObjectByRef(Dll.Game.mainCamera_get(gameRef));

        private static GameUpdateData m_updateData;
        private static GameCallbacks m_gameCallbacks;

        public static EditorSettings editorSettings;

        public static Scene StaticScene {
            get {
                var sceneRef = Dll.Game.StaticScene(gameRef);
                if (sceneRef == CppRef.NullRef)
                    return null;

                return new Scene(sceneRef);
            }
        }

        public static T InstanciateAsset<T>(int assetIdHash) where T : FireYaml.IFile, new() {
            return AssetStore.GetAssetDeserializer(assetIdHash).Instanciate<T>();
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
            m_gameCallbacks.setSceneRef = new GameCallbacks.CppRef_void(cpp_SetSceneRef);
            m_gameCallbacks.setAssetStoreRef = new GameCallbacks.CppRef_void(cpp_SetAssetStoreRef);
            m_gameCallbacks.setUpdateData = new GameCallbacks.SetUpdateData(cpp_SetUpdateData);
            m_gameCallbacks.onInputUpdate = new GameCallbacks.void_void(Input.OnUpdate);
            m_gameCallbacks.writeScene = new GameCallbacks.CppRef_hash_bool(cpp_WriteScene);
            m_gameCallbacks.loadScene = new GameCallbacks.CppRef_hash_bool(LoadScene);

            m_gameCallbacks.runOrCrush = new GameCallbacks.RunOrCrush(Component.RunOrCrush);
            m_gameCallbacks.runOrCrushContactEnter = new GameCallbacks.RunOrCrushContactEnter(Component.RunOrCrushContactEnter);
            m_gameCallbacks.runOrCrushContactExit = new GameCallbacks.RunOrCrushContactExit(Component.RunOrCrushContactExit);

            m_gameCallbacks.isAssignable = new GameCallbacks.CsRef_hash_bool(AssetStore.cpp_IsAssignable);
            m_gameCallbacks.removeCsRef = new GameCallbacks.CsRef_void(CppLinked.RemoveCsRef);
            m_gameCallbacks.loadAssetStore = new GameCallbacks.void_void(LoadAssets);
            m_gameCallbacks.hasAssetInStore = new GameCallbacks.hash_bool(AssetStore.HasAsset);
            m_gameCallbacks.getStringHash = new GameCallbacks.cstr_int(cpp_GetStringHash);
            m_gameCallbacks.loadAsset = new GameCallbacks.CppRef_hash_bool(Assets.cpp_Load);
            m_gameCallbacks.reloadAsset = new GameCallbacks.CppRef_hash_void(Assets.cpp_Reload);
            m_gameCallbacks.saveAsset = new GameCallbacks.CppRef_hash_void(Assets.cpp_Save);
            m_gameCallbacks.pushClipboard = new GameCallbacks.CsRef_void(Clipboard.Push);
            m_gameCallbacks.peekClipboard = new GameCallbacks.void_CppRef(Clipboard.Peek);
            m_gameCallbacks.clipboardIsAssignable = new GameCallbacks.hash_bool(Clipboard.IsAssignable);
            m_gameCallbacks.clipboardIsSameType = new GameCallbacks.hash_bool(Clipboard.IsSameType);
            m_gameCallbacks.clipboardSetActor = new GameCallbacks.CsRef_void(Clipboard.SetActor);

            m_gameCallbacks.createAsset = new GameCallbacks.CreateAsset(AssetStore.cpp_CreateAsset);
            m_gameCallbacks.renameAsset = new GameCallbacks.hash_cstr_bool(AssetStore.cpp_RenameAsset);
            m_gameCallbacks.removeAsset = new GameCallbacks.hash_void(AssetStore.cpp_RemoveAsset);

            m_gameCallbacks.createPrefab = new GameCallbacks.CreatePrefab(Prefab.CreatePrefab);
            m_gameCallbacks.loadPrefab = new GameCallbacks.CsRef_hash_bool(Prefab.LoadPrefab);
            m_gameCallbacks.updatePrefab = new GameCallbacks.CsRef_hash_bool(Prefab.UpdatePrefab);
            
            m_gameCallbacks.setPrefabId = new GameCallbacks.CsRef_hash_void(Actor.SetPrefabId);

            m_gameCallbacks.createSceneAsset = new GameCallbacks.cstr_int(Scene.cpp_CreateSceneAsset);
            m_gameCallbacks.renameSceneAsset = new GameCallbacks.hash_cstr_bool(Scene.cpp_RenameSceneAsset);

            m_gameCallbacks.requestAssetGuid = new GameCallbacks.hash_void(AssetStore.cpp_RequestAssetGuid);
            m_gameCallbacks.setStartupScene = new GameCallbacks.CppRef_void(cpp_SetStartupScene);

            m_gameCallbacks.setUserInterfaceRef = new GameCallbacks.CppRef_void(cpp_SetUserInterfaceRef);
            m_gameCallbacks.isRuntimeAsset = new GameCallbacks.hash_bool(AssetStore.IsRuntimeAsset);
            m_gameCallbacks.addRuntimeAsset = new GameCallbacks.hash_void(AssetStore.AddRuntimeAsset);

            m_gameCallbacks.createSound = new GameCallbacks.CppRef_CsRef(Sound.cpp_CreateSound);
            m_gameCallbacks.soundSetAsset = new GameCallbacks.CsRef_CppRef_void(Sound.cpp_SetAsset);
            m_gameCallbacks.emitterSetSound = new GameCallbacks.CsRef_CsRef_void(AudioEmitter.cpp_SetSound);

            m_gameCallbacks.initUserInterface = new GameCallbacks.void_void(UserInterface.cpp_Init);

            Dll.Game.SetGameCallbacks(Game.gameRef, m_gameCallbacks);
        }

        private static void LoadAssets() {
            AssetStore.Instance = new AssetStore();
            AssetStore.Instance.Init("../../project");

            editorSettings = new EditorSettings(Assets.editor_settings);
            editorSettings.LoadAsset();
        }

        private static void cpp_SetStartupScene(CppRef sceneRef) {
            var scene = new Scene(new Scene(sceneRef).assetId);

            editorSettings.StartupScene = scene;
            editorSettings.SaveAsset();
            editorSettings.SendToCpp();
            
            // var assetIdHash = Assets.editor_settings.GetAssetIDHash();
            // var filesCount = AssetStore.GetAssetFilesCount(assetIdHash);
            // var writer = new FireWriter(ignoreExistingIds: false, writeNewIds: true, startId: filesCount + 1);

            // AssetStore.WriteAsset(assetIdHash, editorSettings, writer);

        }

        private static int cpp_GetStringHash(ulong stringPtr){
            var str = Assets.ReadCString(stringPtr);
            return str.GetAssetIDHash();
        }

        private static void cpp_SetSceneRef(CppRef value) {
            sceneRef = value;
            // Console.WriteLine($"scene {sceneRef}");
        }
        //private static void SetMeshAssetRef(CppRef value) => meshAssetRef = value;
        private static void cpp_SetAssetStoreRef(CppRef value) => assetStoreRef = value;
        private static void cpp_SetUserInterfaceRef(CppRef value) => uiRef = value;
        private static void cpp_SetUpdateData(GameUpdateData value) => m_updateData = value;

#if DETACHED
        public static Stack<Scene> detached_scenes = new Stack<Scene>();

        /// <summary> Detached ready </summary>
        public static void PushScene(Scene scene) {
            detached_scenes.Push(scene);
        }

        /// <summary> Detached ready </summary>
        public static void PopScene() {
            detached_scenes.Pop();
        }

        /// <summary> Detached ready </summary>
        public static Scene GetScene() => detached_scenes.Peek();

#else

        public static void PushScene(Scene scene){
            Dll.Game.PushScene(gameRef, scene.cppRef);
        }

        public static void PushSelectedScene() {
            Dll.Game.PushSelectedScene(gameRef);
        }

        public static void PopScene() {
            Dll.Game.PopScene(gameRef);
        }

#endif

        /// TODO: Изменить. В принципе не нужно создавать ассет. 
        /// ----: Искользуется в C++ только для созранения EditorScene в папку Editor\Ignore.
        /// ----: Как следствие эта сцена игнорируется при загрузке ассетов. 
        /// ----: Первый вызов приводит к созданию ассета. 
        /// ----: Это явно особый ассет, не уверен, что его вообще нужно создавать. 
        /// ----: С другими сценами С++ работает как с обычными ассетами. 
        private static bool cpp_WriteScene(CppRef cppSceneRef, int assetIdHash) {
            try {
                object scene = new Scene(cppSceneRef);

                AssetStore.WriteAsset(assetIdHash, scene);

                return true;
            } catch (Exception e) {

                Console.WriteLine("Exception on WriteScene:");

                if (e.InnerException != null) {
                    Console.WriteLine(e.InnerException.Message);
                    Console.WriteLine(e.InnerException.StackTrace);
                }
                Console.WriteLine(e.Message);
                Console.WriteLine(e.StackTrace);
                return false;
            }
        }

        private static bool LoadScene(CppRef cppSceneRef, int assetIdHash) {
            try {
                var assetId = AssetStore.GetAssetGuid(assetIdHash);
                var scene = new Scene(cppSceneRef);

                scene.Init(assetId, cppSceneRef);
                scene.LoadAsset();

                return true;

            } catch (Exception e) {
                Console.WriteLine("Exception on LoadScene:");

                if (e.InnerException != null) {
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
        public float deltaFixedTime;
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct GameCallbacks {
        public delegate void SetUpdateData(GameUpdateData value);
        public delegate bool RunOrCrush(CsRef componentRef, ComponentCallbacks.ComponentCallback method);
        public delegate bool RunOrCrushContactEnter(CsRef componentRef, ComponentCallbacks.ContactEnter method, CsRef csRef, in Contact contact);
        public delegate bool RunOrCrushContactExit(CsRef componentRef, ComponentCallbacks.ContactExit method, CsRef csRef);
        public delegate bool CreateAsset(ulong pathPtr);
        public delegate int CreatePrefab(CsRef actorRef, ulong pathPtr);

        public delegate void    void_void();
        public delegate CppRef  void_CppRef();

        public delegate void hash_void(int hash);
        public delegate bool hash_bool(int hash);
        public delegate bool hash_cstr_bool(int hash, ulong cstr);

        public delegate void CppRef_void(CppRef value);
        public delegate bool CppRef_hash_bool(CppRef cppRef, int hash);
        public delegate void CppRef_hash_void(CppRef cppRef, int hash);
        public delegate CsRef CppRef_CsRef(CppRef cppRef);
        public delegate void CsRef_CppRef_void(CsRef csRef, CppRef cppRef);
        public delegate void CsRef_CsRef_void(CsRef csRef1, CsRef csRef2);

        public delegate void CsRef_void(CsRef value);
        public delegate bool CsRef_hash_bool(CsRef value, int hash);
        public delegate void CsRef_hash_void(CsRef value, int hash);

        public delegate int cstr_int(ulong cstr);


        public CppRef_void setSceneRef;
        public CppRef_void setAssetStoreRef;
        public SetUpdateData setUpdateData;
        public void_void onInputUpdate;
        public CppRef_hash_bool writeScene;
        public CppRef_hash_bool loadScene;
        public RunOrCrush runOrCrush;
        public RunOrCrushContactEnter runOrCrushContactEnter;
        public RunOrCrushContactExit runOrCrushContactExit;
        public CsRef_hash_bool isAssignable; // From, To
        public CsRef_void removeCsRef;
        public void_void loadAssetStore;
        public hash_bool hasAssetInStore;
        public cstr_int getStringHash;
        public CppRef_hash_bool loadAsset;
        public CppRef_hash_void reloadAsset;
        public CppRef_hash_void saveAsset;
        public CsRef_void pushClipboard;
        public void_CppRef peekClipboard;
        public hash_bool clipboardIsAssignable;
        public hash_bool clipboardIsSameType;
        public CsRef_void clipboardSetActor;
        public CreateAsset createAsset;
        public hash_cstr_bool renameAsset;
        public hash_void removeAsset;
        public CreatePrefab createPrefab;
        public CsRef_hash_bool loadPrefab;
        public CsRef_hash_bool updatePrefab;
        public CsRef_hash_void setPrefabId;
        public cstr_int createSceneAsset;
        public hash_cstr_bool renameSceneAsset;
        public hash_void requestAssetGuid;
        public CppRef_void setStartupScene;
        public CppRef_void setUserInterfaceRef;
        public hash_bool isRuntimeAsset;
        public hash_void addRuntimeAsset;
        public CppRef_CsRef createSound;
        public CsRef_CppRef_void soundSetAsset;
        public CsRef_CsRef_void emitterSetSound;
        public void_void initUserInterface;
    }
}


