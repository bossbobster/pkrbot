from setuptools import setup, Extension
from Cython.Build import cythonize

ext = Extension(
    'pkrbot.pkrbot',  # Build as module within package
    ['pkrbot.pyx'],
    extra_compile_args=[
        '-O3',
        '-ffast-math', '-funroll-loops', '-finline-functions',
        '-fomit-frame-pointer', '-DNDEBUG',
    ],
    extra_link_args=[],
)

setup(
    packages=['pkrbot'],  # Declare pkrbot as a package
    package_dir={'pkrbot': '.'},  # Package contents are in current directory
    ext_modules=cythonize(
        [ext],
        compiler_directives={
            'language_level': '3',
            'boundscheck': False,
            'wraparound': False,
            'cdivision': True,
            'initializedcheck': False,
        }
    ),
)
