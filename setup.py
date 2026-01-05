from setuptools import setup, Extension, find_packages
from Cython.Build import cythonize
import os

# Ensure we're in the right directory
here = os.path.abspath(os.path.dirname(__file__))

ext = Extension(
    'pkrbot._pkrbot',  # Build as private extension module
    [os.path.join(here, 'pkrbot.pyx')],
    extra_compile_args=[
        '-O3',
        '-ffast-math', '-funroll-loops', '-finline-functions',
        '-fomit-frame-pointer', '-DNDEBUG',
    ],
    extra_link_args=[],
)

setup(
    packages=['pkrbot'],
    package_dir={'pkrbot': '.'},
    package_data={'pkrbot': ['*.pyx', '*.pxd']},
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
    zip_safe=False,
)
