#!/usr/bin/env python
from datetime import datetime
import subprocess
import platform
import uuid
import sys
import re

def get_mem():
    if platform.system() == "Linux":
        p1 = subprocess.Popen(["grep", "MemTotal", "/proc/meminfo"], stdout=subprocess.PIPE)
        pattern = r'MemTotal\s*:\s*(.*)$'
        result = re.search(pattern, p1.communicate()[0], re.MULTILINE)
        if result:
            return re.sub(r"\s+", " ", result.group(1))
        p1.close()
    elif platform.system() == "Darwin":
        p1 = subprocess.Popen(["sysctl", "hw.model"], stdout=subprocess.PIPE)
        return p1.communicate()[0]
    return "unknown"

def get_cpu():
    if platform.system() == "Linux":
        p1 = subprocess.Popen(["cat", "/proc/cpuinfo"], stdout=subprocess.PIPE)
        pattern = r'model name\s*:\s*(.*)$'
        result = re.search(pattern, p1.communicate()[0], re.MULTILINE)
        if result:
            return re.sub(r"\s+", " ", result.group(1))
        p1.close()
    elif platform.system() == "Darwin":
        p1 = subprocess.Popen(["system_profiler"], stdout=subprocess.PIPE)
        p2 = subprocess.Popen(["grep", "CPU\ Type"],
                              stdin=p1.stdout, stdout=subprocess.PIPE)
        p1.close()
        return p2.communicate()[0]
    return platform.processor()


def get_info():
    template = ""
    template += '<?xml version="1.0" encoding="UTF-8"?>\n'
    template += '<?xml-model "https://svn.hb.dfki.de/trac/LIMES/raw-attachment/wiki/WikiStart/ExperimentData/description.xml.rnc"?>\n\n'
    template += '<experiment_description>\n'
    template += '  <description>\n'
    template += '    {description}\n'
    template += '  </description>\n'
    template += '  <uuid>\n'
    template += '    {uuid}\n'
    template += '  </uuid>\n'
    template += '  <timestamp>\n'
    template += '    {timestamp}\n'
    template += '  </timestamp>\n'
    template += '  <software>\n'
    template += '    <invocation>\n'
    template += '      {invocation}\n'
    template += '    </invocation>\n'
    template += '    <modules>\n'
    template += '      <module>\n'
    template += '        <name>python</name>\n'
    template += '        <src></src>\n'
    template += '        <revision>{python_version}</revision>\n'
    template += '      </module>\n'
    template += '    </modules>\n'
    template += '  </software>\n'
    template += '  <hardware>\n'
    template += '    <pc>\n'
    template += '      <os>{os}</os>\n'
    template += '      <arch>{arch}</arch>\n'
    template += '      <cpu>{cpu}</cpu>\n'
    template += '      <mem>{mem}</mem>\n'
    template += '    </pc>\n'
    template += '  </hardware>\n'
    template += '  <environment>\n'
    template += '  </environment>\n'
    template += '</experiment_description>\n'
    template += '\n'

    s = template.format(description="My Experiment",
                        uuid=uuid.uuid4(),
                        timestamp=datetime.now().isoformat(),
                        invocation=" ".join(sys.argv),
                        python_version=platform.python_version(),
                        os=" ".join([platform.system(), platform.release(),
                                     platform. version()]),
                        arch=" ".join(platform.architecture()),
                        cpu=get_cpu(),
                        mem=get_mem())
    return s


def main():
    f = open("sysinfo.xml", "w")
    f.write(get_info())
    f.close()


if __name__ == '__main__':
    main()
