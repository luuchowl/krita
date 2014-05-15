/*
 * This file is part of the KDE project
 * Copyright (C) 2014 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "TemplatesModel.h"
#include <kis_factory2.h>
#include <KoTemplateTree.h>
#include <KoTemplateGroup.h>
#include <KoTemplate.h>
#include <kiconloader.h>

struct TemplatesModel::ItemData {
public:
    ItemData()
        : favourite(false)
    {};
    QString name;
    QString description;
    QString file;
    QString icon;
    QString groupName;
    bool favourite;
};

class TemplatesModel::Private {
public:
    Private() {}
    ~Private() { qDeleteAll(items);}
    QList<ItemData*> items;
};

TemplatesModel::TemplatesModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
    QHash<int, QByteArray> roleNames;
    roleNames[NameRole] = "name";
    roleNames[DescriptionRole] = "description";
    roleNames[FileRole] = "file";
    roleNames[IconRole] = "icon";
    setRoleNames(roleNames);

    // Prefill a couple of 
    ItemData* customItem = new ItemData();
    customItem->name = "Custom Image";
    customItem->file = "custom";
    customItem->icon = "filenew-black";
    d->items << customItem;
    ItemData* clipItem = new ItemData();
    clipItem->name = "From Clipboard";
    clipItem->file = "clip";
    clipItem->icon = "fileclip-black";
    d->items << clipItem;
    ItemData* a4pItem = new ItemData();
    a4pItem->name = "Blank Image (A4 Portrait)";
    a4pItem->file = "a4p";
    a4pItem->icon = "A4portrait-black";
    d->items << a4pItem;
    ItemData* a4lItem = new ItemData();
    a4lItem->name = "Blank Image (A4 Landscape)";
    a4lItem->file = "a4l";
    a4lItem->icon = "A4landscape-black";
    d->items << a4lItem;
    ItemData* screenItem = new ItemData();
    screenItem->name = "Blank Image (Screen Size)";
    screenItem->file = "scren";
    screenItem->icon = "filenew-black";
    d->items << screenItem;

    KoTemplateTree templateTree("krita_template", KisFactory2::componentData(), true);
    foreach (KoTemplateGroup *group, templateTree.groups()) {
        if (group->isHidden()) {
            continue;
        }
        foreach (KoTemplate* t, group->templates()) {
            if (t->isHidden())
                continue;

            ItemData* item = new ItemData();
            item->name = t->name();
            item->description = t->description();
            item->file = QString("template://").append(t->file());
            item->icon = KIconLoader::global()->iconPath(t->picture(), KIconLoader::Desktop);
            item->groupName = group->name();
            d->items << item;
        }
    }
}

TemplatesModel::~TemplatesModel()
{
    delete d;
}

QVariant TemplatesModel::data(const QModelIndex& index, int role) const
{
    QVariant data;
    if(index.isValid() && index.row() > -1 && index.row() < d->items.count())
    {
        ItemData* item = d->items[index.row()];
        switch(role) {
            case NameRole:
                data = item->name;
                break;
            case DescriptionRole:
                data = item->description;
                break;
            case FileRole:
                data = item->file;
                break;
            case IconRole:
                data = item->icon;
                break;
            default:
                break;
        }
    }
    return data;
}

int TemplatesModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid())
        return 0;
    return d->items.count();
}

#include "TemplatesModel.moc"
