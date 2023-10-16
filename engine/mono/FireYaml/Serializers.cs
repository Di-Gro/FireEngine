using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;
using YamlDotNet.RepresentationModel;
using System.Numerics;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Globalization;
using System;
using System.Linq;
using System.Diagnostics;

using FireYaml;
using EngineDll;
using UI;
using FireBin;

namespace Engine {
    public class CloseAttribute : Attribute { }

    public class OpenAttribute : Attribute { }

    public class ColorAttribute : Attribute { }
    
    public class SpaceAttribute : Attribute { }

    public class ReadOnlyAttribute : Attribute { }

    public class RangeAttribute : Attribute {
        public int imin, imax;
        public float fmin, fmax;

        public RangeAttribute(int min, int max) {
            imin = min;
            imax = max;
        }

        public RangeAttribute(float min, float max) {
            fmin = min;
            fmax = max;
        }
    }

}

namespace Engine {

    public class SerializerBase {
        private static readonly List<FireBin.Pointer?> s_emptyPointers = new List<FireBin.Pointer?>();

        public virtual bool NeedIncludeBase(Type type) => false;

        public virtual bool NeedSerialize(FieldInfo field) {
            if (field.IsPublic) {
                var closeAttribute = field.GetCustomAttribute<Engine.CloseAttribute>();
                if (closeAttribute != null)
                    return false;
            }
            if (field.IsPrivate) {
                var openAttribute = field.GetCustomAttribute<Engine.OpenAttribute>();
                if (openAttribute == null)
                    return false;
            }
            return true;
        }

        public virtual bool NeedSerialize(PropertyInfo prop) {
            bool canReadWrite = prop.CanRead && prop.CanWrite;
            if (!canReadWrite)
                return false;

            bool isPrivate = prop.GetGetMethod(true).IsPrivate || prop.GetSetMethod(true).IsPrivate;
            bool isPublic = !isPrivate;

            if (isPublic) {
                var closeAttribute = prop.GetCustomAttribute<Engine.CloseAttribute>();
                if (closeAttribute != null)
                    return false;
            }
            if (isPrivate) {
                var openAttribute = prop.GetCustomAttribute<Engine.OpenAttribute>();
                if (openAttribute == null)
                    return false;
            }
            return true;
        }

        public virtual void OnSerialize(FireYaml.FireWriter serializer, string selfPath, Type type, object instance) { }

        public virtual void OnDeserialize(FireYaml.FireReader deserializer, string selfPath, Type type, ref object instance) { }

        public virtual void OnDrawGui(Type type, ref object instance) { }

        public virtual List<string> GetNamesOfExtraFields() { return null; }
        public virtual List<FireBin.Pointer?> WriteExtraFields(FireBin.Serializer writer, Type type, object instance) { return s_emptyPointers; }
        public virtual void ReadExtraFields(FireBin.Deserializer reader, object instance, PtrList list) { }
    }

    public class ActorSerializer : SerializerBase {

        private static LinkedList<WaitedComponent> m_waitedComponents = new LinkedList<WaitedComponent>();

        class WaitedComponent {
            public Actor actor;
            public Component component;
            public LinkedListNode<WaitedComponent> node;


            public void OnEndLoad() {
#if DETACHED
#else
                Dll.Actor.InitComponent(actor.cppRef, component.cppRef);
#endif
                m_waitedComponents.Remove(node);
            }
        }

        private static readonly List<string> s_extraFields = new List<string>() {
            "m_flags",
            "m_children",
            "m_components",
        };

        public override List<string> GetNamesOfExtraFields() => s_extraFields;

