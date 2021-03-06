/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SYNCHRONIZED_H
#define SYNCHRONIZED_H

#include <QtCore/qmutex.h>

#include <vector>

QT_BEGIN_NAMESPACE

template<typename T> class WriteSynchronizedRef
{
    Q_DISABLE_COPY_MOVE(WriteSynchronizedRef)

public:
    WriteSynchronizedRef(std::vector<T> &vector) Q_DECL_NOEXCEPT
        : m_vector(vector)
    {}

    void emplace_back(T && value)
    {
        QMutexLocker lock(&m_mutex);
        m_vector.emplace_back(value);
    }

    void emplace_back(const T &value)
    {
        QMutexLocker lock(&m_mutex);
        m_vector.emplace_back(value);
    }

    void emplace_bulk(std::vector<T> && values)
    {
        QMutexLocker lock(&m_mutex);
        if (!m_vector.empty()) {
            m_vector.insert(m_vector.end(), std::make_move_iterator(values.begin()),
                std::make_move_iterator(values.end()));
        } else {
            m_vector = std::move(values);
        }
    }

private:
    mutable QMutex m_mutex;
    std::vector<T> &m_vector;
};

template<typename T> class ReadSynchronizedRef
{
    Q_DISABLE_COPY_MOVE(ReadSynchronizedRef)

public:
    ReadSynchronizedRef(const std::vector<T> &vector) Q_DECL_NOEXCEPT
        : m_vector(const_cast<std::vector<T> &>(vector))
    {}

    int size() const
    {
        return int(m_vector.size());
    }

    /* Unsafe, do not use inside threads. */
    void reset(const std::vector<T> &vector)
    {
        m_next = -1;
        m_vector = const_cast<std::vector<T> &>(vector);
    }

    bool next(T *value) const
    {
        m_next.fetch_add(1, std::memory_order_acquire);
        const bool hasNext = m_next < m_vector.size();
        if (hasNext)
            *value = m_vector[m_next];
        return hasNext;
    }

private:
    std::vector<T> &m_vector;
    mutable std::atomic<int> m_next { -1 };
};

QT_END_NAMESPACE

#endif
