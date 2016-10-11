function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (installer.value("os") === "win") {
        component.addOperation("CreateShortcut", "@TargetDir@\\VisualStatistics.exe",
                               "@DesktopDir@\\VisualStatistics.lnk");
    } else if (installer.value("os") === "x11") {
        var keyValues = "Version=1.0\nType=Application\nTerminal=false\nExec=@TargetDir@/VisualStatistics.sh\nName=VisualStatistics\nIcon=@TargetDir@/VisualStatistics.png";
        component.addOperation("CreateDesktopEntry", "@HomeDir@/Desktop/VisualStatistics.desktop", keyValues);
        component.addOperation("CreateDesktopEntry", "VisualStatistics.desktop", keyValues);
    }
}
