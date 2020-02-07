# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'FRED Server Manual'
#copyright = '2020, CZ.NIC' # not copyrighted
author = 'Lena'

# The full version, including alpha/beta/rc tags
release = os.getenv('CI_COMMIT_TAG')
if not release:
    release = os.popen('git describe --tags').read().strip().split("-")[0]


# -- General configuration ---------------------------------------------------

default_role = 'code'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
]

# Add any paths that contain templates here, relative to this directory.
#templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build']

# Substitutions
rst_prolog = """
.. |br| raw:: manpage

   .br
"""

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_title = project
#html_short_title =

html_show_copyright = False

html_theme = 'bizstyle'

html_theme_options = {
    'nosidebar': True,
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
#html_static_path = ['_static']


# -- Options for Man pages output ---------------------------------------------

man_pages = [
    ('fred-admin/index', 'fred-admin', 'FRED command-line administration tool', '', 1),
    ('fred-admin/init/index', 'fred-admin-inits', 'FRED command-line administration tool; registry initialization commands', '', 1),
    ('fred-admin/cron/index', 'fred-admin-crons', 'FRED command-line administration tool; cron commands', '', 1),
]

man_show_urls = True