        public override List<FireBin.Pointer?> WriteExtraFields(FireBin.Serializer writer, Type type, object instance) {
            var res = new List<FireBin.Pointer?>();

            var actor = instance as Engine.Actor;
            var children = actor.GetChildren();
            var allComponents = actor.GetComponentsList();
            var components = new List<Component>();

            foreach (var component in allComponents) {
                if (!component.runtimeOnly)
                    components.Add(component);
            }
            res.Add(writer.AddAsScalar(typeof(ulong), (ulong)actor.Flags));
            res.Add(writer.AddAsList(children.GetType(), children));
            res.Add(writer.AddAsList(components.GetType(), components));

            foreach (var child in children)
                writer.AddAsNamedList(child.GetType(), child);

            foreach (var component in components)
                writer.AddAsNamedList(component.GetType(), component);

            return res;
        }

        public override void ReadExtraFields(FireBin.Deserializer des, object instance, FireBin.PtrList list) {
            var actor = instance as Engine.Actor;

            var flagsPtr = list[0].Value;
            var childrenPtr = list[1].Value;
            var componentsPtr = list[2].Value;

            var flags = (Flag)des.Reader.ReadScalar<ulong>(flagsPtr);
            var children = des.Reader.ReadList(childrenPtr);
            var components = des.Reader.ReadList(componentsPtr);

            actor.Flags = flags;

            for (int i = 0; i < children.Count; i++) {
                var childRef = des.Reader.ReadReference(children[i].Value);
                object child = new Actor(actor);
                des.LoadAsNamedList(typeof(Actor), childRef.to, child);
            }
            for (int i = 0; i < components.Count; i++) {
                var compRef = des.Reader.ReadReference(components[i].Value);
                LoadComponent(des, actor, compRef.to);
            }
        }

        public Component LoadComponent(FireBin.Deserializer des, Engine.Actor actor, FireBin.Pointer compPtr) {

            var scriptId = des.Reader.ReadScriptId(compPtr);
            var componentType = FireBin.Deserializer.GetTypeOf(scriptId);

            /// Create
            var componentObj = Activator.CreateInstance(componentType);
            var component = componentObj as Component;

#if DETACHED
            actor.detached_components.Add(component);
#else
            var info = component.CppConstructor();

            /// Bind
            component.CsBindComponent(actor.csRef, info);
            Dll.Actor.BindComponent(actor.cppRef, component.cppRef);
#endif
            /// PostBind
            des.LoadAsNamedList(componentType, compPtr, componentObj);

            /// Init (Delayed Init - wait for the end of load all documents)
            var waited = new WaitedComponent();
            waited.actor = actor;
            waited.component = component;
            waited.node = m_waitedComponents.AddLast(waited);

            des.EndLoadEvent += waited.OnEndLoad;

            return component;
        }

        public override void OnSerialize(FireYaml.FireWriter serializer, string selfPath, Type type, object instance) {
            base.OnSerialize(serializer, selfPath, type, instance);

            if (type != typeof(Engine.Actor))
                return;

            var actor = instance as Engine.Actor;
            if (actor == null)
                return;

            var children = actor.GetChildren();
            var components = actor.GetComponentsList();

            serializer.AddField($"{selfPath}.m_children", children.GetType(), children);
            serializer.AddField($"{selfPath}.m_flags", typeof(ulong), (ulong)actor.Flags);

            m_AddComponents(serializer, selfPath, components);

            foreach (var child in children)
                serializer.CreateDocument(child.GetType(), child);

            foreach (var component in components) {
                if (!component.runtimeOnly)
                    serializer.CreateDocument(component.GetType(), component);
            }
        }

        private void m_AddComponents(FireYaml.FireWriter serializer, string selfPath, List<Component> components) {
            if (components.Count == 0) {
                serializer.AddField($"{selfPath}.m_components", components.GetType(), components);
                return;
            }
            int index = 0;
            foreach (var component in components) {
                if (!component.runtimeOnly)
                    serializer.AddField($"{selfPath}.m_components.{index++}", component.GetType(), component);
            }
        }

