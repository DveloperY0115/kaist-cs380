#ifndef ANIMATION_H
#define ANIMATION_H

#include <algorithm>
#include <fstream>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "io.h"
#include "interpolation.h"
#include "scenegraph.h"
#include "sgutils.h"

namespace Animation {

	using SceneRbtVector = std::vector<std::shared_ptr<SgRbtNode>>;
	using Frame = std::vector<RigTForm>;

	class KeyframeList {

	public:
		//! Default constructor
		//! Creates an empty list of keyframe
		KeyframeList() {
			keyframes_ = std::list<Frame>();
			currentKeyframeIter = keyframes_.end();
		}

		//! Default destructor
		~KeyframeList() = default;

		//! Getter for current keyframe
		//! Warning: Call on empty list is undefined
		Frame& getCurrentKeyframe() {
			return *currentKeyframeIter;
		}

		//! Get frame by index
		//! Index should be in range [-1, n]
		//! where n = length of keyframes - 1
		Frame getFrameByIdx(int idx) {
			// assert(idx >= -1 && idx < keyframes_.size() - 1);
			std::list<Frame>::iterator iter = keyframes_.begin();
			for (int i = -1; i < idx; ++i) {
				iter++;
			}
			return *iter;
		}

		//! Replace the RBT of currently selected keyframe with the input
		void updateCurrentKeyframe(Frame frame) {
			*currentKeyframeIter = frame;
		}

		//! Replace the current keyframe iterator with the input
		void setCurrentKeyframeAs(std::list<Frame>::iterator iter) {
			currentKeyframeIter = iter;
		}

		//! Returns the iterator pointing at the first keyframe
		std::list<Frame>::iterator begin() {
			return keyframes_.begin();
		}

		//! Returns the iterator pointing at the last keyframe
		std::list<Frame>::iterator end() {
			return keyframes_.end();
		}

		//! Returns the length of keyframe list
		int size() {
			return keyframes_.size();
		}

		//! Returns true if the keyframe list is empty,
		//! false otherwise
		bool empty() {
			return keyframes_.empty();
		}

