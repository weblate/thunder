#ifndef ASSIMPCONVERTER_H
#define ASSIMPCONVERTER_H

#include "converters/converter.h"

#include "resources/pose.h"

class Actor;

class MeshSerial;

class aiScene;
class aiNode;
class aiMesh;
class aiBone;
class aiAnimation;

class FbxImportSettings;

typedef list<const aiBone *> BonesList;
typedef map<string, Actor *> ActorsMap;

class AssimpImportSettings : public IConverterSettings {
    Q_OBJECT

    Q_PROPERTY(bool Use_Custom_Scale READ useScale WRITE setUseScale DESIGNABLE true USER true)
    Q_PROPERTY(float Custom_Scale READ customScale WRITE setCustomScale DESIGNABLE true USER true)
    Q_PROPERTY(bool Import_Color READ colors WRITE setColors DESIGNABLE true USER true)
    Q_PROPERTY(bool Import_Normals READ normals WRITE setNormals DESIGNABLE true USER true)

    Q_PROPERTY(bool Import_Animation READ animation WRITE setAnimation DESIGNABLE true USER true)
    Q_PROPERTY(Compression Compress_Animation READ filter WRITE setFilter DESIGNABLE true USER true)
    Q_PROPERTY(float Position_Error READ positionError WRITE setPositionError DESIGNABLE true USER true)
    Q_PROPERTY(float Rotation_Error READ rotationError WRITE setRotationError DESIGNABLE true USER true)
    Q_PROPERTY(float Scale_Error READ scaleError WRITE setScaleError DESIGNABLE true USER true)

public:

    enum Compression {
        Off = 0,
        Keyframe_Reduction
    };
    Q_ENUM(Compression)

    AssimpImportSettings();

    bool colors() const;
    void setColors(bool value);

    bool normals() const;
    void setNormals(bool value);

    bool animation() const;
    void setAnimation(bool value);

    Compression filter() const;
    void setFilter(Compression value);

    bool useScale() const;
    void setUseScale(bool value);

    float customScale() const;
    void setCustomScale(float value);

    float positionError() const;
    void setPositionError(float value);

    float rotationError() const;
    void setRotationError(float value);

    float scaleError() const;
    void setScaleError(float value);

    Object::ObjectList m_Renders;

    QStringList m_Resources;

    BonesList m_Bones;

    ActorsMap m_Actors;

    Actor *m_pRootActor;
    Actor *m_pRootBone;

    bool m_Flip;

protected:
    bool m_UseScale;
    float m_Scale;

    bool m_Colors;
    bool m_Normals;

    bool m_Animation;
    Compression m_Filter;

    float m_PositionError;
    float m_RotationError;
    float m_ScaleError;

};

class AssimpConverter : public IConverter {
public:
    AssimpConverter();

    QStringList suffixes() const { return {"fbx"}; }
    uint32_t contentType() const { return ContentPrefab; }
    uint32_t type() const { return MetaType::type<Actor *>(); }
    uint8_t convertFile(IConverterSettings *);

    IConverterSettings *createSettings() const;

    Actor *importObject(const aiScene *scene, const aiNode *element, Actor *parent, AssimpImportSettings *fbxSettings);

    static MeshSerial *importMesh(const aiMesh *mesh, Actor *parent, AssimpImportSettings *fbxSettings);

    static void importAnimation(const aiScene *scene, AssimpImportSettings *fbxSettings);

    static void importPose(AssimpImportSettings *fbxSettings);

    static QString saveData(const ByteArray &data, const QString &path, int32_t type, AssimpImportSettings *settings);
};

#endif // ASSIMPCONVERTER_H
