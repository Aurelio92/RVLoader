#pragma once

#include <vector>
#include <gccore.h>
#include <ogc/lwp_watchdog.h>

using namespace std;

template <class T>
class IntInterpolator {
    public:
        static T interpolate(T start, T end, float t) {
            T temp = start + t * (end - start);
            return (T)temp;
        }
};

template <class T = int, class interpolator = IntInterpolator<T> >
class Animation {
    private:
        class Step {
            public:
                u64 time;
                u64 duration;
                T start;
                T end;
        };

        u64 startTime; //ticks
        u64 pauseTime;
        u64 totDuration;
        vector<Step> steps;
        bool playing;
        T* output;

        int findStep(u64 time);
    public:
        Animation() {
            startTime = 0;
            pauseTime = 0;
            totDuration = 0;
            playing = false;
            output = NULL;
        }

        ~Animation() {
            steps.clear();
        }

        void addStep(u64 duration, const T& start, const T& end);
        void addReturnToHomeStep(u64 duration);

        void setOutput(T* _output) {
            output = _output;
        }

        bool animate();

        void pause() {
            pauseTime = gettime() - startTime;
            playing = false;
        }

        void resume() {
            startTime = gettime() - (pauseTime - startTime);
            playing = true;
        }

        void stop() {
            startTime = 0;
            pauseTime = 0;
            playing = false;
        }

        void reset() {
            startTime = gettime();
        }
};

template <class T, class interpolator>
void Animation<T, interpolator>::addStep(u64 duration, const T& start, const T& end) {
    Step s;
    if (steps.empty()) {
        //steps vector is empty. Start from zero
        s.time = 0;
    } else {
        //steps vector is not empty. Compute start time for this step
        Step back = steps.back();
        s.time = back.time + back.duration;
    }
    s.duration = duration;
    s.start = start;
    s.end = end;
    steps.push_back(s);

    //Increment total duration
    totDuration += duration;
}

template <class T, class interpolator>
void Animation<T, interpolator>::addReturnToHomeStep(u64 duration) {
    //Only add it if there is at least one animation (i.e. a home is defined)
    if (!steps.empty()) {
        Step s;
        Step back = steps.back();
        s.time = back.time + back.duration;
        s.duration = duration;
        s.start = back.end;
        s.end = steps[0].start;
        steps.push_back(s);

        //Increment total duration
        totDuration += duration;
    }
}

template <class T, class interpolator>
int Animation<T, interpolator>::findStep(u64 time) {
    if (time >= totDuration || steps.empty())
        return -1; //Not found

    //Binary search
    int left = 0;
    int right = steps.size() - 1;
    int center;

    while(left <= right) {
        center = (left + right) / 2;
        Step s = steps[center];
        u64 tEnd = s.time + s.duration;

        if (time >= s.time && time <= tEnd) {
            return center;
        }

        //Keep only right side if our target time is greater than center's tEnd
        if (time > tEnd)
            left = center + 1;
        else //Keep only left side otherwise
            right = center - 1;
    }

    return -1; //Should never happen
}

template <class T, class interpolator>
bool Animation<T, interpolator>::animate() {
    u64 time;
    if (playing)
        time = (gettime() - startTime) % totDuration;
    else
        time = pauseTime;

    int idx = findStep(time);
    if (idx < 0)
        return false;

    if (output != NULL)
        *output = interpolator::interpolate(steps[idx].start, steps[idx].end, (float)(time - steps[idx].time) / steps[idx].duration);

    return true;
}
