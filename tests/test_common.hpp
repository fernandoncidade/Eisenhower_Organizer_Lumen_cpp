#pragma once

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QString>
#include <QTemporaryDir>
#include <QTest>

#include <array>
#include <cstdint>
#include <vector>

#include "source/utils/AppConfig.hpp"

namespace eisenhower_tests {

class ScopedEnvVar {
public:
    ScopedEnvVar(const char *name, const QByteArray &value)
        : name_(name),
          was_set_(qEnvironmentVariableIsSet(name)),
          previous_(was_set_ ? qgetenv(name) : QByteArray())
    {
        qputenv(name_, value);
    }

    ~ScopedEnvVar()
    {
        if (was_set_) {
            qputenv(name_, previous_);
        } else {
            qunsetenv(name_);
        }
    }

    ScopedEnvVar(const ScopedEnvVar &) = delete;
    ScopedEnvVar &operator=(const ScopedEnvVar &) = delete;

private:
    const char *name_ = nullptr;
    bool was_set_ = false;
    QByteArray previous_;
};

class ScopedAppIdentity {
public:
    explicit ScopedAppIdentity(const QByteArray &target_name)
        : env_guard_(source::utils::AppConfig::runtime_target_env_var(), target_name)
    {
    }

private:
    ScopedEnvVar env_guard_;
};

class ScopedLocalAppData {
public:
    ScopedLocalAppData()
    {
        QStandardPaths::setTestModeEnabled(true);
        if (dir_.isValid()) {
            const QByteArray native_path = QDir::toNativeSeparators(dir_.path()).toUtf8();
            local_appdata_guard_.emplace("LOCALAPPDATA", native_path);
            appdata_guard_.emplace("APPDATA", native_path);
        }
    }

    ~ScopedLocalAppData()
    {
        local_appdata_guard_.reset();
        appdata_guard_.reset();
        QStandardPaths::setTestModeEnabled(false);
    }

    bool is_valid() const { return dir_.isValid(); }
    QString root_path() const { return dir_.path(); }
    QString file_path(const QString &rel) const { return QDir(dir_.path()).filePath(rel); }

    ScopedLocalAppData(const ScopedLocalAppData &) = delete;
    ScopedLocalAppData &operator=(const ScopedLocalAppData &) = delete;

private:
    QTemporaryDir dir_;
    std::optional<ScopedEnvVar> local_appdata_guard_;
    std::optional<ScopedEnvVar> appdata_guard_;
};

inline QString source_path(const QString &relative = QString())
{
#ifdef EISENHOWER_SOURCE_DIR
    const QString base = QString::fromUtf8(EISENHOWER_SOURCE_DIR);
#else
    const QString base = QDir::currentPath();
#endif
    return relative.isEmpty() ? base : QDir(base).filePath(relative);
}

inline QString build_dir_path(const QString &relative = QString())
{
#ifdef EISENHOWER_BUILD_DIR
    const QString base = QString::fromUtf8(EISENHOWER_BUILD_DIR);
#else
    const QString base = QCoreApplication::applicationDirPath();
#endif
    return relative.isEmpty() ? base : QDir(base).filePath(relative);
}

inline QString toolchain_suffix()
{
#ifdef EISENHOWER_TOOLCHAIN_SUFFIX
    return QString::fromUtf8(EISENHOWER_TOOLCHAIN_SUFFIX);
#else
    return QStringLiteral("generic");
#endif
}

struct ZipEntryInput {
    QString name;
    QByteArray data;
};

inline uint32_t crc32_ieee(const QByteArray &data)
{
    uint32_t crc = 0xFFFFFFFFu;
    for (unsigned char byte : data) {
        crc ^= static_cast<uint32_t>(byte);
        for (int bit = 0; bit < 8; ++bit) {
            const uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}

inline void append_le16(QByteArray &out, uint16_t v)
{
    out.append(static_cast<char>(v & 0xFFu));
    out.append(static_cast<char>((v >> 8) & 0xFFu));
}

inline void append_le32(QByteArray &out, uint32_t v)
{
    out.append(static_cast<char>(v & 0xFFu));
    out.append(static_cast<char>((v >> 8) & 0xFFu));
    out.append(static_cast<char>((v >> 16) & 0xFFu));
    out.append(static_cast<char>((v >> 24) & 0xFFu));
}

// Writes a valid ZIP archive using STORED (method 0) compression so QZipReader can read it cleanly.
inline bool write_stored_zip(const QString &zip_path, const QList<ZipEntryInput> &entries)
{
    struct CentralMeta {
        QByteArray name_utf8;
        uint32_t crc = 0;
        uint32_t size = 0;
        uint32_t local_offset = 0;
    };

    QByteArray archive;
    std::vector<CentralMeta> central_records;
    central_records.reserve(static_cast<size_t>(entries.size()));

    for (const ZipEntryInput &entry : entries) {
        const QByteArray name_utf8 = entry.name.toUtf8();
        const uint32_t crc = crc32_ieee(entry.data);
        const uint32_t size = static_cast<uint32_t>(entry.data.size());
        const uint32_t local_offset = static_cast<uint32_t>(archive.size());

        // Local file header
        append_le32(archive, 0x04034B50u);
        append_le16(archive, 20);  // version needed
        append_le16(archive, 0x0800u);  // UTF-8 filename flag
        append_le16(archive, 0);   // compression = stored
        append_le16(archive, 0);   // mod time
        append_le16(archive, 0x5421u);  // mod date
        append_le32(archive, crc);
        append_le32(archive, size);
        append_le32(archive, size);
        append_le16(archive, static_cast<uint16_t>(name_utf8.size()));
        append_le16(archive, 0);   // extra field length
        archive.append(name_utf8);
        archive.append(entry.data);

        central_records.push_back(CentralMeta{name_utf8, crc, size, local_offset});
    }

    const uint32_t cd_offset = static_cast<uint32_t>(archive.size());
    for (const CentralMeta &meta : central_records) {
        append_le32(archive, 0x02014B50u);
        append_le16(archive, 20);
        append_le16(archive, 20);
        append_le16(archive, 0x0800u);
        append_le16(archive, 0);
        append_le16(archive, 0);
        append_le16(archive, 0x5421u);
        append_le32(archive, meta.crc);
        append_le32(archive, meta.size);
        append_le32(archive, meta.size);
        append_le16(archive, static_cast<uint16_t>(meta.name_utf8.size()));
        append_le16(archive, 0);
        append_le16(archive, 0);
        append_le16(archive, 0);
        append_le16(archive, 0);
        append_le32(archive, 0);
        append_le32(archive, meta.local_offset);
        archive.append(meta.name_utf8);
    }

    const uint32_t cd_size = static_cast<uint32_t>(archive.size()) - cd_offset;
    append_le32(archive, 0x06054B50u);
    append_le16(archive, 0);
    append_le16(archive, 0);
    append_le16(archive, static_cast<uint16_t>(central_records.size()));
    append_le16(archive, static_cast<uint16_t>(central_records.size()));
    append_le32(archive, cd_size);
    append_le32(archive, cd_offset);
    append_le16(archive, 0);

    QDir().mkpath(QFileInfo(zip_path).absolutePath());
    QFile file(zip_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(archive) == archive.size();
}

}  // namespace eisenhower_tests
