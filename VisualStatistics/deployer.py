import subprocess
import sys
import os
import re

def proj_root_dir():
    path = os.path.dirname(os.path.realpath(__file__))
    return os.path.dirname(path)

def get_version():
    proc = subprocess.Popen(["/usr/bin/git", "describe", "--tags", "--exact-match"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception("get tag information failed: " + err)
    mo = re.match(r"v(\d)\.(\d)\.(\d)\.(\d)", out)
    if mo is None:
        raise Exception("invalid tag format: " + out)
    for i in xrange(1, 5):
        ver_num = int(mo.group(i))
        if ver_num < 0 or ver_num > 9:
            raise Exception("each part of version must >=0 and <= 9")
    return mo.group(1), mo.group(2), mo.group(3), mo.group(4)

def get_version_file():
    path = os.path.dirname(os.path.realpath(__file__))
    return os.path.join(path, "Version.h")

def get_rc_file():
    path = os.path.dirname(os.path.realpath(__file__))
    return os.path.join(path, "VisualStatistics.rc")

def get_package_xml_file():
    path = proj_root_dir()
    return os.path.join(path, "installer", "installer", "packages", "visualstatistics", "meta", "package.xml")

def check_version_existance(ver_info):
    path = "/visualstat/win/visualstatistics/%s.%s.%s.%scontent.7z" % ver_info
    cmd_list = ["/usr/bin/ssh", "-o", "StrictHostKeyChecking=no", "root@sdu.int.nokia-sbell.com", "test", "-f", path]

    proc = subprocess.Popen(cmd_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if proc.returncode == 0:
        raise Exception("version %s.%s.%s.%s already exists on remote" % ver_info)
    if proc.returncode != 1:
        raise Exception("check version existance failed (%d): %s" % (proc.returncode, err))

def update_version_file(ver_info):
    version_file = get_version_file()
    for line in fileinput.input(version_file, inplace=True):
        # inside this loop the STDOUT will be redirected to the file
        # the comma after each print statement is needed to avoid double line breaks
        if line.startswith("#define VER_FILEVERSION "):
            repl = ",".join(ver_info)
            print re.sub(r"\d,\d,\d,\d", repl, line),
        elif line.startswith("#define VER_FILEVERSION_STR "):
            repl = ".".join(ver_info)
            print re.sub(r"\d\.\d\.\d\.\d", repl, line),
        else:
            print line,

def update_package_xml_file(ver_info):
    package_xml_file = get_package_xml_file()
    for line in fileinput.input(package_xml_file, inplace=True):
        if line.startswith("    <Version>"):
            repl = ".".join(ver_info)
            print re.sub(r"\d\.\d\.\d\.\d", repl, line),
        elif line.startswith("    <ReleaseDate>"):
            repl = time.strftime("%Y-%m-%d")
            print re.sub(r"\d{4}\-\d{2}\-\d{2}", repl, line),
        else:
            print line,

def touch_rc_file():
    rc_file = get_rc_file()
    with open(rc_file, "a"):
        os.utime(rc_file, None)

def copy_target_file():
    src = proj_root_dir()
    src = os.path.join(src, "build-VisualStatistics-Deploy", "VisualStatistics.exe")
    dest = proj_root_dir()
    dest = os.path.join(dest, "installer", "installer", "packages", "visualstatistics", "data")
    if not os.path.exists(dest):
        os.makedirs(dest)
    shutil.copy(src, dest)

def update_repository():
    print "updating repository..."
    cwd = os.getcwd()
    path = os.path.join(proj_root_dir(), "installer", "installer")
    os.chdir(path)

    proc = subprocess.Popen(["repogen", "--update-new-components", "-p", "packages", "repository"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception("update repository failed")

    os.chdir(cwd)

def upload_repositry():
    print "uploading updated packages..."
    ver_info = get_version()
    files = "%s*" % ".".join(ver_info)

    path = "../installer/installer/repository/visualstatistics/" + files
    dest = "root@sdu.int.nokia-sbell.com:/visualstat/win/visualstatistics"
    proc = subprocess.Popen("/usr/bin/scp -B %s %s" % (path, dest), stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception(err)

    print "uploading Updates.xml..."
    dest = os.path.dirname(dest)

    path = "../installer/installer/repository/Updates.xml"
    proc = subprocess.Popen(["/usr/bin/scp", "-B", path, dest], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception(err)

def restore_files():
    cwd = os.getcwd()
    os.chdir(proj_root_dir())

    proc = subprocess.Popen(["/usr/bin/git", "checkout", "--", "VisualStatistics/Version.h", "installer/installer/packages/visualstatistics/meta/package.xml"],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception("restore files failed: " + err)

    os.chdir(cwd)

def copy_pdb_file():
    ver_info = get_version()
    dest = os.path.join("/cygdrive/d/VisualStatisticsPDB", "v" + ".".join(ver_info))
    if not os.path.exists(dest):
        os.makedirs(dest)
    shutil.copy("../build-VisualStatistics-Deploy/visualstatistics.pdb", dest)

if __name__ == "__main__":
    if sys.argv[1] == "prebuild":
        import fileinput
        import time

        ver_info = get_version()
        check_version_existance(ver_info)
        update_version_file(ver_info)
        update_package_xml_file(ver_info)

        # touch the rc file to force recompile it
        touch_rc_file()
    else:
        import shutil

        copy_target_file()
        update_repository()
        upload_repositry()
        restore_files()
        copy_pdb_file()
