# -*- coding: utf-8 -*-
import os
from pathlib import Path
from setuptools import setup, Command
from setuptools.command.build_py import build_py
import tarfile

VERSION = "0.0.0"
SUPERTUXKART_URL = f"https://github.com/supertuxkart/stk-code/releases/download/{VERSION}/SuperTuxKart-{VERSION}-src.tar.xz"
this_directory = Path(__file__).absolute().parent
assets_file = this_directory / f'SuperTuxKart-{VERSION}.tar.xz'

class FetchDataCommand(Command):
    description = "fetch the supertuxkart assets"
    user_options = [('force', 'r', 'forcibly fetch the data (delete existing assets)')]
    boolean_options = ['force']

    def initialize_options(self):
        self.force = 0

    def finalize_options(self):
        pass

    def run(self):
        import requests, sys
        
        if not assets_file.exists() or self.force:
            with assets_file.open('wb') as fp, requests.get(SUPERTUXKART_URL, stream=True, allow_redirects=True) as r:
                total_length = r.headers.get('content-length')
                if total_length is None:
                    fp.write(r.content)
                else:
                    dl = 0
                    total_length = int(total_length)
                    for data in r.iter_content(1 << 20):
                        dl += len(data)
                        fp.write(data)
                        done = int(50 * dl / total_length)
                        sys.stdout.write("\r[%s%s] %3d%%" % ('=' * done, ' ' * (50 - done), 100 * dl / total_length))
        else:
            print("Using existing assets (overwrite with '--force')")

class DataTarFile(tarfile.TarFile):
    def filter(self, info: tarfile.TarInfo, path: str):
        path = Path(info.name)
        parents = path.parents
        if len(parents) > 2 and parents[-3].name == "data":
            info.name = path.relative_to(parents[-3])
            return info


class BuildAndCopyData(build_py):
    description = "build_py and copy the supertuxkart assets"

    def run(self):
        super().run()
        for cmd_name in self.get_sub_commands():
            self.run_command(cmd_name)
        target_path = Path(self.build_lib) / self.packages[0] / 'data'
        import shutil
        try:
            shutil.rmtree(target_path)
        except FileNotFoundError:
            pass

        with DataTarFile.open(assets_file, "r:xz") as tf:
            tf.extractall(filter=tf.filter, path=target_path)

        # print(tf)
        # data_files = []
        # base = this_directory / 'data'
        # for p in base.glob('**/*'):
        #     if p.is_file() and p.suffix != '.py':
        #         t = target_path / p.relative_to(base)
        #         t.parent.mkdir(parents=True, exist_ok=True)
        #         self.copy_file(p, t)

    sub_commands = [('fetch_data', lambda x: True)]


def ignore(base, entries):
    return [e for e in entries if '.py' in e]


setup(
    name='PySuperTuxKart2-Data',
    version=VERSION,
    author='Benjamin Piwowarski',
    author_email='benjamin@piwowarski.fr',
    description='Python SuperTuxKart 2 data package',
    long_description="This package contains all the game data for PySTK2",
    url="https://github.com/bpiwowar/pystk2",
    classifiers=[
        'Development Status :: 4 - Beta',
        'Environment :: Console',
        'Intended Audience :: Education',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: GNU General Public License v3 (GPLv3)',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Topic :: Scientific/Engineering',
        'Topic :: Scientific/Engineering :: Artificial Intelligence',
    ],

    # tell setuptools that all packages will be under the 'src' directory
    # and nowhere else
    packages=['pystk2_data'],
    package_dir={'pystk2_data': '.'},

    install_requires=['requests'],
    python_requires='>=3.6',
    # add custom build_ext command
    # cmdclass=dict(fetch_data=FetchDataCommand, build_py=BuildAndCopyData),
    zip_safe=False,
)
