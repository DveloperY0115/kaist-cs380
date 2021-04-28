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

	//! Getter for current keyframe
	//! Warning: Call on empty list is undefined
	Keyframe getCurrentKeyframe() {
		assert(!keyframes_.empty());
		std::list<Keyframe>::iterator it = keyframes_.begin();
		for (int i = 0; i < currentKeyframeIdx; ++i) {
			it++;
		}
		return *it;
	}

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

	//! Remove current keyframe from the list, update the scene graph with the new key frame
	//! The behavior of this function follows the specification
	//! 
	//! (1) If the list becomes empty after deletion, set current keyframe to be UNDEFINED
	//! (2) Otherwise,
	//!		(i) If the deleted frame was NOT the first frame, set the current frame as the one before it
	//!		(ii) If the deleted frame was the first frame, then set the current frame as the one next to it
	void removeCurrentKeyframe() {
		if (keyframes_.empty()) {
			std::cerr << "There's no keyframe!\n";
			return;
		}
		else {
			// Remove the current keyframe from the list
			std::list<Keyframe>::iterator currentIt = keyframes_.begin();
			for (int i = 0; i < currentKeyframeIdx; ++i) {
				currentIt++;
			}
			keyframes_.remove(*currentIt);

			// Case (1)
			if (keyframes_.empty()) {
				currentKeyframeIdx = UNDEFINED;
			}
			else {
				// Case (2) - (i)
				if (currentKeyframeIdx != 0) {
					currentKeyframeIdx -= 1;
				}
				// Case (2) - (ii)
				else {
					// Do nothing
				}
			}
		}
	}

	void sendKeyframeToScene() {
		if (!keyframes_.empty()) {
			// copy the current keyframe to the scene graph
		}
		else {
			std::cerr << "There's no keyframe!\n";
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