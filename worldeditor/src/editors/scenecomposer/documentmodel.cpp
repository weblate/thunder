#include "documentmodel.h"

#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

#include "assetmanager.h"
#include "projectmanager.h"

#include "editors/textedit/textedit.h"

#include "editors/textureedit/textureedit.h"
#include "editors/materialedit/materialedit.h"
#include "editors/meshedit/meshedit.h"
#include "editors/particleedit/particleedit.h"
#include "editors/animationedit/animationedit.h"

DocumentModel::DocumentModel() {
    addEditor(IConverter::ContentTexture, new TextureEdit(this));
    addEditor(IConverter::ContentMaterial, new MaterialEdit(this));
    addEditor(IConverter::ContentPrefab, new MeshEdit(this));
    addEditor(IConverter::ContentEffect, new ParticleEdit(this));
    addEditor(IConverter::ContentAnimationStateMachine, new AnimationEdit(this));
}

DocumentModel::~DocumentModel() {
    auto it = m_Documents.begin();
    while(it != m_Documents.end()) {
        QFileInfo info(it.key());
        int32_t type = AssetManager::instance()->resourceType(info);
        switch(type) {
            case IConverter::ContentCode: {
                TextEdit *editor = dynamic_cast<TextEdit *>(it.value());
                if(editor) {
                    editor->deleteLater();
                }
            } break;
            default: break;
        }
        ++it;
    }
    m_Documents.clear();

    for(auto it : m_Editors) {
        delete it;
    }
    m_Editors.clear();
}

void DocumentModel::addEditor(uint8_t type, IAssetEditor *editor) {
    m_Editors[type] = editor;
}

QString DocumentModel::fileName(IAssetEditor *editor) const {
    return m_Documents.key(editor);
}

IAssetEditor *DocumentModel::openFile(const QString &path) {
    auto it = m_Documents.find(path);
    if(it != m_Documents.end()) {
        return it.value();
    }

    QFileInfo info(path);
    int32_t type = AssetManager::instance()->resourceType(info);

    QDir dir(ProjectManager::instance()->contentPath());
    IConverterSettings *settings = AssetManager::instance()->fetchSettings(dir.absoluteFilePath(info.filePath()));

    IAssetEditor *editor = nullptr;
    auto e = m_Editors.find(type);
    if(e != m_Editors.end()) {
        editor = e.value();
    } else {
        switch(type) {
            case IConverter::ContentCode: {
                TextEdit *text = new TextEdit(this);
                editor = text;
            } break;
            default: break;
        }
    }
    editor->loadAsset(settings);
    m_Documents[path] = editor;

    return editor;
}

void DocumentModel::saveFile(IAssetEditor *editor) {
    if(editor) {
        editor->saveAsset(ProjectManager::instance()->contentPath() + "/" + m_Documents.key(editor));
    }
}

void DocumentModel::saveFileAs(IAssetEditor *editor) {
    if(editor) {
        QString dir = ProjectManager::instance()->contentPath();
        QString path = QFileDialog::getSaveFileName(nullptr,
                                                    tr("Save Document"),
                                                    dir,
                                                    "");
        if(path.length() > 0) {
            editor->saveAsset(path);
        }
    }
}

void DocumentModel::closeFile(const QString &path) {
    auto it = m_Documents.find(path);
    if(it != m_Documents.end()) {
        QFileInfo info(path);
        int32_t type = AssetManager::instance()->resourceType(info);
        switch(type) {
            case IConverter::ContentCode: {
                TextEdit *editor = dynamic_cast<TextEdit *>(it.value());
                if(editor) {
                    editor->deleteLater();
                }
            } break;
            default: break;
        }

        m_Documents.erase(it);
    }
}

void DocumentModel::saveAll() {
    foreach(auto it, m_Documents) {
        if(it->isModified()) {
            it->saveAsset();
        }
    }
}

bool DocumentModel::checkSave(IAssetEditor *editor) {
    if(editor->isModified()) {
        QMessageBox msgBox(nullptr);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("The file has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int result  = msgBox.exec();
        if(result == QMessageBox::Cancel) {
            return false;
        } else if(result == QMessageBox::Yes) {
            editor->saveAsset();
        } else {
            editor->setModified(false);
        }
    }
    return true;
}

QVariant DocumentModel::data(const QModelIndex &index, int role) const {
    switch(role) {
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            QString key = m_Documents.keys().at(index.row());
            QFileInfo info(key);
            if(index.column() == 0) {
                return info.fileName();
            }
        } break;
        default: break;
    }

    return QVariant();
}

QVariant DocumentModel::headerData(int, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return "";
    }
    return QVariant();
}

int DocumentModel::columnCount(const QModelIndex &) const {
    return 2;
}

QModelIndex DocumentModel::index(int row, int column, const QModelIndex &) const {
    if(row >= m_Documents.size()) {
        return QModelIndex();
    }
    return createIndex(row, column, nullptr);
}

QModelIndex DocumentModel::parent(const QModelIndex &) const {
    return QModelIndex();
}

int DocumentModel::rowCount(const QModelIndex &parent) const {
    if(parent.isValid()) {
        return 0;
    }
    return m_Documents.size();
}