        public override void OnDeserialize(FireYaml.FireReader reader, string selfPath, Type type, ref object instance) {
            base.OnDeserialize(reader, selfPath, type, ref instance);

            if (type != typeof(Engine.Actor))
                return;

            var actor = instance as Engine.Actor;
            if (actor == null)
                return;

            var componentsPath = $"{selfPath}.m_components";
            var childrenPath = $"{selfPath}.m_children";
            var flagsPath = $"{selfPath}.m_flags";

            m_LoadComponents(reader, componentsPath, actor);
            m_LoadActorChildren(reader, childrenPath, actor);

            var flagsStr = reader.GetField(flagsPath);
            var str = flagsStr.GetValue(flagsPath, "0");
            var flags = ulong.Parse(str);
            actor.Flags = (Flag)flags;
        }

        private void m_LoadActorChildren(FireYaml.FireReader reader, string childrenPath, Engine.Actor actor) {
            var children = reader.GetField(childrenPath);

            var count = children.GetItemsCount(childrenPath);
            for (int i = 0; i < count; i++) {
                var yamlChild = children.GetValue($"{childrenPath}.{i}");
                var fullPath = $".{yamlChild.value}";

                if (!reader.HasFile(fullPath))
                    throw new Exception("Missing actor");

                if (yamlChild.type == YamlValue.Type.Null)
                    throw new Exception("Actor component can not be Null");

                object child = new Actor(actor);
                reader.LoadDocument(fullPath, ref child);
            }
        }

        private void m_LoadComponents(FireYaml.FireReader reader, string componentsPath, Engine.Actor actor) {
            var components = reader.GetField(componentsPath);

            var count = components.GetItemsCount(componentsPath);
            for (int i = 0; i < count; i++) {
                var yamlChild = components.GetValue($"{componentsPath}.{i}");
                var filePath = $".{yamlChild.value}";

                if (!reader.HasFile(filePath))
                    throw new Exception("Missing component");

                LoadComponent(reader, filePath, actor);
            }
        }

        public Component LoadComponent(FireYaml.FireReader reader, string filePath, Engine.Actor actor) {
            var componentType = reader.GetTypeOf(filePath);

            /// Create
            var componentObj = Activator.CreateInstance(componentType);
            var component = componentObj as Component;
            var info = component.CppConstructor();

            /// Bind
            component.CsBindComponent(actor.csRef, info);
            Dll.Actor.BindComponent(actor.cppRef, component.cppRef);

            /// PostBind
            reader.LoadDocument(filePath, ref componentObj);

            /// Init (Delayed Init - wait for the end of load all documents)
            var waited = new WaitedComponent();
            waited.actor = actor;
            waited.component = component;
            waited.node = m_waitedComponents.AddLast(waited);

            reader.EndLoadEvent += waited.OnEndLoad;

            return component;
        }

    }

    public class ComponentSerializer : SerializerBase {

        public override bool NeedIncludeBase(Type type) {
            bool isCsBased = type.BaseType == typeof(Engine.CSComponent);
            bool isCppBased = type.BaseType == typeof(Engine.CppComponent);

            return !isCsBased && !isCppBased;
        }

    }

    public class MeshComponentSerializer : ComponentSerializer {

        public override void OnSerialize(FireYaml.FireWriter serializer, string selfPath, Type type, object instance) {
            base.OnSerialize(serializer, selfPath, type, instance);

            var meshComponent = instance as Engine.MeshComponent;
            if (meshComponent == null)
                return;

            var mesh = meshComponent.mesh;
            var materials = m_GetMaterialsList(meshComponent);
            var staticMeshType = typeof(Engine.Mesh);
            var needSave = mesh != null && mesh.GetType() == staticMeshType;

            serializer.AddField($"{selfPath}.m_mesh", staticMeshType, needSave ? mesh : null);
            serializer.AddField($"{selfPath}.m_materials", materials.GetType(), materials);
        }

        private static readonly List<string> s_extraFields = new List<string>() {
            "m_mesh",
            "m_materials",
        };

        public override List<string> GetNamesOfExtraFields() => s_extraFields;