		//! Add new keyframe to the list
		void addNewKeyframe(Frame& keyframe) {
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
		//!		Then update the scene graph using the new key frame
		void removeCurrentKeyframe(SceneRbtVector& nodes) {
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
						currentKeyframeIter = keyframes_.erase(currentKeyframeIter);
					}
					// Case (2) - (i)
					else {
						std::list<Frame>::iterator prevIter = currentKeyframeIter;
						prevIter--;
						currentKeyframeIter = keyframes_.erase(currentKeyframeIter);
						currentKeyframeIter = prevIter;
					}
					sendCurrentKeyframeToScene(nodes);
				}
			}
		}

		//! Copy current keyframe to the scene graph
		void sendCurrentKeyframeToScene(SceneRbtVector& nodes) {
			if (currentKeyframeIter == keyframes_.end()) {
				std::cerr << "Current keyframe is undefined!\n";
			}
			else {
				const Frame currentKeyframe = *currentKeyframeIter;
				setSgRbtNodes(nodes, currentKeyframe);
			}
		}

		//! Advance to the next keyframe if possible
		//! If already at the end of the list, do nothing
		void advanceFrame(SceneRbtVector& nodes) {
			if (currentKeyframeIter == keyframes_.end()) {
				std::cerr << "Current keyframe is undefined! (list is empty)\n";
			}
			else {
				std::list<Frame>::iterator lastElem = keyframes_.end();
				lastElem--;
				if (currentKeyframeIter == lastElem) {
					std::cerr << "This is the last keyframe!\n";
				}
				else {
					currentKeyframeIter++;
					setSgRbtNodes(nodes, *currentKeyframeIter);
				}
			}
		}

		//! Retreat to the previous keyframe if possible
		//! If already at the beginning of the list, do nothing
		void retreatFrame(SceneRbtVector& nodes) {
			if (currentKeyframeIter == keyframes_.end()) {
				std::cerr << "Current keyframe is undefined! (list is empty)\n";
			}
			else {
				if (currentKeyframeIter == keyframes_.begin())
					std::cerr << "This is the first keyframe!\n";
				else {
					currentKeyframeIter--;
					setSgRbtNodes(nodes, *currentKeyframeIter);
				}
			}
		}

		//! Export the list of keyframes held by this instance
		void exportKeyframeList(std::string filename) {

			if (keyframes_.empty()) {
				// Do nothing
			}

			//! Get number of frames in the list & number of RBTs in one keyframe
			//! Stored as header (metadata)
			unsigned int numFrames = keyframes_.size();
			unsigned int numRbts = (*keyframes_.begin()).size();

			std::ofstream file(filename);

			if (file.is_open()) {
				file << numFrames << " " << numRbts << "\n";

				for (std::list<Frame>::iterator frameIter = keyframes_.begin(); frameIter != keyframes_.end(); ++frameIter) {
					Frame currentFrame = *frameIter;
					for (std::vector<RigTForm>::iterator rbtIter = currentFrame.begin(); rbtIter != currentFrame.end(); ++rbtIter) {
						RigTForm currentRbt = *rbtIter;
						Cvec3 t = currentRbt.getTranslation();
						Quat q = currentRbt.getRotation();
						file << t[0] << " " << t[1] << " " << t[2] << " " << q[0] << " " << q[1] << " " << q[2] << " " << q[3];
						file << " ";    // separator between RigTForm data
					}
					file << "\n";    // separator between Frame data
				}

				file.close();
			}

			std::cout << "Exported file located at: " << filename << "\n";
		}

		//! Import the list of keyframes stored in the disk
		void importKeyframeList(std::string filename) {

			std::ifstream file(filename);
			std::string line;

			std::list<Frame> keyframes_in = std::list<Frame>();

			if (file.is_open()) {

				int numFrames = -1;
				int numRbts = -1;

				// read the first line containing header in our convention
				if (std::getline(file, line)) {
					std::vector<std::string> header = split<std::string>(line, ' ');
					numFrames = std::stoi(header[0]);
					numRbts = std::stoi(header[1]);
					assert(numFrames > 0 && numRbts > 0);
				}

				while (std::getline(file, line)) {
					std::vector<std::string> data = split<std::string>(line, ' ');
					std::reverse(data.begin(), data.end());    // reverse vector to use pop_back iteratively

					Frame frame = std::vector<RigTForm>();

					for (int i = 0; i < numRbts; ++i) {

						// extract translation
						float t_x = std::stof(data.back());
						data.pop_back();
						float t_y = std::stof(data.back());
						data.pop_back();
						float t_z = std::stof(data.back());
						data.pop_back();

						// extract rotation
						float q_0 = std::stof(data.back());
						data.pop_back();
						float q_1 = std::stof(data.back());
						data.pop_back();
						float q_2 = std::stof(data.back());
						data.pop_back();
						float q_3 = std::stof(data.back());
						data.pop_back();

						Cvec3 t = Cvec3(t_x, t_y, t_z);
						Quat q = Quat(q_0, q_1, q_2, q_3);
						RigTForm rbt = RigTForm(t, q);

						frame.push_back(rbt);
					}

					keyframes_in.push_back(frame);
					numFrames--;
				}

				assert(numFrames == 0);    // check whether the reader consumed all data in the file
				file.close();
			}

			if (keyframes_in.empty()) {
				std::cout << "Something went wrong! Doing nothing...\n";
			}
			else {
				std::cout << "Imported file located at: " << filename << "\n";
				keyframes_ = keyframes_in;
				currentKeyframeIter = keyframes_.begin();    // set the first frame as the current keyframe
			}
		}

		//! Interpolate between keyframes
		//! following Catmull-Rom spline
		//! Return false when reached the end of the keyframes
		//! Return true otherwise (making animation proceed)
		bool interpolateKeyframes(float t, Frame& interFrame) {
			assert(interFrame.empty());

			const int startFrameIdx = floor(t);
			const int endFrameIdx = floor(t + 1);
			const float alpha = t - floor(t);

			if (endFrameIdx > keyframes_.size() - 2) {
				return true;
			}

			Frame interpolantFrame0 = getFrameByIdx(startFrameIdx - 1);
			Frame interpolantFrame1 = getFrameByIdx(endFrameIdx + 1);
			Frame startFrame = getFrameByIdx(startFrameIdx);
			Frame endFrame = getFrameByIdx(endFrameIdx);

			Frame::iterator interpolantIter0 = interpolantFrame0.begin();
			Frame::iterator interpolantIter1 = interpolantFrame1.begin();
			Frame::iterator startIter = startFrame.begin();
			Frame::iterator endIter = endFrame.begin();

			while (startIter != startFrame.end() && endIter != endFrame.end()) {
				interFrame.push_back(Interpolation::CatmullRom(*interpolantIter0, *startIter, *endIter, *interpolantIter1, alpha));
				interpolantIter0++;
				interpolantIter1++;
				startIter++;
				endIter++;
			}

			return false;
		}
		/*
		* Helper functions
		*/

	public:
		//! Print the current keyframe index
		void printCurrentKeyframeIdx() {
			std::cout << "Current keyframe is keyframe # [" << getCurrentKeyframeIdx() << "]\n";
		}

	private:
		//! Calculate the curernt keyframe index
		int getCurrentKeyframeIdx() {
			if (currentKeyframeIter == keyframes_.end()) {
				return -100;
			}
			int idx = -1;
			std::list<Frame>::iterator iter = keyframes_.begin();
			while (iter != currentKeyframeIter) {
				iter++;
				idx++;
			}
			return idx;
		}

	private:
		std::list<Frame> keyframes_;
		std::list<Frame>::iterator currentKeyframeIter;
	};
}

#endif