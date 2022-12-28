using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;
using YamlDotNet.RepresentationModel;
using System.Numerics;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Globalization;
using System;
using System.Diagnostics;
using FireYaml;
using EngineDll;

namespace Engine {
    public class CloseAttribute : Attribute { }

    public class OpenAttribute : Attribute { }

}

namespace Engine {

    public class SerializerBase {

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

        public virtual void OnSerialize(FireYaml.Serializer serializer, string selfPath, Type type, object instance) { }

        public virtual void OnDeserialize(FireYaml.Deserializer deserializer, string selfPath, Type type, ref object instance) { }

    }

    public class ActorSerializer : SerializerBase {

        private static LinkedList<WaitedComponent> m_waitedComponents = new LinkedList<WaitedComponent>();

        class WaitedComponent {
            public Actor actor;
            public Component component;
            public LinkedListNode<WaitedComponent> node;

            public void OnEndLoad() {
                Dll.Actor.InitComponent(actor.cppRef, component.cppRef);
                m_waitedComponents.Remove(node);
            }
        }

        public override void OnSerialize(FireYaml.Serializer serializer, string selfPath, Type type, object instance) {
            base.OnSerialize(serializer, selfPath, type, instance);

            if (type != typeof(Engine.Actor))
                return;

            var actor = instance as Engine.Actor;
            if (actor == null)
                return;

            var components = actor.GetComponentsList();
            var children = actor.GetChildren();

            serializer.AddField($"{selfPath}.m_components", components.GetType(), components);
            serializer.AddField($"{selfPath}.m_children", children.GetType(), children);

            foreach (var child in children)
                serializer.CreateDocument(child.GetType(), child);
            
            foreach (var component in components) 
                serializer.CreateDocument(component.GetType(), component);
        }

        public override void OnDeserialize(FireYaml.Deserializer deserializer, string selfPath, Type type, ref object instance) {
            base.OnDeserialize(deserializer, selfPath, type, ref instance);

            if (type != typeof(Engine.Actor))
                return;

            var actor = instance as Engine.Actor;
            if (actor == null)
                return;

            var componentsPath = $"{selfPath}.m_components";
            var childrenPath = $"{selfPath}.m_children";

            m_LoadComponents(deserializer, componentsPath, actor);
            m_LoadActorChildren(deserializer, childrenPath, actor);
        }

        private void m_LoadActorChildren(FireYaml.Deserializer deserializer, string childrenPath, Engine.Actor actor) {
            var children = deserializer.GetField(childrenPath);

            var count = children.GetItemsCount(childrenPath);
            for (int i = 0; i < count; i++) {
                var yamlChild = children.GetValue($"{childrenPath}.{i}");
                var fullPath = $".{yamlChild.value}";

                if (!deserializer.HasFile(fullPath))
                    throw new Exception("Missing actor");

                if (yamlChild.type == YamlValue.Type.Null)
                    throw new Exception("Actor component can not be Null");

                var child = deserializer.LoadDocument(fullPath) as Engine.Actor;
                child.parent = actor;
            }
        }

        private void m_LoadComponents(FireYaml.Deserializer deserializer, string componentsPath, Engine.Actor actor) {
            var components = deserializer.GetField(componentsPath);

            var count = components.GetItemsCount(componentsPath);
            for (int i = 0; i < count; i++) {
                var yamlChild = components.GetValue($"{componentsPath}.{i}");
                var fullPath = $".{yamlChild.value}";

                if (!deserializer.HasFile(fullPath))
                    throw new Exception("Missing component");

                var componentType = deserializer.GetTypeOf(fullPath);

                /// Create
                var componentObj = Activator.CreateInstance(componentType);
                var component = componentObj as Component;
                var info = component.CppConstructor();

                /// Bind
                component.CsBindComponent(actor.csRef, info);
                Dll.Actor.BindComponent(actor.cppRef, component.cppRef);

                /// PostBind
                deserializer.LoadDocument(fullPath, ref componentObj);

                /// Init (Delayed Init - wait for the end of load all documents)
                var waited = new WaitedComponent();
                waited.actor = actor;
                waited.component = component;
                waited.node = m_waitedComponents.AddLast(waited);

                deserializer.EndLoadEvent += waited.OnEndLoad;
            }
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

        public override void OnSerialize(FireYaml.Serializer serializer, string selfPath, Type type, object instance) {
            base.OnSerialize(serializer, selfPath, type, instance);

            var meshComponent = instance as Engine.MeshComponent;
            if (meshComponent == null)
                return;

            var mesh = meshComponent.mesh;
            var materials = m_GetMaterialsList(meshComponent);
            var staticMeshType = typeof(Engine.StaticMesh);
            var needSave = mesh != null && mesh.GetType() == staticMeshType;

            serializer.AddField($"{selfPath}.m_mesh", staticMeshType, needSave ? mesh : null);
            serializer.AddField($"{selfPath}.m_materials", materials.GetType(), materials);
        }

        public override void OnDeserialize(FireYaml.Deserializer deserializer, string selfPath, Type type, ref object instance) {
            base.OnDeserialize(deserializer, selfPath, type, ref instance);

            var meshComponent = instance as Engine.MeshComponent;
            if (meshComponent == null)
                return;

            var meshPath = $"{selfPath}.m_mesh";
            var materialsPath = $"{selfPath}.m_materials";
            
            m_LoadMesh(deserializer, meshPath, meshComponent);
            m_LoadMaterials(deserializer, materialsPath, meshComponent);

        }

        private List<StaticMaterial> m_GetMaterialsList(MeshComponent meshComponent) {
            var list = new List<StaticMaterial>();

            for (int index = 0; index < meshComponent.MaterialCount; index++) {
                var material = meshComponent.GetMaterial((ulong)index);
                list.Add(material.IsDynamic ? null : material);
            }
            return list;
        }

        private void m_LoadMesh(FireYaml.Deserializer deserializer, string meshPath, MeshComponent meshComponent) {
            
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

            if (!AssetStore.Instance.HasAssetPath(assetId))
                    throw new Exception($"Missing AssetId: {assetId}");

            if (yamlValue.type == YamlValue.Type.AssetId) {
                var mesh = new StaticMesh().LoadFromAsset(assetId);
                Dll.MeshComponent.SetPreInitMesh(meshComponent.cppRef, mesh.cppRef);
                return;
            }
        }

        private void m_LoadMaterials(FireYaml.Deserializer deserializer, string materialsPath, MeshComponent meshComponent) {
            var yamlObject = deserializer.GetField(materialsPath);

            var count = yamlObject.GetItemsCount(materialsPath);
            if (count == 0)
                return;

            var matRefs = new List<ulong>();
            var defaultMaterial = new StaticMaterial().LoadFromAsset(AssetStore.M_Default);

            for (int i = 0; i < count; i++) {
                var valuePath = $"{materialsPath}.{i}";

                /// Если есть, значит равен null
                if(!yamlObject.HasValue(valuePath)) {
                    var yamlValue = yamlObject.GetValue($"{valuePath}.assetId");
                    
                    if (yamlValue.type == YamlValue.Type.AssetId) {
                        var assetId = yamlValue.value;

                        if (!AssetStore.Instance.HasAssetPath(assetId))
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
    }

}
