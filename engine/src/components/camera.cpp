#include "components/camera.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/meshrender.h"

#include "resources/pipeline.h"
#include "resources/texture.h"

class CameraPrivate {
public:
    CameraPrivate() {
        m_FOV       = 45.0; // 2*arctan(height/(2*distance))
        m_Near      = 0.1f;
        m_Far       = 1000.0;
        m_Ratio     = 1.0;
        m_Focal     = 1.0;
        m_OrthoSize = 1.0;
        m_Ortho     = false;
        m_Color     = Vector4();
        m_pPipeline = nullptr;
    }

    static inline bool intersect(Plane pl[6], Vector3 points[8]) {
        for(int i = 0; i < 6; i++) {
            if(pl[i].sqrDistance(points[0]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[1]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[2]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[3]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[4]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[5]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[6]) > 0) {
                continue;
            }
            if(pl[i].sqrDistance(points[7]) > 0) {
                continue;
            }
            return false;
        }
        return true;
    }

    bool m_Ortho;

    float m_FOV;

    float m_Near;

    float m_Far;

    float m_Ratio;

    float m_Focal;

    float m_OrthoSize;

    Vector4 m_Color;

    Pipeline *m_pPipeline;

    static Camera *s_pCurrent;
};

Camera *CameraPrivate::s_pCurrent  = nullptr;

/*!
    \class Camera
    \brief The Camera represents the player's point of view; how the player sees the world.
    \inmodule Engine

*/

Camera::Camera() :
    p_ptr(new CameraPrivate) {

}