        public override List<FireBin.Pointer?> WriteExtraFields(FireBin.Serializer writer, Type type, object instance) {
            var res = new List<FireBin.Pointer?>();

            var meshComponent = instance as Engine.MeshComponent;
            var mesh = meshComponent.mesh;
            var materials = m_GetMaterialsList(meshComponent);
            var staticMeshType = typeof(Engine.Mesh);
            var needSave = mesh != null && mesh.GetType() == staticMeshType;

            res.Add(writer.AddAsAssetRef(staticMeshType, needSave ? mesh : null));
            res.Add(writer.AddAsList(materials.GetType(), materials));

            return res;
        }

        public override void ReadExtraFields(FireBin.Deserializer des, object instance, FireBin.PtrList list) {
            var meshComponent = instance as MeshComponent;

            var meshPtr = list[0].Value;
            var materialsPtr = list[1].Value;

            m_LoadMesh(des, meshPtr, meshComponent);
            m_LoadMaterials(des, materialsPtr, meshComponent);

            Dll.MeshComponent.OnPreInit(meshComponent.cppRef);
        }

        public override void OnDeserialize(FireYaml.FireReader deserializer, string selfPath, Type type, ref object instance) {
            base.OnDeserialize(deserializer, selfPath, type, ref instance);

            var meshComponent = instance as Engine.MeshComponent;
            if (meshComponent == null)
                return;

            var meshPath = $"{selfPath}.m_mesh";
            var materialsPath = $"{selfPath}.m_materials";
            
            m_LoadMesh(deserializer, meshPath, meshComponent);
            m_LoadMaterials(deserializer, materialsPath, meshComponent);

            Dll.MeshComponent.OnPreInit(meshComponent.cppRef);
        }

        private List<StaticMaterial> m_GetMaterialsList(MeshComponent meshComponent) {
            var list = new List<StaticMaterial>();

            for (int index = 0; index < meshComponent.MaterialCount; index++) {
                var material = meshComponent.GetMaterial((ulong)index);
                list.Add(material.IsDynamic ? null : material);
            }
            return list;
        }

        private void m_LoadMesh(FireYaml.FireReader deserializer, string meshPath, MeshComponent meshComponent) {
            
            var meshAssetPath = $"{meshPath}.assetId";
            var meshField = deserializer.GetField(meshPath);
        
            /// Если есть сам меш, значит равен нулю
            if(meshField.HasValue(meshPath))
                return;
                
            /// Если нет, значит меш не установлен
            if(!meshField.HasValue(meshAssetPath))
                return;
                
            var yamlValue = meshField.GetValue(meshAssetPath);
            var assetId = yamlValue.value;
            var assetIdHash = assetId.GetAssetIDHash();
            var isPath = m_IsPath(assetId);

            if (!isPath && !AssetStore.HasAssetPath(assetIdHash))
                    throw new Exception($"Missing AssetId: {assetId}");

            if (yamlValue.type == YamlValue.Type.AssetId) {
                Mesh mesh = null;
                if (isPath)
                    mesh = new Mesh().LoadFromFile(assetId);
                else
                    mesh = new Mesh().LoadFromAsset(assetId);

                Dll.MeshComponent.SetPreInitMesh(meshComponent.cppRef, mesh.cppRef);
                return;
            }
        }

        private void m_LoadMesh(FireBin.Deserializer des, FireBin.Pointer meshPtr, MeshComponent meshComponent) {
            if (meshPtr.offset == FireBin.Pointer.NullOffset)
                return;

            var meshObj = des.LoadAsAssetRef(typeof(Mesh), meshPtr);
            var mesh = meshObj as Mesh;

            var isPath = m_IsPath(mesh.assetId);
            var assetIdHash = mesh.assetId.GetAssetIDHash();

            if (!isPath && !AssetStore.HasAssetPath(assetIdHash))
                throw new Exception($"Missing AssetId: {mesh.assetId}");

            if (isPath)
                mesh = new Mesh().LoadFromFile(mesh.assetId);
            else
                mesh = new Mesh().LoadFromAsset(mesh.assetId);

            Dll.MeshComponent.SetPreInitMesh(meshComponent.cppRef, mesh.cppRef);
        }

