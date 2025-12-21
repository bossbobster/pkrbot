from setuptools import setup, Extension
from Cython.Build import cythonize
import sys
import os

# Read README for long description
this_directory = os.path.abspath(os.path.dirname(__file__))
try:
    with open(os.path.join(this_directory, 'README.md'), encoding='utf-8') as f:
        long_description = f.read()
except FileNotFoundError:
    long_description = 'Fast poker hand evaluation library'

ext = Extension(
    'pkrbot',
    ['pkrbot.pyx'],
    extra_compile_args=[
        '-O3', '-march=native', '-mtune=native',
        '-ffast-math', '-funroll-loops', '-finline-functions',
        '-fomit-frame-pointer', '-DNDEBUG',
    ],
    extra_link_args=[],
)

setup(
    name='pkrbot',
    version='1.0.2',
    description='Fast poker hand evaluation library with eval7-compatible API',
    long_description=long_description,
    long_description_content_type='text/markdown',
    author='Bobby Costin',
    author_email='',
    url='https://github.com/bossbobster/pkrbot',
    license='MIT',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Programming Language :: Python :: 3.12',
        'Programming Language :: Python :: 3.13',
        'Programming Language :: Cython',
        'Topic :: Games/Entertainment',
        'Topic :: Software Development :: Libraries :: Python Modules',
    ],
    keywords='poker, hand evaluation, eval7, cards, game',
    python_requires='>=3.6',
    install_requires=[
        'Cython>=0.29.0',
    ],
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
    py_modules=['pkrbot'],
    zip_safe=False,
)
