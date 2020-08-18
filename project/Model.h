#pragma once

#include <QWidget>
#include <QStandardItemModel>

QAbstractItemModel *LoadModel(QObject *parent, bool allowHidden);