        private bool m_IsPath(string value) {
            return value.Contains('/') || value.Contains('\\') || value.Contains('.');
        }

        private void m_LoadMaterials(FireYaml.FireReader deserializer, string materialsPath, MeshComponent meshComponent) {
            var yamlObject = deserializer.GetField(materialsPath);

            var count = yamlObject.GetItemsCount(materialsPath);
            if (count == 0)
                return;

            var matRefs = new List<ulong>();
            var defaultMaterial = new StaticMaterial().LoadFromAsset(Assets.M_Default);

            for (int i = 0; i < count; i++) {
                var valuePath = $"{materialsPath}.{i}";

                /// Если есть, значит равен null
                if(!yamlObject.HasValue(valuePath)) {
                    var yamlValue = yamlObject.GetValue($"{valuePath}.assetId");
                    
                    if (yamlValue.type == YamlValue.Type.AssetId) {
                        var assetId = yamlValue.value;
                        var assetIdHash = assetId.GetAssetIDHash();

                        if (!AssetStore.HasAssetPath(assetIdHash))
                            throw new Exception($"Missing AssetId: {assetId}");

                        var material = new StaticMaterial().LoadFromAsset(assetId);
                        matRefs.Add(material.cppRef.value);
                        continue;
                    }
                }
                /// Null и другое
                matRefs.Add(defaultMaterial.cppRef.value);
            }
            Dll.MeshComponent.SetPreInitMaterials(meshComponent.cppRef, matRefs.ToArray(), matRefs.Count);
        }

        private void m_LoadMaterials(FireBin.Deserializer des, FireBin.Pointer materialsPtr, MeshComponent meshComponent) {
            var materials = new List<StaticMaterial>();

            des.LoadAsList(materials.GetType(), materialsPtr, materials);
            if (materials.Count == 0)
                return;

            var matRefs = new List<ulong>();
            var defaultMatCppRef = new StaticMaterial().LoadFromAsset(Assets.M_Default).cppRef.value;

            for (int i = 0; i < materials.Count; i++) {
                var material = materials[i];
                if (material == null) {
                    matRefs.Add(defaultMatCppRef);
                    continue;
                }
                //var assetIdHash = material.assetId.GetAssetIDHash();
                //if (!AssetStore.HasAsset(assetIdHash))
                //    throw new Exception($"Missing AssetId: {material.assetId}");

                material.LoadFromAsset(material.assetId);
                matRefs.Add(material.cppRef.value);
            }
            Dll.MeshComponent.SetPreInitMaterials(meshComponent.cppRef, matRefs.ToArray(), matRefs.Count);
        }

        public override void OnDrawGui(Type type, ref object instance) {
            base.OnDrawGui(type, ref instance);

            var meshComponent = instance as Engine.MeshComponent;
            if (meshComponent == null)
                return;

            var mesh = meshComponent.mesh;
            int assetIdHash = 0;

            if(mesh != null)
                assetIdHash = mesh.assetIdHash;

            GUI.Space();

            object changedMesh;
            if (GUI.DrawAsset("Mesh", typeof(Mesh), assetIdHash, out changedMesh))
                meshComponent.mesh = changedMesh as Mesh;
            
            var flags = ImGuiTreeNodeFlags_._Framed | ImGuiTreeNodeFlags_._DefaultOpen;
            if(GUI.CollapsingHeader("Materials", flags, out var size)) {

                var count = (ulong) meshComponent.MaterialCount;
                for (ulong index = 0; index < count; index++) {
                    var material = meshComponent.GetMaterial(index);

                    object changedMaterial;
                    if (GUI.DrawAsset($"Material: {index}", typeof(StaticMaterial), material.assetIdHash, out changedMaterial))
                        meshComponent.SetMaterial(index, changedMaterial as StaticMaterial);

                    if(index < count - 1)
                        GUI.Space();
                }
            }
        }

    }

}
