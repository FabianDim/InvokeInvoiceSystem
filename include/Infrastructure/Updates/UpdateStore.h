#pragma once
#include "Infrastructure/Updates/UpdateSecurity.h"
namespace Infrastructure::Updates {
QString update_root();
QString active_directory(const QString& root);
void set_active_directory(const QString& root, const QString& directory);
Manifest verify_staged(const QString& directory, const QString& current, bool fresh = false);
bool launch_verified(const QString& directory, const Manifest& manifest, qint64 wait_for_pid);
bool launch_cached_update();
bool wait_for_launcher(qint64 pid);
}
