
using Engine;

namespace Scripts {

    [GUID("b1aa2283-6a0f-44f8-a2de-2abefa76175f")]
    public class FirstComponent : CSComponent {

        /// public поля видимы в Inspector-е
        /// private поля невидимы в Inspector-е

        public float publicFloat = 1;
        private float privateFloat = 2;


        /// [Open] / [Close] делают поле видимым/невидимим 
        /// в Inspector-е независимо от уровня доступности
        
        [Open] private float privateOpenFloat = 3;
        [Close] public float publicCloseFloat = 4;


        /// [Range(float, float)] и [Range(int, int)]
        /// Отображают поле как slider
        [Range(0f, 10f)] public float floatField = 5;

        [Range(0, 10)] public int intField = 6;


        /// Actor и наследники Component сериализуются как ссылки
        /// если они установлены в редакторе, то после загрузки 
        /// сцены ссылки сохраняются
        
        public Component component = null;
        public MeshComponent meshComponent = null;
        public Rigidbody rigidbody = null;
        public Actor actorField = null;


        // Вызывается сразу после создания Actor-а
        public override void OnInit() {
            /// Если устанавливаем ссылки в Inspector-е 
            /// МОЖНО: проверить ссылки на Null
            /// НЕЛЬЗЯ: использовать объекты по ссылкам, так как
            ///         нет гарантий, что сами объекты сейчас 
            ///         инициализированы

            if(actorField == null)
                // После исключения компонент отключается до перезапуска сцены
                throw new NullFieldException(this, "actorField");
        }

        // Вызывается сразу после OnInit и после активации Actor-а
        public override void OnActivate() {
           
        }

        // Вызывается на следующий кадр после создания Actor-а
        public override void OnStart() {
            /// Если устанавливаем ссылки в Inspector-е 
            /// то сейчас они гарантировано инициализированы
            /// поэтому их можно использовать
        }

        // Вызывается на следующий кадр после OnStart и повторяется
        public override void OnUpdate() {
            float dt = Game.DeltaTime;
        }

        /// Вызываются во время FixedUpdate -->
        /// 
        /// Нельзя использовать 
        // - Создание / Удаление  Actor / Rigidbody / Character
        //   если нужно удалять - удаляем в OnUpdate
        // - rigidbody.simulate = false;

        /// Можно вызвать только один раз каждую функцию 
        /// если используются больше одного раза, 
        /// учитывается только последнее значение
        // rigidbody.AddForce();
        // rigidbody.SetLinearVelocityClamped();
        // и т.п.

        // Вызывается на следующий кадр после OnStart и повторяется в отдельном цикле
        public override void OnFixedUpdate() {
            float dt = Game.DeltaFixedTime;
        }


        // Вызывается, когда два коллайдера столкнулись
        public override void OnCollisionEnter(Actor otherActor, in Contact contact) { }

        // Вызывается, когда два коллайдера перестали сталкиваться
        public override void OnCollisionExit(Actor otherActor) { }

        // Вызывается, когда коллайдер вошел в триггерр
        public override void OnTriggerEnter(Actor otherActor, in Contact contact) { }

        // Вызывается, когда коллайдер вышел из триггерра
        public override void OnTriggerExit(Actor otherActor) { }

        /// 
        /// <-- FixedUpdate

        // Вызывается перед разрушением Actor-а и при деактивации Actor-а
        public override void OnDeactivate() {

        }

        // Вызывается при разрушении Actor-а
        public override void OnDestroy() {
            /// Если мы хотим разрушить другие Component-ы или Actor-ы 
            /// в OnDestroy, нужно проверить не разрушены ли они раньше,
            /// даже если мы создавали их сами

            if(!actorField.IsDestroyed)
                actorField.Destroy();
        }

    }
}