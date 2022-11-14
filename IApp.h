#pragma once


namespace DiGro {
	namespace GameFramework {
		class IApp {

		public:
			virtual void OnStart();
			virtual void OnStop();

			//virtual IWindow CreateWindow() = 0;
			//virtual IWindow CreateRenderDevice() = 0;
			//virtual IWindow CreateGame() = 0;

		};
	}
}
