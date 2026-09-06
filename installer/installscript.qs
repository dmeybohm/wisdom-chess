// Qt Installer Framework component script for the com.daveme.wisdomchess
// package. Referenced from cmake/Installer.cmake.
//
// Adds the platform integration that the plain file payload cannot
// express: Start Menu and desktop shortcuts plus the MSVC runtime on
// Windows, and a .desktop entry on Linux. On macOS the .app bundle is the
// whole deliverable.
//
// @TargetDir@, @StartMenuDir@, @DesktopDir@ and friends are expanded by
// the installer at install time, not by CMake. Do not run this file
// through configure_file(): it would blank them out.

var DESCRIPTION = "Play chess against the Wisdom Chess engine";

function Component()
{
}

Component.prototype.createOperations = function()
{
    // Extract the payload first; everything below refers to it.
    component.createOperations();

    if (systemInfo.kernelType === "winnt") {
        createWindowsOperations();
    } else if (systemInfo.kernelType === "linux") {
        createLinuxOperations();
    }
}

function createWindowsOperations()
{
    var exe = "@TargetDir@/bin/WisdomChessQml.exe";

    component.addOperation("CreateShortcut", exe, "@StartMenuDir@/Wisdom Chess.lnk",
        "workingDirectory=@TargetDir@/bin",
        "iconPath=" + exe, "iconId=0",
        "description=" + DESCRIPTION);
    component.addOperation("CreateShortcut", exe, "@DesktopDir@/Wisdom Chess.lnk",
        "workingDirectory=@TargetDir@/bin",
        "iconPath=" + exe, "iconId=0",
        "description=" + DESCRIPTION);

    // Install the MSVC runtime the app was built against. Accepted exit
    // codes: 0 = installed, 1638 = a newer version is already present,
    // 3010 = installed but a reboot is pending.
    component.addElevatedOperation("Execute", "{0,1638,3010}",
        "@TargetDir@/bin/vc_redist.x64.exe", "/quiet", "/norestart");
}

function createLinuxOperations()
{
    // CreateDesktopEntry writes the "[Desktop Entry]" header itself.
    var entry = "Type=Application\n"
              + "Name=Wisdom Chess\n"
              + "Comment=" + DESCRIPTION + "\n"
              + "Exec=@TargetDir@/bin/WisdomChessQml\n"
              + "Icon=@TargetDir@/share/icons/wisdom-chess.png\n"
              + "Terminal=false\n"
              + "Categories=Game;BoardGame;\n";

    var targetDir = installer.value("TargetDir");
    var homeDir = installer.value("HomeDir");
    if (targetDir.indexOf(homeDir) === 0) {
        // Installed somewhere under $HOME (e.g. with --root): a relative
        // file name puts the entry in $XDG_DATA_HOME/applications.
        component.addOperation("CreateDesktopEntry", "wisdom-chess.desktop", entry);
    } else {
        // System-wide install (the default, /opt/WisdomChess): the menu
        // entry has to go where the installer's elevated helper can write.
        component.addElevatedOperation("CreateDesktopEntry",
            "/usr/share/applications/wisdom-chess.desktop", entry);
    }
}
