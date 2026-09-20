#include "backup.hpp"
#include "log-helper.hpp"
#include "obs-module-helper.hpp"
#include "path-helpers.hpp"
#include "plugin-state-helpers.hpp"
#include "ui-helpers.hpp"
#include "version.h"

#include <obs-frontend-api.h>
#include <obs-module.h>
#include <util/config-file.h>

#include <QFileDialog>
#include <QMainWindow>
#include <QTextStream>
#include <QTimer>

#include <thread>

namespace advss {

static void showBackupDialogs(const QString &json)
{
	const bool backupWasConfirmed = DisplayMessage(
		obs_module_text("AdvSceneSwitcher.askBackup"), true, false);

	if (!backupWasConfirmed) {
		return;
	}

	QString path = QFileDialog::getSaveFileName(
		nullptr,
		obs_module_text(
			"AdvSceneSwitcher.generalTab.saveOrLoadsettings.exportWindowTitle"),
		GetDefaultSettingsSaveLocation(),
		obs_module_text(
			"AdvSceneSwitcher.generalTab.saveOrLoadsettings.textType"));
	if (path.isEmpty()) {
		return;
	}

	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return;
	}
	auto out = QTextStream(&file);
	out << json;
}

void AskForBackup(obs_data_t *settings)
{
	// Local builds change g_GIT_SHA1 on every commit, which would otherwise
	// prompt for a settings backup on each OBS restart. Skip the dialog.
	(void)settings;
	return;
}

void BackupSettingsOfCurrentVersion()
{
	auto sceneCollectionName = obs_frontend_get_current_scene_collection();
	const std::string fileName =
		std::string("settings-backup-") +
		(sceneCollectionName ? sceneCollectionName : "-") + "-" +
		g_GIT_TAG + ".json";
	bfree(sceneCollectionName);
	auto settingsFile = obs_module_config_path(fileName.c_str());
	if (!settingsFile) {
		return;
	}

	QString dirPath = QFileInfo(settingsFile).absolutePath();
	QDir dir(dirPath);
	if (!dir.exists()) {
		if (!dir.mkpath(dirPath)) {
			blog(LOG_WARNING,
			     "failed to create directory to save automated backup");
			bfree(settingsFile);
			return;
		}
	}

	QFile file(settingsFile);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		bfree(settingsFile);
		return;
	}

	OBSDataAutoRelease data = obs_data_create();
	SavePluginSettings(data);

	auto settings = obs_data_get_json(data);
	QString settingsString = QString(settings ? settings : "");

	auto out = QTextStream(&file);
	out << settingsString;
	bfree(settingsFile);
}

} // namespace advss
