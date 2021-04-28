#ifndef ANIMATION
#define ANIMATION

#include <list>
#include <memory>
#include <vector>

#include "scenegraph.h"
#include "sgutils.h"

using Keyframe = std::vector<RigTForm>;
static const int UNDEFINED = -3;

class KeyframeList {

public:
	//! Default constructor
	//! Creates an empty list of keyframe
	KeyframeList() = default;

	//! Default destructor
	~KeyframeList() = default;

	//! Add new keyframe to the list
	void addNewKeyframe(Keyframe& keyframe) {
		if (keyframes_.empty()) {
			// if empty, push one and set it as current keyframe
			keyframes_.push_back(keyframe);
			currentKeyframeIdx = 0;
		}
		else {
			// otherwise, insert a new frame next to the current keyframe
			std::list<Keyframe>::iterator it = keyframes_.begin();
			for (int i = 0; i < currentKeyframeIdx; ++i) {
				it++;
			}
			keyframes_.insert(it, keyframe);
			currentKeyframeIdx++;
		}
	}

	//! Remove keyframe from the list
	void removeKeyframe() {

	}

	void sendKeyframeToScene() {
		if (!keyframes_.empty()) {
			// copy the current keyframe to the scene graph
		}
	}

	void moveToNextFrame() {

	}

	void moveToPrevFrame() {

	}

	
private:
	std::list<Keyframe> keyframes_;
	int currentKeyframeIdx = UNDEFINED;
};

#endif