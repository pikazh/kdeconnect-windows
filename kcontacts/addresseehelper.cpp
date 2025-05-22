/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Carsten Pfeiffer <pfeiffer@kde.org>

    This SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "addresseehelper_p.h"

#include <QObject>

using namespace KContacts;

Q_GLOBAL_STATIC(AddresseeHelper, s_self)

// static
AddresseeHelper *AddresseeHelper::self()
{
    return s_self;
}

AddresseeHelper::AddresseeHelper()
{
    initSettings();
}

AddresseeHelper::~AddresseeHelper() = default;

static void addToSet(const QStringList &list, QSet<QString> &container)
{
    for (const auto &str : list) {
        if (!str.isEmpty()) {
            container.insert(str);
        }
    }
}

void AddresseeHelper::initSettings()
{
    mTitles.clear();
    mSuffixes.clear();
    mPrefixes.clear();

    mTitles.insert(QObject::tr("Dr."));
    mTitles.insert(QObject::tr("Miss"));
    mTitles.insert(QObject::tr("Mr."));
    mTitles.insert(QObject::tr("Mrs."));
    mTitles.insert(QObject::tr("Ms."));
    mTitles.insert(QObject::tr("Prof."));

    mSuffixes.insert(QObject::tr("I"));
    mSuffixes.insert(QObject::tr("II"));
    mSuffixes.insert(QObject::tr("III"));
    mSuffixes.insert(QObject::tr("Jr."));
    mSuffixes.insert(QObject::tr("Sr."));

    mPrefixes.insert(QStringLiteral("van"));
    mPrefixes.insert(QStringLiteral("von"));
    mPrefixes.insert(QStringLiteral("de"));

    /*
    KConfig _config(QStringLiteral("kabcrc"), KConfig::NoGlobals);
    KConfigGroup config(&_config, QStringLiteral("General"));

    addToSet(config.readEntry("Prefixes", QStringList()), mTitles);
    addToSet(config.readEntry("Inclusions", QStringList()), mPrefixes);
    addToSet(config.readEntry("Suffixes", QStringList()), mSuffixes);
    mTreatAsFamilyName = config.readEntry("TreatAsFamilyName", true);
    */
    mTreatAsFamilyName = true;
}

bool AddresseeHelper::containsTitle(const QString &title) const
{
    return mTitles.contains(title);
}

bool AddresseeHelper::containsPrefix(const QString &prefix) const
{
    return mPrefixes.contains(prefix);
}

bool AddresseeHelper::containsSuffix(const QString &suffix) const
{
    return mSuffixes.contains(suffix);
}

bool AddresseeHelper::treatAsFamilyName() const
{
    return mTreatAsFamilyName;
}
