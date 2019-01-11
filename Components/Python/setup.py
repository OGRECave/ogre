import setuptools
import time, os.path as path, platform
from setuptools import setup, find_packages, Extension
import numpy as np
import glob

major_ver = 1
minor_ver = 11
nano_ver = 5
branch = ''

version = "%d.%d.%d%s" % (major_ver, minor_ver, nano_ver, branch)

##### Get info from associated text files #####
# Write version.py
with open( "ogre3d/__version__.py", 'w' ) as fh:
    fh.write( "__version__ = '" + version + "'\n" )

# Get the long description from the README file
try:
    with open(path.join('.', 'README.rst'), encoding='utf-8') as f:
        readmerst = f.read()
except: # No long description
    readmerst = ""
    pass

with open('requirements.txt') as f:
    requirements = f.read().splitlines()

def setup_module():
    metadata = dict(
                      name = "ogre3d",
                      version = version,
                      description='Python Ogre3D',
                      long_description = readmerst,
                      author='Pavel Rojtberg',
                      author_email='',
                      url='',
                      license='MIT',
                      packages=find_packages(),
                      install_requires=requirements,
                      setup_requires=requirements,
                      entry_points={
                          # "gui_scripts" suppresses stdout, which we generally do not want
                          "console_scripts": [],
                          "gui_scripts": []
                          },
                      # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
                      extras_require={
                        },
                      ext_modules = [
                        ],
                      package_data={
                        '': ['*.pyd', '*.dll', '*.exe', '*.cfg', '*.dat'],
                        },
                      classifiers=[
                            # How mature is this project? Common values are
                            #   3 - Alpha
                            #   4 - Beta
                            #   5 - Production/Stable
                            'Development Status :: 5 - Production',
                    
                            # Pick your license as you wish (should match "license" above)
                            'License :: MIT',
                    
                            # Specify the Python versions you support here. In particular, ensure
                            # that you indicate whether you support Python 2, Python 3 or both.
                            'Programming Language :: Python :: 3',
                            'Programming Language :: Python :: 3.7',
							
                            # OS
                            'Operating System :: Microsoft :: Windows',
                            'Operating System :: Microsoft :: Linux'
                        ],
                    keywords=[''],
                    #zip_safe=False, # DLLs cannot be zipped
    )

    setup(**metadata)


if __name__ == '__main__':
    t0 = time.time()
    setup_module()
    t1 = time.time()
    print( "Completed: build/install time (s): %.3f" % (t1-t0) )
