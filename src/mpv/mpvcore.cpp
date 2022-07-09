/*
 * SPDX-FileCopyrightText: 2022 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "mpvcore.h"
#include "mpvrenderer.h"

#include <cstring>

MpvCore::MpvCore(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    m_mpv = mpv_create();
    if (!m_mpv) {
        qFatal("could not create mpv context");
    }
    if (mpv_initialize(m_mpv) < 0) {
        qFatal("could not initialize mpv context");
    }
}

MpvCore::~MpvCore()
{
    if (m_mpv_gl) {
        mpv_render_context_free(m_mpv_gl);
    }
    mpv_terminate_destroy(m_mpv);
}

QQuickFramebufferObject::Renderer *MpvCore::createRenderer() const
{
    return new MpvRenderer(const_cast<MpvCore *>(this));
}

void MpvCore::mpvEvents(void *ctx)
{
    QMetaObject::invokeMethod(static_cast<MpvCore*>(ctx), &MpvCore::eventHandler, Qt::QueuedConnection);
}

mpv_node_list *MpvCore::create_list(mpv_node *dst, bool is_map, int num)
{
    dst->format = is_map ? MPV_FORMAT_NODE_MAP : MPV_FORMAT_NODE_ARRAY;
    mpv_node_list *list = new mpv_node_list();
    dst->u.list = list;
    if (!list) {
        list = nullptr;
    }
    list->values = new mpv_node[num]();
    if (!list->values){
        list = nullptr;
    }
    if (is_map) {
        list->keys = new char*[num]();
        if (!list->keys){
            list = nullptr;
        }
    }
    return list;
}

void MpvCore::setNode(mpv_node *dst, const QVariant &src)
{
    if (test_type(src, QMetaType::QString)) {
        dst->format = MPV_FORMAT_STRING;
        dst->u.string = dup_qstring(src.toString());
        if (!dst->u.string)
            dst->format = MPV_FORMAT_NONE;
    } else if (test_type(src, QMetaType::Bool)) {
        dst->format = MPV_FORMAT_FLAG;
        dst->u.flag = src.toBool() ? 1 : 0;
    } else if (test_type(src, QMetaType::Int) ||
               test_type(src, QMetaType::LongLong) ||
               test_type(src, QMetaType::UInt) ||
               test_type(src, QMetaType::ULongLong))
    {
        dst->format = MPV_FORMAT_INT64;
        dst->u.int64 = src.toLongLong();
    } else if (test_type(src, QMetaType::Double)) {
        dst->format = MPV_FORMAT_DOUBLE;
        dst->u.double_ = src.toDouble();
    } else if (src.canConvert<QVariantList>()) {
        QVariantList qlist = src.toList();
        mpv_node_list *list = create_list(dst, false, qlist.size());
        if (!list)
            dst->format = MPV_FORMAT_NONE;
        list->num = qlist.size();
        for (int n = 0; n < qlist.size(); n++)
            setNode(&list->values[n], qlist[n]);
    } else if (src.canConvert<QVariantMap>()) {
        QVariantMap qmap = src.toMap();
        mpv_node_list *list = create_list(dst, true, qmap.size());
        if (!list)
            dst->format = MPV_FORMAT_NONE;
        list->num = qmap.size();
        int n = 0;
        for (auto it = qmap.constKeyValueBegin(); it != qmap.constKeyValueEnd(); ++it) {
            list->keys[n] = dup_qstring(it.operator*().first);
            if (!list->keys[n]) {
                free_node(dst);
                dst->format = MPV_FORMAT_NONE;
            }
            setNode(&list->values[n], it.operator*().second);
            ++n;
        }
    } else {
        dst->format = MPV_FORMAT_NONE;
    }
    return;
}

bool MpvCore::test_type(const QVariant &v, QMetaType::Type t)
{
    // The Qt docs say: "Although this function is declared as returning
    // QVariant::Type(obsolete), the return value should be interpreted
    // as QMetaType::Type." So a cast is needed to avoid warnings.
    return static_cast<int>(v.type()) == static_cast<int>(t);
}

char *MpvCore::dup_qstring(const QString &s)
{
    QByteArray b = s.toUtf8();
    char *r = new char[b.size() + 1];
    if (r)
        std::memcpy(r, b.data(), b.size() + 1);
    return r;
}

void MpvCore::free_node(mpv_node *dst)
{
    switch (dst->format) {
    case MPV_FORMAT_STRING:
        delete[] dst->u.string;
        break;
    case MPV_FORMAT_NODE_ARRAY:
    case MPV_FORMAT_NODE_MAP: {
        mpv_node_list *list = dst->u.list;
        if (list) {
            for (int n = 0; n < list->num; n++) {
                if (list->keys)
                    delete[] list->keys[n];
                if (list->values)
                    free_node(&list->values[n]);
            }
            delete[] list->keys;
            delete[] list->values;
        }
        delete list;
        break;
    }
    default: ;
    }
    dst->format = MPV_FORMAT_NONE;
}

int MpvCore::setProperty(const QString &name, const QVariant &value)
{
    mpv_node node;
    setNode(&node, value);
    return mpv_set_property(m_mpv, name.toUtf8().data(), MPV_FORMAT_NODE, &node);
}

QVariant MpvCore::getProperty(const QString &name)
{
    mpv_node node;
    int err = mpv_get_property(m_mpv, name.toUtf8().data(), MPV_FORMAT_NODE, &node);
    if (err < 0) {
        return QVariant::fromValue(ErrorReturn(err));
    }
    node_autofree f(&node);
    return node_to_variant(&node);
}

QVariant MpvCore::command(const QVariant &params)
{
    mpv_node node;
    setNode(&node, params);
    mpv_node result;
    int err = mpv_command_node(m_mpv, &node, &result);
    if (err < 0) {
        return QVariant::fromValue(ErrorReturn(err));
    }
    node_autofree f(&result);
    return node_to_variant(&result);
}

inline QVariant MpvCore::node_to_variant(const mpv_node *node)
{
    switch (node->format) {
    case MPV_FORMAT_STRING:
        return QVariant(QString::fromUtf8(node->u.string));
    case MPV_FORMAT_FLAG:
        return QVariant(static_cast<bool>(node->u.flag));
    case MPV_FORMAT_INT64:
        return QVariant(static_cast<qlonglong>(node->u.int64));
    case MPV_FORMAT_DOUBLE:
        return QVariant(node->u.double_);
    case MPV_FORMAT_NODE_ARRAY: {
        mpv_node_list *list = node->u.list;
        QVariantList qlist;
        for (int n = 0; n < list->num; n++)
            qlist.append(node_to_variant(&list->values[n]));
        return QVariant(qlist);
    }
    case MPV_FORMAT_NODE_MAP: {
        mpv_node_list *list = node->u.list;
        QVariantMap qmap;
        for (int n = 0; n < list->num; n++) {
            qmap.insert(QString::fromUtf8(list->keys[n]),
                        node_to_variant(&list->values[n]));
        }
        return QVariant(qmap);
    }
    default: // MPV_FORMAT_NONE, unknown values (e.g. future extensions)
        return QVariant();
    }
}

inline int MpvCore::get_error(const QVariant &v)
{
    if (!v.canConvert<ErrorReturn>())
        return 0;
    return v.value<ErrorReturn>().error;
}
