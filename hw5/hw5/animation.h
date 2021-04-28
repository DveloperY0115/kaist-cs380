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
	KeyframeList() {
		keyframes_ = std::list<Keyframe>();
		currentKeyframeIter = keyframes_.end();
	}

	//! Default destructor
	~KeyframeList() = default;

	//! Getter for current keyframe
	//! Warning: Call on empty list is undefined
	Keyframe getCurrentKeyframe() {
		return *currentKeyframeIter;
	}

	//! Add new keyframe to the list
	void addNewKeyframe(Keyframe& keyframe) {
		if (keyframes_.empty()) {
			keyframes_.push_back(keyframe);
			currentKeyframeIter = keyframes_.begin();
		}
		else {
			// otherwise, insert a new frame next to the current keyframe
			currentKeyframeIter++;
			keyframes_.insert(currentKeyframeIter, keyframe);    // 'insert' will insert new element before the current iterator
			currentKeyframeIter--;
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
			// Case (1)
			if (keyframes_.size() == 1) {
				keyframes_.erase(currentKeyframeIter);
				currentKeyframeIter = keyframes_.end();
			}
			else {
				// Case (2) - (ii)
				if (currentKeyframeIter == keyframes_.begin()) {
					std::list<Keyframe>::iterator nextIter = ++currentKeyframeIter;
					keyframes_.erase(currentKeyframeIter);
					currentKeyframeIter = nextIter;
				}
				// Case (2) - (i)
				else {
					std::list<Keyframe>::iterator prevIter = --currentKeyframeIter;
					keyframes_.erase(currentKeyframeIter);
					currentKeyframeIter = prevIter;
				}
			}
		}
	}
	
	//! Copy current keyframe to the scene graph
	void sendCurrentKeyframeToScene(SceneRbtVector& nodes) {
		if (currentKeyframeIter == keyframes_.end()) {
			std::cerr << "Current keyframe is undefined!\n";
		}
		else {
			const Keyframe currentKeyframe = *currentKeyframeIter;
			setSgRbtNodes(nodes, currentKeyframe);
		}
	}

	//! Advance to the next keyframe if possible
	//! If already at the end of the list, do nothing
	void advanceFrame() {
		if (currentKeyframeIter == keyframes_.end()) {
			std::cerr << "Current keyframe is undefined! (list is empty)\n";
		}
		else {
			currentKeyframeIter++;

			if (currentKeyframeIter == keyframes_.end())
				std::cerr << "There's no next keyframe\n";
				currentKeyframeIter--;
		}
	}

	//! Retreat to the previous keyframe if possible
	//! If already at the beginning of the list, do nothing
	void retreatFrame() {
		if (currentKeyframeIter == keyframes_.end()) {
			std::cerr << "Current keyframe is undefined! (list is empty)\n";
		}
		else {
			if (currentKeyframeIter == keyframes_.begin())
				std::cerr << "There's no previous keyframe\n";
			else {
				currentKeyframeIter--;
			}
		}
	}

	
private:
	std::list<Keyframe> keyframes_;
	std::list<Keyframe>::iterator currentKeyframeIter;
};

#endif