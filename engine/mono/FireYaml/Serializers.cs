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

        public override void OnSerialize(FireYaml.Serializer serializer, string selfPath, Type type, object instance) {
            base.OnSerialize(serializer, selfPath, type, instance);

            if (type != typeof(Engine.Actor))
                return;

            var actor = instance as Engine.Actor;
            if (actor == null)
                return;

            var components = actor.GetComponentsList();
            var children = actor.GetChildren();

            Console.WriteLine($"#: ActorSerializer.OnSerialize()");

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

                var component = m_CreateComponent(deserializer, fullPath, actor);
                deserializer.LoadDocument(fullPath, ref component);
            }
        }

        private object m_CreateComponent(FireYaml.Deserializer deserializer, string fullPath, Engine.Actor actor) {

            var componentType = deserializer.GetTypeOf(fullPath);

            var addComponent = actor.GetType().GetMethod(nameof(Engine.Actor.AddComponent));
            if (addComponent == null)
                throw new Exception("Actor.AddComponent method not found");

            addComponent = addComponent.MakeGenericMethod(new Type[] { componentType });

            var component = addComponent.Invoke(actor, null);
            if (component == null)
                throw new Exception("Can not create component");

            return component;
        }
    }

    public class CSComponentSerializer : SerializerBase {

        public override bool NeedIncludeBase(Type type) {
            return type.BaseType != typeof(Engine.CSComponent);
        }

    }

}
