#include "resources/animationclip.h"

#define TRACKS  "Tracks"

static hash<string> hash_str;

/*!
    \class AnimationClip
    \brief AnimationClip resource contains keyframe based animation data.
    \inmodule Resource

    An AnimationClip resource contains keyframe based animation data.
    The animation data split to a number of tracks that must be connected with the properties of Components.
    Each track can contain multiple curves or channels like X, Y and Z
    Which allows them to animate elements independently.
*/

/*!
    \internal
*/
void AnimationClip::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();
    {
        auto section = data.find(TRACKS);
        if(section != data.end()) {
            m_Tracks.clear();
            for(auto it : (*section).second.value<VariantList>()) {
                VariantList trackList = it.toList();
                auto i = trackList.begin();

                Track track;
                track.path = (*i).toString();
                i++;
                track.property = (*i).toString();
                i++;

                track.hash = hash_str(track.path + "." + track.property);

                for(auto it : (*i).toList()) {
                    VariantList curveList = it.toList();
                    auto t = curveList.begin();

                    int32_t component = (*t).toInt();
                    t++;

                    AnimationCurve curve;
                    while(t != curveList.end()) {
                        VariantList keyList = (*t).toList();
                        auto k = keyList.begin();

                        AnimationCurve::KeyFrame key;
                        key.m_Position = static_cast<uint32_t>((*k).toInt());
                        k++;
                        key.m_Type = static_cast<AnimationCurve::KeyFrame::Type>((*k).toInt());
                        k++;
                        key.m_Value = (*k).toFloat();
                        k++;
                        key.m_LeftTangent = (*k).toFloat();
                        k++;
                        key.m_RightTangent = (*k).toFloat();

                        curve.m_Keys.push_back(key);

                        t++;
                    }
                    track.curves[component] = curve;
                }
                m_Tracks.push_back(track);
            }
        }
    }
}
/*!
    Returns duration of the animation clip in milliseconds.
*/
uint32_t AnimationClip::duration() const {
    PROFILE_FUNCTION();

    uint32_t result = 0;
    for(auto track : m_Tracks) {
        for(auto curve : track.curves) {
            result = MAX(curve.second.m_Keys.back().m_Position, result);
        }
    }
    return result;
}
