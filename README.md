[![License](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://gitlab.expidus.com/expidus/esconf/-/blob/master/COPYING)

# esconf


Esconf is a hierarchical (tree-like) configuration system where the immediate 
child nodes of the root are called “channels”. All settings beneath the 
channel nodes are called “properties.”
See the esconf homepage for usage and examples.

----

### Homepage

[Esconf documentation](https://docs.expidus.com/expidus/esconf/start)

### Changelog

See [NEWS](https://gitlab.expidus.com/expidus/esconf/-/blob/master/NEWS) for details on changes and fixes made in the current release.

### Source Code Repository

[Esconf source code](https://gitlab.expidus.com/expidus/esconf)

### Download a Release Tarball

[Esconf archive](https://archive.expidus.org/src/expidus/esconf)
    or
[Esconf tags](https://gitlab.expidus.com/expidus/esconf/-/tags)

### Installation

From source: 

    % cd esconf
    % ./autogen.sh
    % make
    % make install

From release tarball:

    % tar xf esconf-<version>.tar.bz2
    % cd esconf-<version>
    % ./configure
    % make
    % make install

### Reporting Bugs

Visit the [reporting bugs](https://docs.expidus.com/expidus/esconf/bugs) page to view currently open bug reports and instructions on reporting new bugs or submitting bugfixes.

