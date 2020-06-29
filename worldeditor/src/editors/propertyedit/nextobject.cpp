 #include "nextobject.h"

#include <QVariant>
#include <QColor>
#include <QEvent>

#include <object.h>
#include <invalid.h>

#include "custom/Vector2DProperty.h"
#include "custom/Vector3DProperty.h"
#include "custom/ColorProperty.h"
#include "custom/FilePathProperty.h"
#include "custom/AssetProperty.h"

#include <engine.h>
#include <components/component.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/textrender.h>
#include <resources/material.h>
#include <resources/mesh.h>
#include <resources/font.h>

#include "assetmanager.h"

#include "editors/componentselect/componentselect.h"

#include "controllers/objectctrl.h"

#include <QMap>
#include <QMenu>

enum Axises {
    AXIS_X = (1<<0),
    AXIS_Y = (1<<1),
    AXIS_Z = (1<<2)
};

Q_DECLARE_METATYPE(Vector4)
Q_DECLARE_METATYPE(Alignment)
Q_DECLARE_METATYPE(Axises)

#define COLOR       "Color"
#define AXISES      "Axises"
#define ALIGNMENT   "Alignment"
#define COMPONENT   "Component"
#define TEMPLATE    "Template"

const QString EditorTag("editor=");

NextObject::NextObject(Object *data, QObject *parent) :
        QObject(parent),
        m_pObject(data) {

    onUpdated();
}

QString NextObject::name() {
    if(m_pObject) {
        return m_pObject->name().c_str();
    }
    return QString();
}

void NextObject::setName(const QString &name) {
    if(m_pObject) {
        m_pObject->setName(qPrintable(name));
        emit updated();
    }
}

QMenu *NextObject::menu(Object *obj) {
    QMenu *result = nullptr;
    if(obj == nullptr || dynamic_cast<Transform *>(obj) || dynamic_cast<Actor *>(obj)) {
        return result;
    }

    result = new QMenu();
    QAction *del = new QAction(tr("Remove Component"), this);
    del->setProperty(COMPONENT, obj->typeName().c_str());
    result->addAction(del);

    connect(del, SIGNAL(triggered(bool)), this, SLOT(onDeleteComponent()));

    return result;
}

Object *NextObject::component(const QString &name) {
    QStringList path(name.split('/'));
    QStringList dir(path.mid(0, path.size()));
    return findChild(dir);
}

void NextObject::onUpdated() {
    foreach(QByteArray it, dynamicPropertyNames()) {
        setProperty(it, QVariant());
    }

    if(m_pObject) {
        buildObject(m_pObject);

        setObjectName(m_pObject->typeName().c_str());
    }
    emit updated();
}

void NextObject::onDeleteComponent() {
    emit deleteComponent(sender()->property(COMPONENT).toString());
}

void NextObject::buildObject(Object *object, const QString &path) {
    const MetaObject *meta = object->metaObject();
    blockSignals(true);
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);
        QString name(property.name());
        if(name.indexOf("Enabled") == -1) {
            name = path + "/" + name;
            Variant data = property.read(object);

            setProperty(qPrintable(name), qVariant(data, property));
        } else {
            setProperty(qPrintable(path + "/"), QVariant(true));
        }
    }
    blockSignals(false);

    for(Object *it : object->getChildren()) {
        Invalid *invalid = dynamic_cast<Invalid *>(it);
        if(invalid) {
            blockSignals(true);
            invalid->setName(tr("%1 (Invalid)").arg(invalid->typeName().c_str()).toStdString());
            setProperty( qPrintable(path + "/" + invalid->name().c_str() + QString("/")), QVariant(true) );
            blockSignals(false);
        } else if(dynamic_cast<Component *>(it)) {
            buildObject(it, (path.isEmpty() ? "" : path + "/") + QString::fromStdString(it->typeName()));
        }
    }
}

bool NextObject::event(QEvent *e) {
    if(e->type() == QEvent::DynamicPropertyChange && !signalsBlocked()) {
        QDynamicPropertyChangeEvent *ev = static_cast<QDynamicPropertyChangeEvent *>(e);
        QString name   = ev->propertyName();
        QVariant value = property(qPrintable(name));
        if(value.isValid()) {
            QStringList list = name.split('/');
            if(m_pObject) {
                Object *o = findChild(list);
                QString propertyName = list.join('/');
                Variant current = o->property(qPrintable(propertyName));

                const MetaObject *meta  = o->metaObject();
                int index = meta->indexOfProperty(qPrintable(propertyName));
                MetaProperty property = meta->property(index);

                Variant target = aVariant(value, current.userType(), editor(property));

                if(target.isValid() && current != target) {
                    emit aboutToBeChanged(o, propertyName, target);

                    onUpdated();

                    emit changed(o, propertyName);
                }
            }
        }
    }
    return false;
}

