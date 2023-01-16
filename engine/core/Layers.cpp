#include "Layers.h"

namespace Layers {

	JPH::uint8 GetLayerFromMotionType(JPH::EMotionType motionType) {
		switch (motionType) {
		case JPH::EMotionType::Static:
			return Layers::NON_MOVING;
		case JPH::EMotionType::Kinematic:
			return Layers::MOVING;
		case JPH::EMotionType::Dynamic:
			return Layers::MOVING;
		}
	}

}