Camera::~Camera() {
    delete p_ptr;
}
/*!
    Returns render pipline which attached to the camera.
*/
Pipeline *Camera::pipeline() {
    if(p_ptr->m_pPipeline == nullptr) {
        p_ptr->m_pPipeline = Engine::objectCreate<Pipeline>("Pipeline");
    }
    return p_ptr->m_pPipeline;
}
/*!
    Attaches render pipeline to the camera.
*/
void Camera::setPipeline(Pipeline *pipeline) {
    p_ptr->m_pPipeline = pipeline;
}
/*!
    Returns \a view and \a projection matrices using output parameters.
*/
void Camera::matrices(Matrix4 &view, Matrix4 &projection) const {
    projection = projectionMatrix();

    Transform *t = actor()->transform();
    Matrix4 m;
    m.translate(-t->position());
    view = Matrix4(t->rotation().toMatrix()).inverse() * m;
}
/*!
    Returns projection matrix for the camera.
*/
Matrix4 Camera::projectionMatrix() const {
    if(p_ptr->m_Ortho) {
        float width = p_ptr->m_OrthoSize * p_ptr->m_Ratio;
        return Matrix4::ortho(-width / 2, width / 2, -p_ptr->m_OrthoSize / 2, p_ptr->m_OrthoSize / 2, p_ptr->m_Near, p_ptr->m_Far);
    }
    return Matrix4::perspective(p_ptr->m_FOV, p_ptr->m_Ratio, p_ptr->m_Near, p_ptr->m_Far);
}
/*!
    Transforms position from \a worldSpace into \a screenSpace using \a modelView and \a projection matrices.
    Returns true if success; otherwise returns false.
*/
bool Camera::project(const Vector3 &worldSpace, const Matrix4 &modelView, const Matrix4 &projection, Vector3 &screenSpace) {
    Vector4 in;
    Vector4 out;

    in  = Vector4(worldSpace.x, worldSpace.y, worldSpace.z, 1.0f);
    out = modelView * in;
    in  = projection * out;

    if(in.w == 0.0f) {
        return false;
    }
    in.w  = 1.0f / in.w;
    in.x *= in.w;
    in.y *= in.w;
    in.z *= in.w;

    screenSpace.x = (in.x * 0.5f + 0.5f);
    screenSpace.y = (in.y * 0.5f + 0.5f);
    screenSpace.z = (1.0f + in.z) * 0.5f;

    return true;
}
/*!
    Transforms position from \a worldSpace into \a screenSpace using \a modelView and \a projection matrices.
    Returns true if success; otherwise returns false.
*/
bool Camera::unproject(const Vector3 &screenSpace, const Matrix4 &modelView, const Matrix4 &projection, Vector3 &worldSpace) {
    Matrix4 final;
    Vector4 in;
    Vector4 out;

    final = (projection * modelView).inverse();

    in.x = (screenSpace.x) * 2.0f - 1.0f;
    in.y = (screenSpace.y) * 2.0f - 1.0f;
    in.z = 2.0f * screenSpace.z - 1.0f;
    in.w = 1.0f;
    out  = final * in;

    if(out.w == 0.0f) {
        return false;
    }
    out.w = 1.0f / out.w;

    worldSpace.x = out.x * out.w;
    worldSpace.y = out.y * out.w;
    worldSpace.z = out.z * out.w;

    return true;
}
/*!
    Returns ray with origin point in camera position and direction to projection plane with \a x and \a y coordinates.
*/
Ray Camera::castRay(float x, float y) {
    Transform *t = actor()->transform();
    Vector3 p   = t->position();
    Vector3 dir = t->rotation() * Vector3(0.0, 0.0,-1.0);
    dir.normalize();

    Vector3 view;
    if(p_ptr->m_Ortho) {
        p = Vector3(p.x + (x - 0.5f) * p_ptr->m_OrthoSize * p_ptr->m_Ratio,
                    p.y - (y - 0.5f) * p_ptr->m_OrthoSize, p.z);
    } else {
        float tang    = tan(p_ptr->m_FOV * DEG2RAD);
        Vector3 right = dir.cross(Vector3(0.0f, 1.0f, 0.0f));
        Vector3 up    = right.cross(dir);
        view = Vector3( (x - 0.5f) * tang * p_ptr->m_Ratio) * right +
               Vector3(-(y - 0.5f) * tang) * up + p + dir;

        dir = (view - p);
        dir.normalize();
    }
    return Ray(p, dir);
}
/*!
    Returns field of view angle for the camera in degrees.
*/
float Camera::fov() const {
    return p_ptr->m_FOV;
}
/*!
    Sets field of view angle for the camera in degrees.
    \note Applicable only for the perspective mode.
*/
void Camera::setFov(const float value) {
    p_ptr->m_FOV   = value;
}
/*!
    Returns a distance to near cut plane.
*/
float Camera::nearPlane() const {
    return p_ptr->m_Near;
}
/*!
    Sets a distance to near cut plane.
*/
void Camera::setNear(float value) {
    p_ptr->m_Near  = value;
}
/*!
    Returns a distance to far cut plane.
*/
float Camera::farPlane() const {
    return p_ptr->m_Far;
}
/*!
    Sets a distance to far cut plane.
*/
void Camera::setFar(const float value) {
    p_ptr->m_Far   = value;
}
/*!
    Returns the aspect ratio (width divided by height).
*/
float Camera::ratio() const {
    return p_ptr->m_Ratio;
}
/*!
    Sets the aspect \a ratio (width divided by height).
*/
void Camera::setRatio(float ratio) {
    p_ptr->m_Ratio = ratio;
}
/*!
    Returns a focal distance for the camera.
*/
float Camera::focal() const {
    return p_ptr->m_Focal;
}
/*!
    Sets a focal distance for the camera.
*/
void Camera::setFocal(const float focal) {
    p_ptr->m_Focal = focal;
}
/*!
    Returns the color with which the screen will be cleared.
*/
Vector4 Camera::color() const {
    return p_ptr->m_Color;
}
/*!
    Sets the \a color with which the screen will be cleared.
*/
void Camera::setColor(const Vector4 &color) {
    p_ptr->m_Color = color;
}
/*!
    Returns camera size for orthographic mode.
*/
float Camera::orthoSize() const {
    return p_ptr->m_OrthoSize;
}
/*!
    Sets camera \a size for orthographic mode.
*/
void Camera::setOrthoSize(const float size) {
    p_ptr->m_OrthoSize = size;
}
/*!
    Returns true for the orthographic mode; for the perspective mode, returns false.
*/
bool Camera::orthographic() const {
    return p_ptr->m_Ortho;
}
/*!
    Sets orthographic \a mode.
*/
void Camera::setOrthographic(const bool mode) {
    p_ptr->m_Ortho = mode;
}
/*!
    Returns current active camera.
*/
Camera *Camera::current() {
    return CameraPrivate::s_pCurrent;
}
/*!
    Sets current active camera.
*/
void Camera::setCurrent(Camera *current) {
    CameraPrivate::s_pCurrent = current;
}
/*!
    Returns frustum corners for the \a camera.
*/
array<Vector3, 8> Camera::frustumCorners(const Camera &camera) {
    Transform *t = camera.actor()->transform();

    return Camera::frustumCorners(camera.p_ptr->m_Ortho, (camera.p_ptr->m_Ortho) ?
                                                          camera.p_ptr->m_OrthoSize :
                                                          camera.p_ptr->m_FOV,
                                                          camera.p_ptr->m_Ratio,
                                  t->worldPosition(), t->worldRotation(),
                                  camera.p_ptr->m_Near, camera.p_ptr->m_Far);
}
/*!
    Returns frustum corners with provided parameters.
*/
array<Vector3, 8> Camera::frustumCorners(bool ortho, float sigma, float ratio, const Vector3 &position, const Quaternion &rotation, float nearPlane, float farPlane) {
    float nh;
    float fh;
    float nw;
    float fw;
    if(ortho) {
        nh = sigma * 0.5f;
        nw = nh * ratio;
        fw = nw;
        fh = nh;
    } else {
        float tang  = tanf(sigma * DEG2RAD * 0.5f);
        nh = nearPlane * tang;
        fh = farPlane * tang;
        nw = nh * ratio;
        fw = fh * ratio;
    }

    Vector3 dir   = rotation * Vector3(0.0f, 0.0f,-1.0f);
    Vector3 right = dir.cross(rotation * Vector3(0.0f, 1.0f, 0.0f));
    Vector3 up    = right.cross(dir);
    Vector3 nc    = position + dir * nearPlane;
    Vector3 fc    = position + dir * farPlane;

    return {nc + up * nh - right * nw,
            nc + up * nh + right * nw,
            nc - up * nh + right * nw,
            nc - up * nh - right * nw,

            fc + up * fh - right * fw,
            fc + up * fh + right * fw,
            fc - up * fh + right * fw,
            fc - up * fh - right * fw};
}
/*!
    Filters out an incoming \a list which are not in the frustum.
    Returns filtered list.
*/
Object::ObjectList Camera::frustumCulling(ObjectList &list, const array<Vector3, 8> &frustum) {
    Plane pl[6];
    pl[0] = Plane(frustum[1], frustum[0], frustum[4]); // top
    pl[1] = Plane(frustum[7], frustum[3], frustum[2]); // bottom
    pl[2] = Plane(frustum[3], frustum[7], frustum[0]); // left
    pl[3] = Plane(frustum[2], frustum[1], frustum[6]); // right
    pl[4] = Plane(frustum[0], frustum[1], frustum[3]); // near
    pl[5] = Plane(frustum[5], frustum[4], frustum[6]); // far

    Object::ObjectList result;
    for(auto it : list) {
        Renderable *item = static_cast<Renderable *>(it);
        AABBox box = item->bound();
        if(box.extent.x < 0.0f || box.intersect(pl, 6)) {
            result.push_back(item);
        }
    }
    return result;
}

#ifdef NEXT_SHARED
#include "handles.h"

bool Camera::drawHandles(ObjectList &selected) {
    Transform *t = actor()->transform();
    if(isSelected(selected)) {
        array<Vector3, 8> a = frustumCorners(p_ptr->m_Ortho, (p_ptr->m_Ortho) ? p_ptr->m_OrthoSize : p_ptr->m_FOV,
                                             p_ptr->m_Ratio, t->worldPosition(), t->worldRotation(), nearPlane(), farPlane());

        Handles::s_Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
        Vector3Vector points(a.begin(), a.end());
        Mesh::IndexVector indices   = {0, 1, 1, 2, 2, 3, 3, 0,
                                       4, 5, 5, 6, 6, 7, 7, 4,
                                       0, 4, 1, 5, 2, 6, 3, 7};

        Handles::drawLines(Matrix4(), points, indices);
    }
    Handles::s_Color = Handles::s_Normal;
    return Handles::drawBillboard(t->position(), Vector2(0.5f), Engine::loadResource<Texture>(".embedded/camera.png"));
}
#endif