QString NextObject::editor(const MetaProperty &property) {
    if(property.table() && property.table()->annotation) {
        QString annotation(property.table()->annotation);
        QStringList list = annotation.split(',');
        foreach(QString it, list) {
            int index = it.indexOf(EditorTag);
            if(index > -1) {
                return it.remove(EditorTag);
            }
        }
    }
    return QString();
}

Object *NextObject::findChild(QStringList &path) {
    Object *parent  = m_pObject;
    if(parent == nullptr) {
        return nullptr;
    }
    foreach(QString str, path) {
        for(Object *it : parent->getChildren()) {
            if(it->typeName() == str.toStdString()) {
                parent  = it;
                path.pop_front();
                break;
            }
        }
    }
    return parent;
}

QVariant NextObject::qVariant(Variant &v, const MetaProperty &property) {
    QString editor = NextObject::editor(property);

    switch(v.userType()) {
        case MetaType::BOOLEAN: {
            return QVariant(v.toBool());
        }
        case MetaType::INTEGER: {
            int32_t value = v.toInt();
            if(editor == AXISES) {
                return QVariant::fromValue(static_cast<Axises>(value));
            } else if(editor == ALIGNMENT) {
                return QVariant::fromValue(static_cast<Alignment>(value));
            }
            return QVariant(v.toInt());
        }
        case MetaType::FLOAT: {
            return QVariant(v.toFloat());
        }
        case MetaType::STRING: {
            return QVariant(v.toString().c_str());
        }
        case MetaType::VECTOR2: {
            return QVariant::fromValue(v.toVector2());
        }
        case MetaType::VECTOR3: {
            return QVariant::fromValue(v.toVector3());
        }
        case MetaType::VECTOR4: {
            Vector4 value = v.toVector4();
            if(editor == COLOR) {
                QColor r;
                r.setRgbF(value.x, value.y, value.z, value.w);
                return QVariant(r);
            }
            return QVariant::fromValue(value);
        }
        default: break;
    }

    if(v.data() == nullptr) {
        return QVariant();
    }
    Object *o = *(reinterpret_cast<Object **>(v.data()));
    if(editor == COMPONENT) {
        Actor *actor = static_cast<Actor *>(m_pObject);
        SceneComponent cmp;
        cmp.type = QString(property.type().name()).replace(" *", "");
        cmp.component = static_cast<Component *>(o);
        cmp.scene = actor->scene();
        return QVariant::fromValue(cmp);
    }
    if(editor == TEMPLATE) {
        return QVariant::fromValue(Template(Engine::reference(o).c_str(), v.userType()));
    }
    return QVariant();
}

Variant NextObject::aVariant(QVariant &v, uint32_t type, const QString &editor) {
    if(type == MetaType::type<Alignment>()) {
        Alignment value = v.value<Alignment>();
        return Variant::fromValue(value);
    }

    switch(type) {
        case MetaType::BOOLEAN: {
            return Variant(v.toBool());
        }
        case MetaType::INTEGER: {
            if(editor == AXISES) {

            }
            return Variant(v.toInt());
        }
        case MetaType::FLOAT: {
            return Variant(v.toFloat());
        }
        case MetaType::STRING: {
            if(v.canConvert<QFileInfo>()) {
                QFileInfo p = v.value<QFileInfo>();
                return Variant(qUtf8Printable(p.absoluteFilePath()));
            } else if(v.canConvert<Template>()) {
                Template p = v.value<Template>();
                return Variant(qUtf8Printable(p.path));
            }
            return Variant(qUtf8Printable(v.toString()));
        }
        case MetaType::VECTOR2: {
            return Variant(v.value<Vector2>());
        }
        case MetaType::VECTOR3: {
            return Variant(v.value<Vector3>());
        }
        case MetaType::VECTOR4: {
            if(editor == COLOR) {
                QColor c = v.value<QColor>();
                return Variant(Vector4(c.redF(), c.greenF(), c.blueF(), c.alphaF()));
            }
            return Variant(v.value<Vector4>());
        }
        default: break;
    }

    if(editor == COMPONENT) {
        SceneComponent c = v.value<SceneComponent>();
        return Variant(type, &c.component);
    }
    if(editor == TEMPLATE) {
        Template p  = v.value<Template>();
        if(!p.path.isEmpty()) {
            Object *m = Engine::loadResource<Object>(qPrintable(p.path));
            return Variant(type, &m);
        }
    }

    return Variant();
}
