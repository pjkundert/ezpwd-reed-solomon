from setuptools import setup, Extension
import os, sys

here				= os.path.abspath( os.path.dirname( __file__ ))

__version__			= None
__version_info__		= None
exec( open( 'version.py', 'r' ).read() )

install_requires		= open( os.path.join( here, "requirements.txt" )).readlines()

ezcod_module			= Extension(
    "_ezcod",
    sources			= [
        "ezcod/ezcod.i",
    ],
    include_dirs		= [ "../../c++" ],
    swig_opts			= [ "-c++",
                                      "-I./ezcod",
                                      "-I../../c++",
                                    "-outdir",
                                      "ezcod" ],
    extra_compile_args		= [ "-std=c++11", "-O3" ],
    libraries			= []
)

bch_module			= Extension(
    "_BCH",
    sources			= [
        "BCH/BCH.i",
    ],
    include_dirs		= [ "../../c++",
                                    "../../c++/ezpwd/bch_include" ],
    swig_opts			= [ "-c++",
                                      "-I./BCH",
                                      "-I../../c++",
                                      "-I../../c++/ezpwd/bch_include",
                                    "-outdir",
                                      "BCH" ],
    extra_objects		= [ "../../djelic_bch.o" ],
    extra_compile_args		= [ "-std=c++11", "-O3" ],
    extra_link_args		= [ "-std=c++11", "-O3" ],
    libraries			= []
)

setup(
    name			= "ezpwd_reed_solomon",
    version			= __version__,
    tests_require		= [ "pytest" ],
    author			= "Perry Kundert",
    author_email		= "perry@hardconsulting.com",
    url				= "https://github.com/pjkundert/ezpwd_reed_solomon",
    description			= """Python bindings for ezpwd_reed_solomon ezcod, BCH""",
    ext_modules			= [ ezcod_module, bch_module ],
    packages			= [
        "ezpwd_reed_solomon",
        "ezpwd_reed_solomon/ezcod",
        "ezpwd_reed_solomon/BCH"
    ],
    package_dir			= {
        "ezpwd_reed_solomon":		".",
        "ezpwd_reed_solomon/ezcod":	"./ezcod",
        "ezpwd_reed_solomon/BCH":	"./BCH",
    },
    install_requires		= install_requires,
    license			= "Dual License; GPLv3 and Proprietary",
    keywords			= "ezpwd ezcod reed-solomon error erasure correction",
    classifiers			= [
        "License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)",
        "License :: Other/Proprietary License",
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3.10",
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Environment :: Console",
        "Environment :: Web Environment",
        "Topic :: Scientific/Engineering :: Interface Engine/Protocol Translator",
        "Topic :: Text Processing :: Filters"
    ],
)
