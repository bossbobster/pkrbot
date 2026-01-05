from setuptools import setup, Extension
from Cython.Build import cythonize
import os

# Read README
here = os.path.abspath(os.path.dirname(__file__))
with open(os.path.join(here, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

ext = Extension(
    'pkrbot',
    ['pkrbot.pyx'],
    extra_compile_args=[
        '-O3',
        '-ffast-math', '-funroll-loops', '-finline-functions',
        '-fomit-frame-pointer', '-DNDEBUG',
    ],
)

setup(
    name='pkrbot',
    version='1.0.15',
    description='Fast poker hand evaluation library with eval7-compatible API',
    long_description=long_description,
    long_description_content_type='text/markdown',
    author='Bobby Costin',
    url='https://github.com/bossbobster/pkrbot',
    license='MIT',
    python_requires='>=3.8',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Programming Language :: Python :: 3.12',
        'Programming Language :: Python :: 3.13',
        'Programming Language :: Cython',
        'Topic :: Games/Entertainment',
    ],
    keywords='poker hand evaluation eval7 cards game pokerbots',
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
