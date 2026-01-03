import os
import subprocess

# Change from Scripts directory to root
os.chdir('../')

print("Generating Visual Studio 2022 solution.")
subprocess.call(["vendor/bin/premake5.exe", "vs2022"])
