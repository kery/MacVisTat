function Controller()
{
}

var canceled = false;

Controller.prototype.IntroductionPageCallback = function()
{
  if (installer.value("os") === "win" && installer.isInstaller()) {
    installer.setDefaultPageVisible(QInstaller.StartMenuSelection, false);
    var startMenuDir = installer.value("AllUsersStartMenuProgramsPath") + "\\VisualStatistics";
    var oldExe = "C:\\Program Files\\VisualStatistics\\VisualStatistics.exe";
    while (installer.fileExists(startMenuDir) || installer.fileExists(oldExe)) {
      var result = QMessageBox.critical("quit.oldexe", "Installer",
          "An old version of this program has been found, please uninstall it before continue!",
          QMessageBox.Retry | QMessageBox.Cancel);
      if (result == QMessageBox.Retry) {
        continue;
      } else {
        canceled = true;
        installer.setValue("FinishedText", "<font color='red'>Installation canceled.</font>");
        installer.setDefaultPageVisible(QInstaller.TargetDirectory, false);
        installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
        installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);
        installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
        installer.setDefaultPageVisible(QInstaller.PerformInstallation, false);
        gui.clickButton(buttons.NextButton);
        break;
      }
    }
  }
}

Controller.prototype.FinishedPageCallback = function()
{
  if (installer.isUpdater()) {
    if (installer.status == QInstaller.Success) {
      gui.clickButton(buttons.FinishButton);
    }
  } else if (installer.isInstaller() && canceled) {
    var page = gui.currentPageWidget();
    page.RunItCheckBox.setChecked(false);
    page.RunItCheckBox.setVisible(false);
  }
}
